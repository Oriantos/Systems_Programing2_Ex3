#pragma once

#include <string>
#include <memory>
#include "Role.hpp"
#include "Game.hpp"
#include "Exceptions.hpp"

class Game;

/**
 * @brief Represents a player in the Coup-style game.
 * 
 * Each Player has a name, a coin balance, a Role (via unique_ptr), and a pointer
 * to the Game in which they participate. All Coup actions (gather, tax, bribe,
 * arrest, sanction, coup) are implemented here, with appropriate exception checks.
 */
class Player {
private:
    std::string            _name;   ///< Unique player name.
    int                    _coins;  ///< Current coin balance.
    std::unique_ptr<Role>  _role;   ///< Owned Role object.
    Game*                  _game;   ///< Non-owned pointer to the Game instance.

    /**
     * @brief Throws NotYourTurn if this player is not the one whose turn it is.
     */
    void ensureMyTurn() const;

public:

    /**
     * @brief Construct a Player with a given name, role, and game pointer.
     * @param name  Unique player name.
     * @param role  A unique_ptr to the role to assign to this player.
     * @param game  Pointer to the Game object.
     */
    Player(const std::string& name, std::unique_ptr<Role> role, Game* game);


    void specialAction(Player& self, Player& target);


    /**
     * @brief Copy constructor — deep-copies the Role.
     * @param other The Player to copy from.
     */
    Player(const Player& other);

    /**
     * @brief Copy assignment — deep-copies the Role.
     * @param other The Player to assign from.
     * @return Reference to this.
     */
    Player& operator=(const Player& other);

    ~Player() = default;

    /** @name Accessors */
    ///@{
    std::string name() const { return _name; }
    int coins() const { return _coins; }
    std::string roleName() const { return _role->name(); }
    Game* game() const { return _game; }
    ///@}

    /** @name Actions (each throws if role doesn't permit or out of turn) */
    ///@{
    /**
     * @brief Gather action: gain 1 coin immediately (cannot be blocked).
     * Advances to the next turn.
     */
    void gather();

    /**
     * @brief Tax action: register a pending Tax (2 coins, or 3 if Governor).
     * Must call game->registerTax(this) and advance turn. Coins are awarded
     * when the pending Tax is processed in Game::processPending().
     */
    void tax();

    /**
     * @brief Bribe action: pay 4 coins to register a pending Bribe.
     * Stays on the same turn (extra turn) if not blocked. Pending is processed later.
     */
    void bribe();

    /**
     * @brief Arrest action: register a pending Arrest on target (steal 1 or 2 coins).
     * Advances turn. If role cannot arrest, throws IllegalAction.
     * Coins actually move when pending is resolved.
     * @param target The Player to arrest/steal from.
     */
    void arrest(Player& target);

    /**
     * @brief Sanction action: pay 3 coins, register pending Sanction on target.
     * Advances turn. Sanction deducts 1 coin from target when processed.
     * @param target The Player to sanction.
     */
    void sanction(Player& target);

    /**
     * @brief Coup action: pay 7 coins and register a pending Coup on target.
     * Advances turn. Coup removes target when processed.
     * @param target The Player to coup.
     */
    void coup(Player& target);
    ///@}

    /** @name Hooks & Helpers (called by Game when resolving pending actions) */
    ///@{
    /**
     * @brief Called by Game when this player has been successfully arrested.
     * Invokes the role’s onArrested hook (e.g. General refund, Merchant penalty).
     */
    void handleArrested();

    /**
     * @brief Called by Game when this player has been successfully sanctioned.
     * Invokes the role’s onSanctioned hook (e.g. Baron compensation).
     */
    void handleSanctioned();

    virtual void onSanctioned(Player& target);
    /**
     * @brief Called by Game at the start of this player’s turn.
     * Invokes the role’s onStartTurn hook (e.g. Merchant’s +1 bonus).
     */
    void onStartTurn();
    ///@}

    /** @name Coin Management (internal use) */
    ///@{
    /**
     * @brief Add coins to this player’s balance.
     * @param n Number of coins to add (n ≥ 0).
     */
    void addCoins(int n);

    /**
     * @brief Remove coins from this player’s balance.
     * Throws OutOfCoins if n > current balance.
     * @param n Number of coins to remove.
     */
    void removeCoins(int n);
    ///@}

    /**
     * @brief Change this player’s Role (for future expansion).
     * @param newRole A unique_ptr to the new Role.
     */
    void setRole(std::unique_ptr<Role> newRole);

    /**
     * @brief Clone this player’s role (helper for copy constructor/assignment).
     * @return A new unique_ptr<Role> cloned from the current role.
     */
    std::unique_ptr<Role> cloneRole() const { return _role->clone(); }
};
