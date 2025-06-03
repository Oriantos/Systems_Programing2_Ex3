#include "../include/Baron.hpp"




/**
 * @brief Baron’s invest action: pay 3 coins, gain 6 coins.
 * Throws OutOfCoins if self.coins() < 3.
 */
void Baron::specialAction(Player& self, Player& /*target*/) {
    if (self.coins() < 3) {
        throw OutOfCoins("Baron cannot invest (needs 3 coins)");
    }
    self.removeCoins(3);
    self.addCoins(6);
}

/**
 * @brief When sanctioned, Baron gains +1 coin instead of losing one.
 * @param self The Player (Baron) who was sanctioned.
 */
void Baron::onSanctioned(Player& self) {
    self.addCoins(1);
}


