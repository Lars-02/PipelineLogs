#include <iostream>
#include <fstream>
#include <iomanip>
#include "renderer.hpp"
#include "unordered_set"

using namespace std;

const int CLEAR_LENGTH = 50;

ostream &bold_on(ostream &os)
{
    return os << "\e[1m";
}

ostream &bold_off(ostream &os)
{
    return os << "\e[0m";
}

void Renderer::clear()
{
    cout << string(CLEAR_LENGTH - 3, '\n');
}

string Renderer::input()
{
    string input;
    getline(cin, input);
    return input;
}

void Renderer::render(const string &pipeline_id, const unordered_map<string, LogMessage> &messages)
{
    bold_on(cout);
    cout << "Pipeline " << pipeline_id << "\n";
    bold_off(cout);
    // 1) Find explicit tail (next_id == "-1")
    string tail;
    for (const auto &[id, msg] : messages)
    {
        if (msg.getNextId() == "-1")
        {
            tail = id;
            break;
        }
    }

    // 2) If no explicit tail, fallback to orphan-detection:
    if (tail.empty())
    {
        unordered_set<string> referenced;
        for (const auto &[id, msg] : messages)
            if (msg.getNextId() != "-1")
                referenced.insert(msg.getNextId());

        for (const auto &[id, msg] : messages)
            if (!referenced.count(id))
            {
                tail = id;
                break;
            }
    }

    if (tail.empty())
    {
        cerr << "Warning: pipeline " << pipeline_id << " has no terminating message\n";
        return;
    }

    // Build reverse (next -> current)
    unordered_map<string, string> prev;
    for (const auto &[id, msg] : messages)
    {
        if (msg.getNextId() != "-1" && messages.count(msg.getNextId()))
            prev[msg.getNextId()] = id;
    }

    // Traverse backwards from tail
    vector<pair<string, string>> ordered;
    string cur = tail;
    while (!cur.empty() && messages.count(cur))
    {
        const auto &msg = messages.at(cur);
        ordered.push_back({msg.getId(), msg.getBody()});
        cur = prev.count(cur) ? prev[cur] : "";
    }

    for (auto &[id, body] : ordered)
        cout << "  " << id << " | " << body << "\n";
}