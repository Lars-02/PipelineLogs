#include "log_message_factory.hpp"
#include "log_message.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <optional>
#include <unordered_set>

optional<LogMessage> LogMessageFactory::createFromStream(std::ifstream &input, size_t &lineNumber,
                                                         string &pipeline_id)
{
    string line;
    vector<string> headerTokens;
    bool bodyStarted = false;
    string bodyBuffer;
    unordered_set<size_t> usedLines; // track lines already consumed

    // Collect header tokens (pipeline_id, id, encoding)
    while (headerTokens.size() < 3)
    {
        if (!getline(input, line))
            return nullopt;
        ++lineNumber;

        if (usedLines.count(lineNumber))
            continue;

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

        usedLines.insert(lineNumber);
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
            if (usedLines.count(lineNumber))
                continue;
            if (line.empty())
                continue;
            size_t openPos = line.find('[');
            if (openPos != string::npos)
            {
                bodyStarted = true;
                bodyBuffer = line.substr(openPos + 1);
                usedLines.insert(lineNumber);
                break;
            }
            usedLines.insert(lineNumber);
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
        if (usedLines.count(lineNumber))
            continue;
        if (!bodyBuffer.empty())
            bodyBuffer += "\n";
        bodyBuffer += line;
        usedLines.insert(lineNumber);
        closePos = bodyBuffer.find(']');
    }

    string rawBody = bodyBuffer.substr(0, closePos);
    string afterBody = bodyBuffer.substr(closePos + 1);

    // Parse next_id (multi-line, until next log starts)
    string next_id;

    auto isHeaderLine = [](const string &line) -> bool
    {
        stringstream ss(line);
        string first;
        ss >> first;
        return !first.empty(); // any non-empty line could start a new log
    };

    // Collect token(s) from same line after body
    stringstream ss(afterBody);
    string token;
    while (ss >> token)
        next_id += token;

    // Collect next lines until we see a new header
    streampos lastPos = input.tellg();
    while (true)
    {
        lastPos = input.tellg();
        if (!getline(input, line))
            break;
        ++lineNumber;
        if (usedLines.count(lineNumber))
            continue;
        if (line.empty())
            continue;
        // check if line is a new log header (first token + second token + encoding)
        stringstream test(line);
        string t1, t2, t3;
        test >> t1 >> t2 >> t3;
        if (!t1.empty() && !t2.empty() && !t3.empty())
        {
            // new header line; rewind stream for next message
            input.seekg(lastPos);
            --lineNumber;
            break;
        }
        // otherwise, this line is part of the next_id
        stringstream ss2(line);
        while (ss2 >> token)
            next_id += token;

        usedLines.insert(lineNumber);
    }

    string decoded = LogMessage::decodeBody(rawBody, encoding);
    return LogMessage(msg_id, encoding, decoded, next_id.empty() ? "-1" : next_id);
}
