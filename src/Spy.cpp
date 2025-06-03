#include "../include/Spy.hpp"
#include <iostream>


/**
 * @brief Implements Spy’s blockArrest: finds and removes a pending Arrest on target.
 * Throws IllegalAction if no such pending Arrest.
 */
void Spy::blockArrest(Player& target) {
    Game* g = target.game();
    if (!g) {
        throw IllegalAction("Game pointer is null");
    }
    g->blockArrest(this, &target);
}

/**
 * @brief Implements Spy’s specialAction: print target’s coin count to stdout.
 * @param self   The Spy performing the action.
 * @param target The Player whose coins are revealed.
 */
void Spy::specialAction(Player& self, Player& target) {
    std::cout << "Spy \"" << self.name() << "\" sees that \""
              << target.name() << "\" has "
              << target.coins() << " coins.\n";
}
