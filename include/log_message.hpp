#ifndef LOG_MESSAGE_HPP
#define LOG_MESSAGE_HPP
#include <string>

using namespace std;

class LogMessage
{
private:
    std::string id;
    int encoding;
    std::string body;
    std::string next_id;

public:
    LogMessage() = default;
    LogMessage(const std::string &id, int encoding, const std::string &body, const std::string &next_id);

    // Accessors
    const std::string &getId() const;
    int getEncoding() const;
    const std::string &getBody() const;
    const std::string &getNextId() const;

    // Mutators
    void setBody(const std::string &decodedBody);

    // Utility
    static std::string decodeBody(const std::string &body, int encoding);
};

#endif // LOG_MESSAGE_HPP
