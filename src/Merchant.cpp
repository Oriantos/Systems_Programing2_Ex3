#include "../include/Merchant.hpp"

/**
 * @brief Called at the start of Merchant’s turn:
 * If they have ≥3 coins, they gain +1 coin automatically.
 */
void Merchant::onStartTurn(Player& self) {
    if (self.coins() >= 3) {
        self.addCoins(1);
    }
}

/**
 * @brief Called when Merchant is arrested:
 * If they have ≥2 coins, lose 2; otherwise lose as many as they have.
 * @param self The Player (Merchant) being arrested.
 */
void Merchant::onArrested(Player& self) {
    if (self.coins() >= 2) {
        self.removeCoins(2);
    } else {
        self.removeCoins(self.coins());
    }
}
