#include "Message.h"
#include "MessageQueue.h"
#include "Controller.h"
#include "Handler.h"

int main() {
    // Create a shared message queue
    MessageQueue queue;

    // Create controller and handler, sharing the same queue
    Controller controller(queue);
    Handler handler(queue);

    // Start threads for controller and handler
    std::thread controllerThread(controller);
    std::thread handlerThread(handler);

    // Wait for both threads to finish
    controllerThread.join();
    handlerThread.join();

    return 0;
}

