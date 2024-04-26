#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>
#include <memory>
#include <utility>
#include "json.hpp"
#include "probe.h"
#include "receiver.h"
#include "simulate.h"

void Simulate::createProbe(int& _probeId, std::string& packetId, std::string& ipAddress, int& _Frequency){
    auto probe{std::make_shared<Probe>(_probeId, packetId, ipAddress, _Frequency)};
    createdProbes.push_back(probe);
}

void Simulate::startProbe(const size_t ind){
    if (getCreatedProbe(ind) == nullptr) std::cerr << "No probe at index to start!" << std::endl;
    auto selectedProbe{getCreatedProbe(ind)};
    selectedProbe->set_isActive(true);
    std::thread newActiveProbe = selectedProbe->startSensorData();
    activeSensors.push_back(std::move(newActiveProbe));
}

std::shared_ptr<Probe> Simulate::getCreatedProbe(const size_t ind){
    if (createdProbes[ind] == nullptr) throw std::runtime_error("No probe at given index!");
    return createdProbes[ind];
}


void Simulate::createReceiver(const int& id, std::shared_ptr<Probe> probe, bool isActive){
    if(probe == nullptr) std::cerr << "Receiver " << id << " No probe to attach!" << std::endl;
    else {
        auto receiver{std::make_shared<Receiver>(id, probe, isActive)};
        createdReceivers.push_back(std::move(receiver));
    }
}

std::shared_ptr<Receiver> Simulate::getCreatedReceiver(const size_t ind){
    if (createdReceivers[ind] == nullptr) throw std::runtime_error("No receiver at given index!");
    return createdReceivers[ind];
}

void Simulate::startReceiver(const size_t ind){
    if (getCreatedReceiver(ind) == nullptr) std::cerr<<"No receiver to start at given index!"<<std::endl;
    else {
        auto selectedReceiver{getCreatedReceiver(ind)};
        selectedReceiver->set_isActive(true);
        std::thread newActiveReceiver{selectedReceiver->startReceiverData()};
        activeReceivers.push_back(std::move(newActiveReceiver));
    }
}

void Simulate::addAllReceivers(){
    if(createdProbes.empty()) throw std::runtime_error("No probes created!");
    else if(createdReceivers.empty()) throw std::runtime_error("No receivers created!");
    else {
        for (const auto &sensor: createdProbes) {
            for (int i = 0; i < createdReceivers.size(); i++) {
                if (getCreatedReceiver(i)->get_listenedSensor()->getId() == sensor->getId())
                    sensor->addReceiver(getCreatedReceiver(i));
            }
        }
    }
}

void Simulate::stopAllReceivers(){
    if (activeReceivers.empty()) std::cerr<<"No receivers to stop!"<<std::endl;
    else {
        for (auto &thread: activeReceivers) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        activeReceivers.clear();
    }
}

void Simulate::stopAllProbes(){
    if (activeReceivers.empty()) std::cerr<<"No probes to stop!"<<std::endl;
    else {
        for (auto &thread: activeSensors) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
}

void Simulate::startAllProbes(){
    if(createdProbes.empty()) throw std::runtime_error("No probes created!");
    else {
        for (int i = 0; i < createdProbes.size(); i++) {
            startProbe(i);
        }
    }
}

void Simulate::startAllReceivers(){
    if(createdReceivers.empty()) throw std::runtime_error("No receivers created!");
    else {
        for (int i = 0; i < createdReceivers.size(); i++) {
            if(getCreatedReceiver(i)->get_isActive()) startReceiver(i);
        }
    }
}

void Simulate::loadConfigs(){
    std::ifstream fProbes("probeConf.json");
    std::ifstream fReceivers("receiverConf.json");

    json data = json::parse(fProbes);
    json data2 = json::parse(fReceivers);

    for (const auto& probe : data["Probes"]) {
        int id{probe["ID"]};
        std::string type{probe["Type"]};
        std::string ipAddress{probe["IPAddr"]};
        int frequency{probe["Frequency"]};
        createProbe(id, type, ipAddress, frequency);
    }

    for(const auto& receiver : data2["Receivers"]){
        int id{receiver["ID"]};
        bool isActive{receiver["isActive"]};
        int probeId{receiver["listenedSensorId"]};
        createReceiver(id, getCreatedProbe(probeId - 1), isActive);

    }
    addAllReceivers();
}