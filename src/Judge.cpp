#include "../include/Judge.hpp"

/**
 * @brief Implements Judge’s blockBribe: cancels a pending Bribe on target,
 * returning 4 coins to the pool. Throws if no such pending Bribe.
 */
void Judge::blockBribe(Player& target) {
    Game* g = target.game();
    if (!g) {
        throw IllegalAction("Game pointer is null");
    }
    g->blockBribe(this, &target);
}

/**
 * @brief Hook if Judge is sanctioned. In this implementation, the extra
 * penalty (offender paying +1) is handled by Game::blockSanction, so nothing
 * additional is needed here.
 */
void Judge::onSanctioned(Player& /*self*/) {
    // No extra code here; handled by Game::blockSanctioner logic.
    return;
}
