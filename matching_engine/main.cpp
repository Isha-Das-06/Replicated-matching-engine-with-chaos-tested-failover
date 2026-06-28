#include "MatchingEngine.h"
#include <iostream>

int main() {
    me::MatchingEngine engine(10000);
    
    std::cout << "Submitting BUY LIMIT 100 @ $10\n";
    auto execs1 = engine.submit_order(1, 1, me::Side::BUY, me::OrderType::LIMIT, 1000, 100);
    std::cout << "Best Bid: " << engine.get_best_bid() << " Best Ask: " << engine.get_best_ask() << "\n";

    std::cout << "Submitting SELL LIMIT 50 @ $10\n";
    auto execs2 = engine.submit_order(2, 1, me::Side::SELL, me::OrderType::LIMIT, 1000, 50);
    
    for (const auto& e : execs2) {
        std::cout << "Execution: BuyID=" << e.buy_id << " SellID=" << e.sell_id 
                  << " Price=" << e.price << " Qty=" << e.quantity << "\n";
    }

    std::cout << "Best Bid: " << engine.get_best_bid() << " Best Ask: " << engine.get_best_ask() << "\n";
    
    return 0;
}
