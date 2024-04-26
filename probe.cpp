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
#include "probe.h"
#include "receiver.h"

void Probe::addReceiver(const std::shared_ptr<Receiver>& receiver){
    if (receiver == nullptr) throw std::runtime_error("No receiver to add!");
    else activerec.push_back(receiver);
}

std::pair<std::string, std::chrono::milliseconds::rep> Probe::sendICMP(){
    boost::asio::io_context io_context;
    boost::asio::ip::icmp::socket socket(io_context, boost::asio::ip::icmp::v4());

    boost::asio::ip::icmp::endpoint destination(boost::asio::ip::address::from_string(IPAddr), 0);

    std::string request(64, '\0');
    request[0] = 8;
    request[1] = 0;
    request[2] = 0;
    request[3] = 0;
    request[4] = 0;
    request[5] = 42;
    request[6] = 0;
    request[7] = 0;

    uint32_t checksum = 0;
    for (size_t i = 0; i < request.size(); i += 2) {
        checksum += (static_cast<uint8_t>(request[i]) << 8) + static_cast<uint8_t>(request[i + 1]);
    }
    checksum = (checksum >> 16) + (checksum & 0xFFFF);
    checksum += (checksum >> 16);
    checksum = ~checksum;

    request[2] = checksum >> 8;
    request[3] = checksum & 0xFF;

    auto start_time = std::chrono::steady_clock::now();
    socket.send_to(boost::asio::buffer(request), destination);

    boost::asio::ip::icmp::endpoint sender;
    std::array<char, 128> reply_buffer;
    socket.receive_from(boost::asio::buffer(reply_buffer), sender);
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    auto duration_c = duration.count();
    std::string send = sender.address().to_string();
    return std::make_pair(send, duration_c);
}

std::pair<std::string, std::chrono::milliseconds::rep> Probe::sendHTTP(){
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(IPAddr, "http");
    boost::asio::ip::tcp::endpoint endpoint = *endpoints.begin();
    std::string server_ip = endpoint.address().to_string();
    boost::asio::ip::tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    std::string request = "GET / HTTP/1.1\r\nHost: " + IPAddr + "\r\n\r\n";

    auto start_time = std::chrono::steady_clock::now();
    boost::asio::write(socket, boost::asio::buffer(request));

    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n\r\n");

    auto end_time = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    auto duration_c = duration.count();

    return std::make_pair(server_ip, duration_c);

}

void Probe::sendToReceiver(){
    std::string address;
    std::chrono::milliseconds::rep time;
    if (packetType == "ICMP"){
        for (int i = 0; i < 10; ++i) {
            std::tie(address, time) = sendICMP();
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(1000.0 / Frequency)));
            std::string telegram = "Ping response received from " + address + ": "
                                   + "RTT = " + std::to_string(time) + " ms, Packet type: " + packetType + "\n";
            std::lock_guard<std::mutex> lock(mtx);
            if(!activerec.empty()) for (auto & j : activerec) j->receiveValues(telegram);
        }
    } else if (packetType == "HTTP"){
        for (int i = 0; i < 10; ++i) {
            std::tie(address, time) = sendHTTP();
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(1000.0 / Frequency)));
            std::string telegram = "Ping response received from " + address + ": "
                                   + "RTT = " + std::to_string(time) + " ms, Packet type: " + packetType + "\n";
            std::lock_guard<std::mutex> lock(mtx);
            if(!activerec.empty()) for (auto & j : activerec) j->receiveValues(telegram);
        }
    } else {
        std::string telegram = "Unknown/Not supported packet type";
        if(!activerec.empty()) for (auto & j : activerec) j->receiveValues(telegram);
    }

}

bool Probe::get_isActive() const{
    return isActive;
}

void Probe::set_isActive(bool value){
    isActive = value ;
}

double Probe::getFrequency() const{
    return Frequency;
}

int Probe::getId() const{
    return probeId;
}

std::thread Probe::startSensorData(){
    std::thread generatedData(&Probe::sendToReceiver, this);
    return generatedData;
}