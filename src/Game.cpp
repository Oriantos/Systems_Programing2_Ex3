#include "../include/Game.hpp"
#include <iostream>

//
// Constructor: initialize coin pool to 50 and currentIndex = 0.
//
Game::Game()
    : _currentIndex(0), _poolCoins(50) {}

Game::~Game() = default;

//
// Add a player to the game.
// Throws if player pointer is null or name already exists.
//
void Game::addPlayer(Player* player) {
    if (!player) {
        throw IllegalAction("Cannot add null player");
    }
    for (auto* p : _players) {
        if (p->name() == player->name()) {
            throw IllegalAction("Duplicate player name: " + player->name());
        }
    }
    _players.push_back(player);
}

//
// Return the name of the player whose turn it is.
// Throws if no players.
//
std::string Game::turn() const {
    if (_players.empty()) {
        throw IllegalAction("No players in game");
    }
    return _players[_currentIndex]->name();
}

//
// Advance to the next turn:
//  1. process any pending actions (Tax, Bribe, Arrest, Sanction, Coup).
//  2. advance currentIndex (wrap around).
//  3. call onStartTurn() for the new current player.
// If no players remain, does nothing.
//
void Game::nextTurn() {
    processPending();
    if (_players.empty()) return;
    _currentIndex = (_currentIndex + 1) % _players.size();
    _players[_currentIndex]->onStartTurn();
}

//
// Get names of all active players (in join order).
//
std::vector<std::string> Game::players() const {
    std::vector<std::string> names;
    for (auto* p : _players) {
        names.push_back(p->name());
    }
    return names;
}

//
// Remove a player from the game (successful Coup).
// Throws if not found. Adjusts currentIndex accordingly.
//
void Game::removePlayer(Player* player) {
    auto it = std::find(_players.begin(), _players.end(), player);
    if (it == _players.end()) {
        throw IllegalAction("Player to remove not found: " + player->name());
    }
    size_t idx = std::distance(_players.begin(), it);
    _players.erase(it);
    if (_players.empty()) {
        _currentIndex = 0;
        return;
    }
    if (idx < _currentIndex || _currentIndex >= _players.size()) {
        // If removal was before current index, shift back one.
        _currentIndex = (_currentIndex == 0) ? 0 : _currentIndex - 1;
    }
}

//
// If exactly one player remains, return their name.
// Otherwise throw GameStillActive.
//
std::string Game::winner() const {
    if (_players.size() == 1) {
        return _players[0]->name();
    }
    throw GameStillActive("More than one player remains");
}

//
// Coin pool (treasury) accessors/mutators.
//
int Game::poolCoins() const {
    return _poolCoins;
}
void Game::takeFromPool(int n) {
    if (n > _poolCoins) {
        throw IllegalAction("Not enough coins in the pool");
    }
    _poolCoins -= n;
}
void Game::returnToPool(int n) {
    if (n < 0) return;
    _poolCoins += n;
}

//
// If a player has ≥10 coins, they must coup rather than any other action.
// Returns true if p.coins() ≥ 10, else false.
//
bool Game::mustCoup(const Player& p) const {
    return p.coins() >= 10;
}

//
// Register pending actions that can be blocked.
// Simply push them into the _pending vector. These will be resolved
// in processPending() at the next nextTurn().
//
void Game::registerTax(Player* actor) {
    _pending.push_back({actor, nullptr, ActionType::Tax});
}
void Game::registerBribe(Player* actor) {
    _pending.push_back({actor, nullptr, ActionType::Bribe});
}
void Game::registerArrest(Player* actor, Player* target) {
    _pending.push_back({actor, target, ActionType::Arrest});
}
void Game::registerSanction(Player* actor, Player* target) {
    _pending.push_back({actor, target, ActionType::Sanction});
}
void Game::registerCoup(Player* actor, Player* target) {
    _pending.push_back({actor, target, ActionType::Coup});
}

//
// Blocking methods: look for a matching pending action (same type & target/actor),
// then remove it and apply any “undo” or compensation if needed.
//
//  - blockTax: remove pending Tax (actor never gets coins).
//  - blockBribe: remove pending Bribe and return 4 coins to pool.
//  - blockArrest: remove pending Arrest (no coin transfer).
//  - blockSanction: remove pending Sanction, make offender pay +1 to pool.
//  - blockCoup: remove pending Coup, offender pays +5 to blocker, return 7 to pool.
//
void Game::blockTax(Player* blocker, Player* target) {
    for (auto it = _pending.begin(); it != _pending.end(); ++it) {
        if (it->type == ActionType::Tax && it->actor == target) {
            _pending.erase(it);
            return;
        }
    }
    throw IllegalAction("No pending Tax to block on " + target->name());
}

