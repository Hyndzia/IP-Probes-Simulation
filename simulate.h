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
#include <chrono>
#include <utility>
#include "json.hpp"
#include "probe.h"
#include "receiver.h"

using json = nlohmann::json;


class Simulate{
public:

void createProbe(int& _probeId, std::string& packetId, std::string& ipAddress, int& _Frequency);

void startProbe(const size_t ind);

std::shared_ptr<Probe> getCreatedProbe(const size_t ind);

void createReceiver(const int& id, std::shared_ptr<Probe> probe, bool isActive);

std::shared_ptr<Receiver> getCreatedReceiver(const size_t ind);
void startReceiver(const size_t ind);

void addAllReceivers();

void stopAllReceivers();

void stopAllProbes();

void startAllProbes();
void startAllReceivers();

void loadConfigs();

private:
std::vector<std::shared_ptr<Probe>> createdProbes;
std::vector<std::shared_ptr<Receiver>> createdReceivers;
std::vector<std::thread> activeSensors;
std::vector<std::thread> activeReceivers;
};
