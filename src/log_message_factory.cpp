#include "log_message_factory.hpp"
#include "log_message.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <optional>

using std::getline;
using std::nullopt;
using std::optional;
using std::string;
using std::stringstream;
using std::vector;

optional<LogMessage> LogMessageFactory::createFromStream(std::ifstream &input, size_t &lineNumber,
                                                         string &pipeline_id)
{
    string line;
    vector<string> headerTokens;
    bool bodyStarted = false;
    string bodyBuffer;

    // Collect header tokens (pipeline_id, id, encoding)
    while (headerTokens.size() < 3)
    {
        if (!getline(input, line))
            return nullopt; // EOF without a complete message
        ++lineNumber;

        if (line.empty())
            continue;

        size_t openPos = line.find('[');
        string headPart = (openPos == string::npos) ? line : line.substr(0, openPos);

        // tokenize headPart
        {
            stringstream ss(headPart);
            string tok;
            while (ss >> tok)
                headerTokens.push_back(tok);
        }

        // if '[' present on this line, start bodyBuffer with content after '['
        if (openPos != string::npos)
        {
            bodyStarted = true;
            bodyBuffer = line.substr(openPos + 1);
        }
    }

    // headerTokens now has at least 3 tokens
    pipeline_id = headerTokens[0];
    string msg_id = headerTokens[1];
    int encoding = 0;
    try
    {
        encoding = stoi(headerTokens[2]);
    }
    catch (...)
    {
        cerr << "Line " << lineNumber << ": Warning: invalid encoding -> " << headerTokens[2] << "\n";
        return nullopt;
    }

    // If body not yet started, find '[' in next lines
    if (!bodyStarted)
    {
        while (true)
        {
            if (!getline(input, line))
            {
                cerr << "Line " << lineNumber << ": Warning: could not find opening [ for pipeline " << pipeline_id << "\n";
                return nullopt;
            }
            ++lineNumber;
            if (line.empty())
                continue;
            size_t openPos = line.find('[');
            if (openPos != string::npos)
            {
                bodyStarted = true;
                bodyBuffer = line.substr(openPos + 1);
                break;
            }
            // otherwise keep searching
        }
    }

    // Collect body until ']' is found
    size_t closePos = bodyBuffer.find(']');
    while (closePos == string::npos)
    {
        if (!getline(input, line))
        {
            cerr << "Line " << lineNumber << ": Warning: unmatched [ in body for pipeline " << pipeline_id << "\n";
            return nullopt;
        }
        ++lineNumber;
        if (!bodyBuffer.empty())
            bodyBuffer += "\n";
        bodyBuffer += line;
        closePos = bodyBuffer.find(']');
    }

    string rawBody = bodyBuffer.substr(0, closePos);
    string afterBody = bodyBuffer.substr(closePos + 1);

    // Parse next_id (may be on same line or subsequent lines)
    string next_id;
    {
        stringstream ss(afterBody);
        if (!(ss >> next_id))
        {
            // read more lines until we get a non-empty token to use as next_id
            while (true)
            {
                if (!getline(input, line))
                {
                    cerr << "Line " << lineNumber << ": Warning: missing next_id for pipeline " << pipeline_id << ", message " << msg_id << "\n";
                    return nullopt;
                }
                ++lineNumber;
                stringstream ss2(line);
                if (ss2 >> next_id)
                    break;
            }
        }
    }

    string decoded = LogMessage::decodeBody(rawBody, encoding);
    return LogMessage(msg_id, encoding, decoded, next_id);
}
