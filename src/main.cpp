#include <bits/stdc++.h>
#include <filesystem>
#include "log_message_factory.hpp"
#include "log_message.hpp"
#include "renderer.hpp"

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

void processFile(const fs::path &filePath)
{
    vector<string> pipelineOrder;
    PipelineMap pipelines = readLogFile(filePath, pipelineOrder);

    for (const auto &pipeline_id : pipelineOrder)
        Renderer::render(pipeline_id, pipelines[pipeline_id]);
}

std::vector<fs::directory_entry> getFilesInFolder(const std::string &folder)
{
    std::vector<fs::directory_entry> files;

    for (auto &entry : fs::directory_iterator(folder))
        if (entry.is_regular_file())
            files.push_back(entry);

    if (files.empty())
    {
        std::cerr << "No files found in folder " << folder << "\n";
        return {};
    }

    // Sort files alphabetically by file name
    std::sort(files.begin(), files.end(), [](const fs::directory_entry &a, const fs::directory_entry &b)
              { return a.path().filename().string() < b.path().filename().string(); });

    return files;
}

int main()
{
    const std::string folder = "input/";
    auto files = getFilesInFolder(folder);
    bool again = false;
    if (files.empty())
    {
        cerr << "No files found in folder " << folder << "\n";
        return 1;
    }

    while (true)
    {
        if (again)
            Renderer::clear();
        cout << "\nSelect a log file to process:\n";
        for (size_t i = 0; i < files.size(); ++i)
            cout << i << ": " << files[i].path().filename() << "\n";
        cout << "q: Quit program\n";

        string inputStr;
        size_t choice = 0;
        bool valid = false;

        while (!valid)
        {

            cout << "Enter choice: ";
            inputStr = Renderer::input();

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
                cout << "Invalid input. Enter a valid number or 'q' to quit.\n";
            }
        }

        processFile(files[choice].path());

        cout << "\nDo you want to process another file? [Y/n]: ";
        string againInput = Renderer::input();
        again = againInput.empty() || (againInput[0] != 'n' && againInput[0] != 'N');
        if (!again)
            return 0;
    }

    return 0;
}
