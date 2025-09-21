#include <bits/stdc++.h>
#include <filesystem>
#include "log_message_factory.hpp"
#include "log_message.hpp"

using namespace std;
namespace fs = std::filesystem;

using PipelineMap = unordered_map<string, unordered_map<string, LogMessage>>;

PipelineMap readLogFile(const fs::path &filePath, vector<string> &pipelineOrder)
{
    ifstream input(filePath);
    if (!input.is_open())
    {
        cerr << "Error: could not open " << filePath << "\n";
        return {};
    }

    PipelineMap pipelines;
    size_t lineNumber = 0;

    while (true)
    {
        string pipeline_id;
        auto maybeMsg = LogMessageFactory::createFromStream(input, lineNumber, pipeline_id);
        if (!maybeMsg.has_value())
            break;

        LogMessage msg = std::move(maybeMsg.value());

        if (pipelines.find(pipeline_id) == pipelines.end())
            pipelineOrder.push_back(pipeline_id);

        pipelines[pipeline_id][msg.getId()] = std::move(msg);
    }

    return pipelines;
}

void printPipeline(const string &pipeline_id, const unordered_map<string, LogMessage> &messages)
{
    cout << "Pipeline " << pipeline_id << "\n";

    string tail;
    for (const auto &[id, msg] : messages)
    {
        if (msg.getNextId() == "-1")
        {
            tail = id;
            break;
        }
    }

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

    vector<pair<string, string>> ordered;
    string cur = tail;
    while (!cur.empty() && messages.count(cur))
    {
        const auto &msg = messages.at(cur);
        ordered.push_back({msg.getId(), msg.getBody()});
        cur = prev.count(cur) ? prev[cur] : "";
    }

    for (auto &[id, body] : ordered)
        cout << "\t" << id << "| " << body << "\n";
}

void processFile(const fs::path &filePath)
{
    vector<string> pipelineOrder;
    PipelineMap pipelines = readLogFile(filePath, pipelineOrder);

    for (const auto &pipeline_id : pipelineOrder)
        printPipeline(pipeline_id, pipelines[pipeline_id]);
}

int main()
{
    string folder = "input/";
    vector<fs::directory_entry> files;

    for (auto &entry : fs::directory_iterator(folder))
        if (entry.is_regular_file())
            files.push_back(entry);

    if (files.empty())
    {
        cerr << "No files found in folder " << folder << "\n";
        return 1;
    }

    while (true)
    {
        cout << "\nSelect a log file to process:\n";
        for (size_t i = 0; i < files.size(); ++i)
            cout << i << ": " << files[i].path().filename() << "\n";
        cout << "q: Quit program\n";

        string inputStr;
        size_t choice;
        bool valid = false;

        while (!valid)
        {
            cout << "Enter choice: ";
            cin >> inputStr;

            if (inputStr == "q" || inputStr == "Q")
                return 0;

            try
            {
                choice = stoi(inputStr);
                if (choice < files.size())
                    valid = true;
                else
                    cout << "Invalid index. Try again.\n";
            }
            catch (...)
            {
                cout << "Invalid input. Enter a number or 'q' to quit.\n";
            }
        }

        processFile(files[choice].path());

        cout << "\nDo you want to process another file? [Y/n]: ";
        string again;
        cin.ignore();
        getline(cin, again);
        if (!again.empty() && (again[0] == 'n' || again[0] == 'N'))
            break;
    }

    return 0;
}
