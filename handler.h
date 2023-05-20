#pragma once

#include <thread>
#include<queue>
#include <iostream>
#include "Message.h"
#include "MessageQueue.h"

// Handler class that receives and processes messages from a MessageQueue
class Handler {
public:
    Handler(MessageQueue& queue) : queue(queue) {}

    // Function to be run in a separate thread, receives and processes messages from the queue
    void operator()() {
        while (true) {
            Message msg = queue.receive();
            if (msg.getId() == MessageId::TERMINATE) {
                break;
            }

            std::visit([msg](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, bool>) {
                    std::cout << "SetState " << arg << "\n";
                }
                else if constexpr (std::is_same_v<T, uint8_t>) {
                    std::cout << "SetIntensity " << (unsigned int)arg << "\n";
                }
                else if constexpr (std::is_same_v<T, Color>) {
                    std::cout << "SetColor " << arg.r << ", " << arg.g << ", " << arg.b << "\n";
                }
                }, msg.getData());
        }
    }

private:
    MessageQueue& queue;
};