#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include "Player.hpp"
#include "Exceptions.hpp"
#include "ActionType.hpp"



/**
 * @brief Manages the overall game state:
 *   - Keeps track of active players, turn order, and coin pool (treasury).
 *   - Registers pending actions that can be blocked (Tax, Bribe, Arrest, Sanction, Coup).
 *   - Allows roles to block those pending actions.
 *   - Processes pending actions at the start of each nextTurn().
 */
class Game {
public:
    Game();
    ~Game();

    /**
     * @brief Add a new player to the game.
     * Throws IllegalAction if name is duplicate or player pointer is null.
     * @param player A pointer to a dynamically allocated Player.
     */
    void addPlayer(Player* player);

    /**
     * @brief Returns the name of the player whose turn it currently is.
     * @return The current turn’s player name.
     */
    std::string turn() const;

    /**
     * @brief Advance to the next player’s turn.
     * First processes any pending actions, then advances currentIndex,
     * and finally invokes onStartTurn() for the new current player.
     */
    void nextTurn();

    /**
     * @brief Get a list of all active players’ names, in join order.
     * @return Vector of player names.
     */
    std::vector<std::string> players() const;

    /**
     * @brief Remove a player from the game (successful coup).
     * Throws IllegalAction if player not found.
     * @param player The Player to remove.
     */
    void removePlayer(Player* player);

    /**
     * @brief If exactly one player remains, return their name.
     * Otherwise throw GameStillActive.
     * @return The winner’s name (if exactly one left).
     */
    std::string winner() const;

    /** @name Coin pool (treasury) management */
    ///@{
    /**
     * @brief Get the current number of coins in the central pool.
     * @return Coins available in the pool.
     */
    int poolCoins() const;

    /**
     * @brief Remove n coins from the pool. Throws IllegalAction if insufficient.
     * @param n Number of coins to take.
     */
    void takeFromPool(int n);

    /**
     * @brief Return n coins to the pool (e.g., when bribe is blocked).
     * @param n Number of coins to return (n ≥ 0).
     */
    void returnToPool(int n);
    ///@}

    /**
     * @brief Checks if a given player must coup this turn (if they have ≥10 coins).
     * @param p The Player to check.
     * @return True if p.coins() ≥ 10, else false.
     */
    bool mustCoup(const Player& p) const;

    /** @name Registering pending actions that can be blocked */
    ///@{
    void registerTax(Player* actor);
    void registerBribe(Player* actor);
    void registerArrest(Player* actor, Player* target);
    void registerSanction(Player* actor, Player* target);
    void registerCoup(Player* actor, Player* target);
    ///@}

    /** @name Blocking methods (called by Role::blockX) */
    ///@{
    void blockTax(Player* blocker, Player* target);
    void blockBribe(Player* blocker, Player* target);
    void blockArrest(Player* blocker, Player* target);
    void blockSanction(Player* blocker, Player* target);
    void blockCoup(Player* blocker, Player* target);
    ///@}

private:
    /**
     * @brief Internal struct representing a pending action that may be blocked.
     */
    struct PendingAction {
        Player* actor;    ///< The player who initiated the action.
        Player* target;   ///< The target player (nullptr if none).
        ActionType type;  ///< The type of action.
    };

    std::vector<Player*>       _players;   ///< Active players (in join order).
    size_t                     _currentIndex; ///< Index in _players of whose turn it is.
    int                        _poolCoins; ///< Number of coins in the central pool.
    std::vector<PendingAction> _pending;   ///< List of pending actions that can be blocked.

    /**
     * @brief Return a pointer to the Player whose turn it is.
     * @return The current player pointer, or nullptr if no players.
     */
    Player* getCurrentPlayer() const;

    /**
     * @brief Returns true if the given player pointer is still active in _players.
     * @param p The Player* to check.
     * @return True if p is still present.
     */
    bool isActive(Player* p) const;

    /**
     * @brief Process all pending actions (Tax, Bribe, Arrest, Sanction, Coup).
     * Called once at the beginning of each nextTurn(). Finalizes any action
     * not blocked, applies coin changes, removals, role hooks, etc.
     */
    void processPending();
};
