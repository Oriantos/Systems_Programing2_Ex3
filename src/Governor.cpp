#include "../include/Governor.hpp"



/**
 * @brief Implements Governor’s blockTax: finds and removes the pending Tax on target.
 * Throws IllegalAction if no such pending Tax.
 */
void Governor::blockTax(Player& target) {
    Game* g = target.game();
    if (!g) {
        throw IllegalAction("Game pointer is null");
    }
    g->blockTax(this, &target);
}
