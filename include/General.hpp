#pragma once

#include "Role.hpp"
#include "Player.hpp"
#include "Game.hpp"

/**
 * @brief The General role:
 *   - blockCoup(): can pay 5 coins to cancel another player’s Coup.
 *   - onArrested(): if arrested, immediately refund 1 coin.
 */
class General : public Role, Player {
public:
    General();
    ~General() override = default;

    /// @copydoc Role::canCoup — false here; everyone can coup by default if ≥7 coins.
    bool canCoup() const override { return false; }

    /// @copydoc Role::blockCoup
    void blockCoup(Player& target) override;

    /// @copydoc Role::onArrested
    void onArrested(Player& self) override;

    /// @copydoc Role::specialAction — nothing extra; blockCoup is invoked directly.
    void specialAction(Player& /*self*/, Player& /*target*/) override { }

    /// @copydoc Role::name
    std::string name() const override { return "General"; }

    /// @copydoc Role::clone
    std::unique_ptr<Role> clone() const override {
        return std::make_unique<General>(*this);
    }
};
