//
// Created by Jake Meegan on 14/11/2019.
//

#ifndef ASSIGNMENT_1_THREADSAFESTACK_H
#define ASSIGNMENT_1_THREADSAFESTACK_H

#include <stack>
#include <mutex>

struct empty_stack: std::exception {
    const char* what() const throw() {
        return "Stack is empty";
    }
};

template <typename T>
class ThreadSafeStack {
public:
    explicit ThreadSafeStack() = default;


    std::future<int> push(T &&new_value) {
        std::lock_guard<std::mutex> lock(gate);
        std::future<int> future(new_value.sol.get_future());
        stack.emplace(new_value);
        return future;
    }

    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(gate);
        if(stack.empty())
            throw empty_stack();

        std::shared_ptr<T> const res(std::make_shared<T>(stack.top()));
        stack.pop();
        return res;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(gate);
        return stack.empty();
    }

private:
    std::stack<T> stack;
    std::mutex gate;
};


#endif //ASSIGNMENT_1_THREADSAFESTACK_H
