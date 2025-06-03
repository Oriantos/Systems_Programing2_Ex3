#include "../include/Player.hpp"
#include <iostream>

//
// Private helper
//
void Player::ensureMyTurn() const {
    if (_game->turn() != _name) {
        throw NotYourTurn("Player \"" + _name + "\" tried to act out of turn");
    }
}

//
// Constructor
//
Player::Player(const std::string& name, std::unique_ptr<Role> role, Game* game)
    : _name(name), _coins(0), _role(std::move(role)), _game(game) {
    if (!game) {
        throw IllegalAction("Game pointer is null for player " + name);
    }
}

/**
    * @brief Delegate to the Role’s specialAction.
    *
    *   This lets test_Baron.cpp call carol.specialAction(carol, carol).
    */
void Player::specialAction(Player& self, Player& target) {
    _role->specialAction(self, target);
}

//
// Copy constructor — deep-copy the Role
//
Player::Player(const Player& other)
    : _name(other._name),
      _coins(other._coins),
      _role(other.cloneRole()),
      _game(other._game) {
    // Note: _game pointer is shared; logic assumes same Game instance
}

//
// Copy assignment — deep-copy the Role
//
Player& Player::operator=(const Player& other) {
    if (this == &other) return *this;
    _name = other._name;
    _coins = other._coins;
    _role = other.cloneRole();
    _game = other._game;
    return *this;
}

//
// Called by Game at start of this player’s turn.
//
void Player::onStartTurn() {
    _role->onStartTurn(*this);
    // If _coins >= 10, game may force coup, but that’s external logic
}

//
// Called by Game when this player is arrested.
//
void Player::handleArrested() {
    _role->onArrested(*this);
}

//
// Called by Game when this player is sanctioned.
//
void Player::handleSanctioned() {
    _role->onSanctioned(*this);
}

void Player::onSanctioned(Player &p) {
    return;
}


//
// Add coins to this player’s balance.
//
void Player::addCoins(int n) {
    if (n < 0) return;
    _coins += n;
}

//
// Remove coins from this player’s balance. Throws if insufficient.
//
void Player::removeCoins(int n) {
    if (n > _coins) {
        throw OutOfCoins("Player \"" + _name + "\" cannot remove " + std::to_string(n) + " coins");
    }
    _coins -= n;
}

//
// Change this player’s Role (for future expansions).
//
void Player::setRole(std::unique_ptr<Role> newRole) {
    _role = std::move(newRole);
}

//
// 'Gather': +1 coin immediately; cannot be blocked.
// Advances turn.
//
void Player::gather() {
    ensureMyTurn();
    _coins += 1;
    _game->nextTurn();
}

//
// 'Tax': register a pending Tax for processing/​blocking.
// If role cannot tax, throw. Advances turn.
// Coins (+2 or +3) are awarded in Game::processPending().
//
void Player::tax() {
    ensureMyTurn();
    if (!_role->canTax()) {
        throw IllegalAction("Role " + _role->name() + " cannot tax");
    }
    _game->registerTax(this);
    _game->nextTurn();
}

//
// 'Bribe': pay 4 coins, register a pending Bribe.
// If role cannot bribe, throw. The 4 coins are deducted now;
// if blocked, Game returns them to pool. Does NOT advance turn
// (extra turn). Pending is processed later.
//
void Player::bribe() {
    ensureMyTurn();
    if (!_role->canBribe()) {
        throw IllegalAction("Role " + _role->name() + " cannot bribe");
    }
    if (_coins < 4) {
        throw OutOfCoins("Need 4 coins to bribe");
    }
    _coins -= 4;
    _game->registerBribe(this);
    // Do not call nextTurn(); bribe gives immediate extra turn if not blocked.
}

//
// 'Arrest': register a pending Arrest on target.
// If role cannot arrest, throw. Advances turn. Coin transfer
// happens in Game::processPending(). Self ≠ target.
//
void Player::arrest(Player& target) {
    ensureMyTurn();
    if (!_role->canArrest()) {
        throw IllegalAction("Role " + _role->name() + " cannot arrest");
    }
    if (this == &target) {
        throw IllegalAction("Cannot arrest yourself");
    }
    _game->registerArrest(this, &target);
    _game->nextTurn();
}

//
// 'Sanction': pay 3 coins, register pending Sanction on target.
// If role cannot sanction, or insufficient coins, throw. Advances turn.
// The target loses 1 coin (or Baron receives +1) when processed.
//
void Player::sanction(Player& target) {
    ensureMyTurn();
    if (!_role->canSanction()) {
        throw IllegalAction("Role " + _role->name() + " cannot sanction");
    }
    if (_coins < 3) {
        throw OutOfCoins("Need 3 coins to sanction");
    }
    _coins -= 3;
    _game->registerSanction(this, &target);
    _game->nextTurn();
}

//
// 'Coup': pay 7 coins, register a pending Coup on target.
// If insufficient coins or self-target, throw. Advances turn.
// Removal happens in Game::processPending().
//
void Player::coup(Player& target) {
    ensureMyTurn();
    if (_coins < 7) {
        throw OutOfCoins("Need 7 coins to coup");
    }
    if (this == &target) {
        throw IllegalAction("Cannot coup yourself");
    }
    _coins -= 7;
    _game->registerCoup(this, &target);
    _game->nextTurn();
}
