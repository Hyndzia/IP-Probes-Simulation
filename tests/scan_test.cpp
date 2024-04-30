#include <iostream>
#include <boost/asio.hpp>
#include <chrono>

bool is_port_open(const std::string& hostname, uint16_t port) {
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket(io_context);
    boost::asio::deadline_timer timer(io_context);
    bool flag = false;

    try {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(hostname), port);
        socket.async_connect(endpoint, [&](const boost::system::error_code& ec) {
            if (!ec) {
                flag = true;
            }
        });

        timer.expires_from_now(boost::posix_time::milliseconds(100));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (ec == boost::asio::error::operation_aborted) {
            } else if (!flag) {

                socket.cancel();
            }
        });
        io_context.run();

    } catch(const boost::system::system_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return flag;
}

void scan_ports(uint16_t startVal, uint16_t endVal, const std::string& hostname){
    bool x;
    std::cout<<"Scanning ports on "<<hostname<<"...."<<std::endl;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (uint32_t port = startVal; port <= endVal; port++) {
        if(is_port_open(hostname, port)) std::cout<<"Port "<<port<<" open!"<<std::endl;
        //else std::cout<<"Port "<<port<<" closed!"<<std::endl;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
    auto seconds = duration.count()/1000;
    std::cout<<"\n\n Scanning completed in "<<seconds<<" s!"<<std::endl;
}

std::string resolve_dns(const std::string& hostname) {
    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(hostname, "");

        for (const auto& endpoint : endpoints) {
            if (endpoint.endpoint().address().is_v4()) {
                return endpoint.endpoint().address().to_string();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return "";
}

int main(){
    char host[100]; std::string hostname;
    uint16_t start; uint16_t end;
    std::cout<<"Enter hostname"<<std::endl;
    scanf("%s", &host); //cin doesnt work when entering host
    hostname = resolve_dns(host);
    std::cout<<"Enter starting point: ";
    std::cin>>start;
    std::cout<<"\n\n";
    std::cout<<"Enter ending point: ";
    std::cin>>end;
    scan_ports(start, end, hostname);
    return 0;
}