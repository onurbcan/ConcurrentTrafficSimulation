#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
    return;
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
     while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        TrafficLightPhase status = _messageQueue.receive();
        if(status == TrafficLightPhase::green)
            break;
    }
    return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::random_device randomDevice;
    std::mt19937 mt(randomDevice());
    std::uniform_int_distribution<int> dist(4000, 6000); //in milliseconds
    auto randDuration = dist(mt);
    auto initialTime = std::chrono::system_clock::now();
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto finalTime = std::chrono::system_clock::now();
        auto timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(finalTime - initialTime).count();
        if(timeDifference >= randDuration)
        {
            if(_currentPhase == TrafficLightPhase::red)
                _currentPhase = TrafficLightPhase::green;
            else if(_currentPhase == TrafficLightPhase::green)
                _currentPhase = TrafficLightPhase::red;
            _messageQueue.send(std::move(_currentPhase));
            initialTime = std::chrono::system_clock::now();
        }
    }
    return;
}