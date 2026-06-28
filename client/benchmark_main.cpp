#include "LobsterParser.h"
#include "ReplayClient.h"
#include "../matching_engine/MatchingEngine.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_lobster_csv>\n";
        return 1;
    }

    std::string filepath = argv[1];
    
    std::cout << "Parsing LOBSTER file: " << filepath << "...\n";
    client::LobsterParser parser(filepath);
    auto messages = parser.parse();
    
    std::cout << "Parsed " << messages.size() << " messages.\n";
    
    me::MatchingEngine engine(messages.size() + 10000); // Ensure pool is large enough
    client::ReplayClient client(engine);
    
    std::cout << "Starting replay benchmark...\n";
    client.replay(messages);
    
    return 0;
}
