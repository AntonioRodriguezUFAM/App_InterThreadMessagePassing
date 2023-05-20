#pragma once

#include <string>
#include <iostream>
#include <thread>
#include<queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include<iostream>


class Receiver {
public:
    void Run() {
        t = std::thread(&Receiver::Consume, this);
    }
    void Join() {
        if (t.joinable()) {
            t.join();
        }
    }
private:
    std::thread t;

    std::any GetMessage() {
        std::any message;
        std::unique_lock<std::mutex> lock(m);

        cv.wait(lock, []() {return not msg_queue.empty(); });
        message = msg_queue.front();
        msg_queue.pop();

        return message;
    }

    void Consume() {
        while (true) {
            const std::any msg = GetMessage();
            // call matching handlers
        }
    }
};