void Game::blockBribe(Player* blocker, Player* target) {
    for (auto it = _pending.begin(); it != _pending.end(); ++it) {
        if (it->type == ActionType::Bribe && it->actor == target) {
            // Return 4 coins to pool
            returnToPool(4);
            _pending.erase(it);
            return;
        }
    }
    throw IllegalAction("No pending Bribe to block on " + target->name());
}

void Game::blockArrest(Player* blocker, Player* target) {
    for (auto it = _pending.begin(); it != _pending.end(); ++it) {
        if (it->type == ActionType::Arrest && it->target == target) {
            _pending.erase(it);
            return;
        }
    }
    throw IllegalAction("No pending Arrest to block on " + target->name());
}

void Game::blockSanction(Player* blocker, Player* target) {
    for (auto it = _pending.begin(); it != _pending.end(); ++it) {
        if (it->type == ActionType::Sanction && it->target == target) {
            // Offender pays extra 1 coin back to pool
            it->actor->removeCoins(1);
            returnToPool(1);
            _pending.erase(it);
            return;
        }
    }
    throw IllegalAction("No pending Sanction to block on " + target->name());
}

void Game::blockCoup(Player* blocker, Player* target) {
    for (auto it = _pending.begin(); it != _pending.end(); ++it) {
        if (it->type == ActionType::Coup && it->target == target) {
            if (blocker->coins() < 5) {
                throw OutOfCoins("Need 5 coins to block Coup");
            }
            blocker->removeCoins(5);
            // Return 7 coins to pool since coup is cancelled
            returnToPool(7);
            _pending.erase(it);
            return;
        }
    }
    throw IllegalAction("No pending Coup to block on " + target->name());
}

//
// Return the Player* whose turn it is, or nullptr if no players.
//
Player* Game::getCurrentPlayer() const {
    if (_players.empty()) return nullptr;
    return _players[_currentIndex];
}

//
// Returns true if p is still in the active _players list.
//
bool Game::isActive(Player* p) const {
    return std::find(_players.begin(), _players.end(), p) != _players.end();
}

//
// Process all pending actions in the order they were registered.
// Any action not removed by a corresponding blockX() is finalized here:
//  - Tax: actor gains 2 coins (3 if Governor).
//  - Bribe: actor retains turn (decrement currentIndex to remain on same player).
//  - Arrest: steal 1 coin (2 if Merchant), call target->handleArrested().
//  - Sanction: if target has ≥1 coin, remove 1, return to pool; if target is Baron, call target->handleSanctioned().  
//  - Coup: if target still active, remove them (calls removePlayer); else return 7 coins to pool.
void Game::processPending() {
    // Copy pending list so we can clear _pending immediately
    std::vector<PendingAction> toProcess = _pending;
    _pending.clear();

    for (auto& pa : toProcess) {
        switch (pa.type) {
            case ActionType::Gather:
                pa.actor->addCoins(1);
                break;
            case ActionType::Tax: {
                // Grant coins to actor: 3 if Governor, else 2
                if (pa.actor->roleName() == "Governor") {
                    pa.actor->addCoins(3);
                } else {
                    pa.actor->addCoins(2);
                }
                break;
            }

            case ActionType::Bribe: {
                // Extra turn: keep actor as current player
                if (pa.actor == getCurrentPlayer()) {
                    if (_currentIndex > 0) _currentIndex--;
                    else _currentIndex = _players.size() - 1;
                }
                break;
            }

            case ActionType::Arrest: {
                if (!isActive(pa.actor) || !isActive(pa.target)) break;
                int stolen = 1;
                if (pa.target->roleName() == "Merchant") stolen = 2;
                if (pa.target->coins() < stolen) stolen = pa.target->coins();
                pa.target->removeCoins(stolen);
                pa.actor->addCoins(stolen);
                pa.target->handleArrested();
                break;
            }

            case ActionType::Sanction: {
                if (!isActive(pa.actor) || !isActive(pa.target)) break;
                if (pa.target->coins() > 0) {
                    pa.target->removeCoins(1);
                    returnToPool(1);
                }
                if (pa.target->roleName() == "Baron") {
                    pa.target->handleSanctioned();
                }
                break;
            }

            case ActionType::Coup: {
                if (isActive(pa.target)) {
                    removePlayer(pa.target);
                } else {
                    // If target was already removed, return 7 coins to pool
                    returnToPool(7);
                }
                break;
            }
        }
    }
}
