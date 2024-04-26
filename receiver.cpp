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



void Receiver::receiveValues(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    receivedValues.push_back(message);
}

void Receiver::printValues() {
    while (printing) {
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(1000.0/get_listenedSensor()->getFrequency())));
        std::lock_guard<std::mutex> lock(mtx);
        if (!receivedValues.empty()) {
            std::string messageToPrint{receivedValues.back()};
            receivedValues.pop_back();
            if(messageToPrint.find("Warning")!= std::string::npos) std::cout << "\n!![Receiver: "<<receiverId<<"]  "<< messageToPrint << "!!\n"<<std::endl;
            else if (messageToPrint.find("Alarm")!= std::string::npos) std::cout << "\n!!!!![Receiver: "<<receiverId<<"]  "<< messageToPrint << "!!!!!\n"<< std::endl;
            else std::cout << "[Receiver: "<<receiverId<<"]  "<< messageToPrint << std::endl;
        }
    }
}

std::shared_ptr<Probe> Receiver::get_listenedSensor() const{
    return listenedSensor;
}

std::thread Receiver::startReceiverData(){
    printing = true;
    std::thread receivedData(&Receiver::printValues, this);
    return receivedData;
}

bool Receiver::get_isActive() const{
    return isActive;
}
void Receiver::set_isActive(bool value){
    isActive = value;
}