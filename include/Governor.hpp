#pragma once

#include "Role.hpp"
#include "Player.hpp"
#include "Game.hpp"

/**
 * @brief The Governor role:
 *   - canTax(): collects 3 coins instead of the standard 2.
 *   - blockTax(): can block another player’s Tax.
 */
class Governor : public Role,Player {
public:
    Governor();
    ~Governor() override = default;

    /// @copydoc Role::canTax
    bool canTax() const override { return true; }

    /// @copydoc Role::blockTax
    void blockTax(Player& target) override;

    /// @copydoc Role::specialAction — no extra code; Game sees canTax() to allow 3-coin tax.
    void specialAction(Player& /*self*/, Player& /*target*/) override { }

    /// @copydoc Role::name
    std::string name() const override { return "Governor"; }

    /// @copydoc Role::clone
    std::unique_ptr<Role> clone() const override {
        return std::make_unique<Governor>(*this);
    }
};
