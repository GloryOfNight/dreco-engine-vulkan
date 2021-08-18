#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <functional>

struct thread_task
{
    thread_task(){};

    std::function<void()> job;

    std::function<void()> callback;
};

class thread_pool 
{
public:
    thread_pool();
    ~thread_pool();

    void tick();

    void queueTask(const thread_task& task);

private:
    std::vector<std::thread> _threads;
};