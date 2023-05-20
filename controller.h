#pragma once

#include <thread>
#include "Message.h"
#include "MessageQueue.h"

// Controller class that sends messages to a MessageQueue
class Controller {
public:
    Controller(MessageQueue& queue) : queue(queue) {}

    // Function to be run in a separate thread, sends messages to the queue
    void operator()() {
        Color white = { 255, 255, 255 };
        uint8_t intensity = 255;

        queue.send(Message{ MessageId::SET_STATE, true });
        queue.send(Message{ MessageId::SET_INTENSITY, intensity });
        queue.send(Message{ MessageId::SET_COLOR, white });
        queue.send(Message{ MessageId::SET_STATE, false });
        queue.send(Message{ MessageId::TERMINATE, false });
    }

private:
    MessageQueue& queue; // reference to the message queue
};
