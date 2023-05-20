
#include <string>
#include <iostream>
#include <thread>
#include<queue>
#include <mutex>
#include <any>

#include <condition_variable>
#include <chrono>


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

    template <typename Callback>
    void Handle(Callback cb) {
        static_assert(function_traits<Callback>::arity == 1, "Only one argument is supported");
        callbackWrappers.emplace_back(cb);
    }

private:
    std::thread t;
    std::vector<CallbackWrapper> callbackWrappers;

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
            for (CallbackWrapper& callbackWrapper : callbackWrappers) {
                if (callbackWrapper(msg.type())) {
                    callbackWrapper(msg);
                }
            }
        }
    }
};


template <typename Callback>
void Handle(Callback cb) {
    static_assert(function_traits<Callback>::arity == 1, "Only one argument is supported");
    callbackWrappers.emplace_back(cb);
}

template<typename T>
struct Wrapper : public CallbackBase {
    typedef typename function_traits<T>::template arg<0> arg_type;

    Wrapper(const T& t) :
        wrappedObject(t) {}

    bool operator()(const std::type_info& type) const override {
        if (typeid(arg_type) == type) {
            return true;
        }
        else {
            return false;
        }
    }

    void operator()(const std::any& data) override {
        wrappedObject(std::any_cast<arg_type>(data));
    }

    T wrappedObject;
};

struct CallbackWrapper {
    template <typename T>
    CallbackWrapper(T&& obj) :
        wrappedCallback(std::make_unique<Wrapper<typename std::remove_reference<T>::type>>
            (std::forward<T>(obj))) { }

    struct CallbackBase {
        virtual bool operator()(const std::type_info& type) const = 0;
        virtual void operator()(const std::any& data) = 0;
        virtual ~CallbackBase() {}
    };

    template<typename T>
    struct Wrapper : public CallbackBase {

        typedef typename function_traits<T>::template arg<0> arg_type;

        Wrapper(const T& t) :
            wrappedObject(t) {}

        bool operator()(const std::type_info& type) const override {
            if (typeid(arg_type) == type) {
                return true;
            }
            else {
                return false;
            }
        }

        void operator()(const std::any& data) override {
            wrappedObject(std::any_cast<arg_type>(data));
        }

        T wrappedObject;
    };

    bool operator()(const std::type_info& type) {
        return (*wrappedCallback)(type);
    }

    void operator()(const std::any& data) {
        return (*wrappedCallback)(data);
    }

    std::unique_ptr<CallbackBase> wrappedCallback;
};


struct Color {
    int r;
    int g;
    int b;
};

struct Message {
    enum class Id {
        SET_STATE = 0,
        SET_INTENSITY,
        SET_COLOR,
        TERMINATE
    };
    union Params {
        Color rgb;
        uint8_t intensity;
        bool state;
    };

    Id id;
    Params params;
};

void SetState(const bool state) {
    std::cout << "SetState " << state << "\n";
}

void SetIntensity(const uint8_t intensity) {
    std::cout << "SetIntensity " << (unsigned int)intensity << "\n";
}

void SetColor(const Color color) {
    std::cout << "SetColor " << color.r << ", " << color.g << ", " << color.b << "\n";
}


std::condition_variable cv;
std::mutex m;
std::queue<Message> msg_queue;

// The GetMessage function will block the calling thread until a message is received where it returns a copy of the received message
Message GetMessage() {
    Message message;
    std::unique_lock<std::mutex> lock(m);

    cv.wait(lock, []() {return not msg_queue.empty(); });
    message = msg_queue.front();
    msg_queue.pop();

    return message;
}

// he PutMessage function makes sure to acquire the mutex before pushing a new message and notifying the receiving thread.
void PutMessage(const Message& message) {
    std::lock_guard<std::mutex> lk(m);
    msg_queue.push(message);
    cv.notify_one();
}

// The controller thread can be implemented as follows.
void controller()
{
    Color white = { 255, 255, 255 };
    uint8_t intensity = 255;

    PutMessage(Message{ Message::Id::SET_STATE, true });      // switch on
    PutMessage(Message{ Message::Id::SET_INTENSITY, intensity });   // set the highest intensity
    PutMessage(Message{ Message::Id::SET_COLOR, white });     // set color to white
    PutMessage(Message{ Message::Id::SET_STATE, false });     // switch off
    PutMessage(Message{ Message::Id::TERMINATE });
}
// And the handler thread can be implemented as follows.

void handler()
{
    while (true) {
        const Message msg = GetMessage();
        switch (msg.id) {
        case Message::Id::SET_STATE:
            SetState(msg.params.state);
            break;
        case Message::Id::SET_INTENSITY:
            SetIntensity(msg.params.intensity);
            break;
        case Message::Id::SET_COLOR:
            SetColor(msg.params.rgb);
            break;
        default:
            break;
        }
    }
}


/*
Once started the handler thread will wait for a message to be received. 
When a message is received, its id will be checked and its param is passed to the correct function handler. 
Finally, we can create a simple main function to test our program
*/

int main() {
    std::thread c(handler);
    std::thread p(controller);

    c.join();
    p.join();

    // How do we use this framework?
    Receiver receiver;

    receiver.Handle([](const bool state) {
        std::cout << "SetState " << state << "\n";
        });
    receiver.Handle([](const uint8_t intensity) {
        std::cout << "SetIntensity " << (unsigned int)intensity << "\n";
       });
    receiver.Handle([](const Color& color) {
        std::cout << "SetColor " << color.r << ", " << color.g << ", " << color.b << "\n";
        });
    receiver.Run();
    receiver.Join();

    return 0;
}