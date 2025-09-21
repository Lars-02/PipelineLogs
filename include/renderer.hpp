#ifndef PIPELINE_RENDERER_HPP
#define PIPELINE_RENDERER_HPP

#include <vector>
#include "map"
#include <string>
#include "unordered_map"
#include "log_message.hpp"

using namespace std;

class Renderer
{
public:
    static void clear();

    static string input();

    static void render(const string &pipeline_id, const unordered_map<string, LogMessage> &messages);
};

#endif // PIPELINE_RENDERER_HPP