#ifndef LOG_MESSAGE_FACTORY_HPP
#define LOG_MESSAGE_FACTORY_HPP

#include "log_message.hpp"
#include <string>
#include <fstream>
#include <optional>

using namespace std;

class LogMessageFactory
{
public:
    static std::optional<LogMessage> createFromStream(ifstream &input, size_t &lineNumber,
                                                      string &pipeline_id);
};

#endif // LOG_MESSAGE_FACTORY_HPP