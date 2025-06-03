#pragma once

#include "Role.hpp"
#include "Player.hpp"
#include "Game.hpp"
#include <iostream>

/**
 * @brief The Spy role:
 *   - specialAction(): “Look at” another player’s coin count (prints to stdout).
 *   - blockArrest(): can block another player’s Arrest.
 */
class Spy : public Role,Player {
public:
    Spy();
    ~Spy() override = default;

    /// @copydoc Role::canArrest — Spy does NOT perform arrest, only blocks it.
    bool canArrest() const override { return false; }

    /// @copydoc Role::blockArrest
    void blockArrest(Player& target) override;

    /**
     * @brief Reveal target’s coin count to stdout.
     * @param self   The Spy performing the action.
     * @param target The Player whose coins are revealed.
     */
    void specialAction(Player& self, Player& target) override;

    /// @copydoc Role::name
    std::string name() const override { return "Spy"; }

    /// @copydoc Role::clone
    std::unique_ptr<Role> clone() const override {
        return std::make_unique<Spy>(*this);
    }
};
