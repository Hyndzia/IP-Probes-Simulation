#pragma once
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <utility>
#include "json.hpp"
#include "receiver.h"

class Receiver;
class Probe{
public:
    Probe(int& _probeId, std::string& packetType, std::string& ipAddress, int& Frequency_):
            probeId(_probeId), packetType(packetType), IPAddr(ipAddress), Frequency(Frequency_), isActive(false){}

    void addReceiver(const std::shared_ptr<Receiver>& receiver);

    std::pair<std::string, std::chrono::milliseconds::rep> sendICMP();

    std::pair<std::string, std::chrono::milliseconds::rep> sendHTTP();

    void sendToReceiver();

    bool get_isActive() const;

    void set_isActive(bool value);

    double getFrequency() const;

    int getId() const;

    std::thread startSensorData();

private:
    int probeId;
    std::string IPAddr;
    int Frequency;
    std::string packetType;
    std::string probeQuality;
    bool isActive;
    mutable std::mutex mtx;
    std::vector<std::shared_ptr<Receiver>> activerec;
};