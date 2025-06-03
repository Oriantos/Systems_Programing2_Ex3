#pragma once

#include <stdexcept>
#include <string>

/**
 * @brief Thrown when a player attempts an action not allowed by game rules or role abilities.
 */
class IllegalAction : public std::runtime_error {
public:
    explicit IllegalAction(const std::string& msg)
        : std::runtime_error("Illegal action: " + msg) {}
};

/**
 * @brief Thrown when a player tries to spend more coins than they currently have.
 */
class OutOfCoins : public std::runtime_error {
public:
    explicit OutOfCoins(const std::string& msg = "Not enough coins")
        : std::runtime_error("Out of coins: " + msg) {}
};

/**
 * @brief Thrown when a player attempts to act when it is not their turn.
 */
class NotYourTurn : public std::runtime_error {
public:
    explicit NotYourTurn(const std::string& msg = "Not your turn")
        : std::runtime_error("Not your turn: " + msg) {}
};

/**
 * @brief Thrown when someone queries the winner but the game is still active.
 */
class GameStillActive : public std::runtime_error {
public:
    explicit GameStillActive(const std::string& msg = "Game still active")
        : std::runtime_error("Game still active: " + msg) {}
};
