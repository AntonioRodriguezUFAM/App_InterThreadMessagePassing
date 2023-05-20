#pragma once

#include <variant>

// Color structure to hold RGB color information
struct Color {
    int r;
    int g;
    int b;
};

// Enum to identify the type of the message
enum class MessageId {
    SET_STATE,
    SET_INTENSITY,
    SET_COLOR,
    TERMINATE
};

// A variant that can hold any of the different types of messages
using MessageData = std::variant<bool, uint8_t, Color>;

// Message class that holds a MessageId and MessageData
class Message {
public:
    Message(MessageId id, MessageData data) : id(id), data(data) {}
    MessageId getId() const { return id; }
    const MessageData& getData() const { return data; }

private:
    MessageId id;
    MessageData data;
};
