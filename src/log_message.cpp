#include "log_message.hpp"
#include <cstdlib>

LogMessage::LogMessage(const std::string &id, int encoding, const std::string &body, const std::string &next_id)
    : id(id), encoding(encoding), body(body), next_id(next_id) {}

const std::string &LogMessage::getId() const {
    return id;
}

int LogMessage::getEncoding() const {
    return encoding;
}

const std::string &LogMessage::getBody() const {
    return body;
}

const std::string &LogMessage::getNextId() const {
    return next_id;
}

void LogMessage::setBody(const std::string &decodedBody) {
    body = decodedBody;
}

std::string LogMessage::decodeBody(const std::string &body, int encoding) {
    if (encoding != 1) return body;

    std::string decoded;
    for (size_t i = 0; i + 1 < body.size(); i += 2) {
        std::string byteStr = body.substr(i, 2);
        char c = static_cast<char>(strtol(byteStr.c_str(), nullptr, 16));
        decoded.push_back(c);
    }
    return decoded;
}
