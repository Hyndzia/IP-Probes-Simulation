#pragma once

#include "probe.h"
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>

class Probe;
class Receiver{
public:
    Receiver(const int& id, std::shared_ptr<Probe>&  sensor, bool& isActive): isActive(isActive), receiverId(id),
                                                                              listenedSensor(sensor), printing(false) {}

    void receiveValues(const std::string& message);

    void printValues();

    std::shared_ptr<Probe> get_listenedSensor() const;

    std::thread startReceiverData();

    bool get_isActive() const;
    void set_isActive(bool value);

private:
    int receiverId;
    std::shared_ptr<Probe> listenedSensor;
    bool isActive;
    mutable std::mutex mtx;
    bool printing;
    std::thread printingThread;
    std::vector<std::string> receivedValues;
};