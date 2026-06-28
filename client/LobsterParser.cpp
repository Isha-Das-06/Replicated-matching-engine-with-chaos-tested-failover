#include "LobsterParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace client {

LobsterParser::LobsterParser(const std::string& filepath) : filepath_(filepath) {}

std::vector<LobsterMessage> LobsterParser::parse() {
    std::vector<LobsterMessage> messages;
    std::ifstream file(filepath_);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open lobster file: " << filepath_ << "\n";
        return messages;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        LobsterMessage msg;
        
        // Time
        if (!std::getline(ss, token, ',')) continue;
        msg.time = std::stod(token);
        
        // Event Type
        if (!std::getline(ss, token, ',')) continue;
        msg.event_type = std::stoi(token);
        
        // Order ID
        if (!std::getline(ss, token, ',')) continue;
        msg.order_id = std::stoull(token);
        
        // Size
        if (!std::getline(ss, token, ',')) continue;
        msg.size = std::stoull(token);
        
        // Price
        if (!std::getline(ss, token, ',')) continue;
        msg.price = std::stoull(token);
        
        // Direction
        if (!std::getline(ss, token, ',')) continue;
        msg.direction = std::stoi(token);
        
        messages.push_back(msg);
    }
    
    return messages;
}

} // namespace client
