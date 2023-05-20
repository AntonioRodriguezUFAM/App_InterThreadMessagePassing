#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include "Message.h"

// MessageQueue class to handle thread-safe sending and receiving of messages
class MessageQueue {
public:
    // send a message, thread-safe
    void send(const Message& msg) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(msg);
        cond_.notify_one();
    }

    // receive a message, thread-safe
    Message receive() {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [this] { return !queue_.empty(); });
        Message msg = queue_.front();
        queue_.pop();
        return msg;
    }

private:
    std::mutex mtx_; // mutex for thread safety
    std::condition_variable cond_; // condition variable for signaling
    std::queue<Message> queue_; // queue of messages
};


