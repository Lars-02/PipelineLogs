#ifndef PIPELINE_RENDERER_HPP
#define PIPELINE_RENDERER_HPP

#include <vector>
#include "map"
#include <string>

using namespace std;

class Renderer
{
public:
    static void clear();

    static string input();

    void render() const;

    string log_name;

    static Renderer &instance();

    Renderer(const Renderer &) = delete;

    Renderer(Renderer &&) = delete;

    Renderer &operator=(const Renderer &) = delete;

    Renderer &operator=(Renderer &&) = delete;

private:
    static void log(const string &string);

    static void printLog(const string &string);

    static string getLogName();

    static Renderer _instance;

    Renderer();
};

#endif // PIPELINE_RENDERER_HPP