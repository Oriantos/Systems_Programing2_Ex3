#include "../include/General.hpp"


/**
 * @brief Implements General’s blockCoup: if there is a pending Coup on target,
 * pay 5 coins to cancel it (return 7 to pool). Throws if insufficient coins or no pending Coup.
 */
void General::blockCoup(Player& target) {
    Game* g = target.game();
    if (!g) {
        throw IllegalAction("Game pointer is null");
    }
    g->blockCoup(this, &target);
}

/**
 * @brief When arrested, General immediately receives 1 coin refund.
 * @param self The Player (General) who was arrested.
 */
void General::onArrested(Player& self) {
    self.addCoins(1);
}
