#ifndef SISPROG4SEM_THREADSAFEQUEUE_H
#define SISPROG4SEM_THREADSAFEQUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <chrono>
#include <optional>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable cond_var;

public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    void push(const T& item);
    void push(T&& item);

    T pop();

    bool empty() const;
    size_t size() const;
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
};

template<typename T>
void ThreadSafeQueue<T>::push(const T& item) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(item);
    }
    cond_var.notify_one();
}

template<typename T>
void ThreadSafeQueue<T>::push(T&& item) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(item));
    }
    cond_var.notify_one();
}

template<typename T>
T ThreadSafeQueue<T>::pop() {
    std::unique_lock<std::mutex> lock(mutex);
    cond_var.wait(lock, [this]{ return !queue.empty(); });
    T item = std::move(queue.front());
    queue.pop();
    return item;
}


template<typename T>
bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty();
}

template<typename T>
size_t ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.size();
}

#endif //SISPROG4SEM_THREADSAFEQUEUE_H
