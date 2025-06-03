#pragma once

#include "Role.hpp"
#include "Player.hpp"
#include "Game.hpp"
#include "ActionType.hpp"

/**
 * @brief The Judge role:
 *   - blockBribe(): can cancel another player’s Bribe (returning their 4 coins to the pool).
 *   - onSanctioned(): if sanctioned, the offender (sanctioner) pays +1 coin back to the pool.
 */
class Judge : public Role,Player {
public:
    Judge();
    ~Judge() override = default;

    /// @copydoc Role::canBribe — false here; Judge only blocks bribes.
    bool canBribe() const override { return false; }

    /// @copydoc Role::blockBribe
    void blockBribe(Player& target) override;

    /// @copydoc Role::onSanctioned — understanding that Game’s blockSanction handles extra payment.
    void onSanctioned(Player& /*self*/) override;

    /// @copydoc Role::specialAction — no extra code.
    void specialAction(Player& /*self*/, Player& /*target*/) override { }

    /// @copydoc Role::name
    std::string name() const override { return "Judge"; }

    /// @copydoc Role::clone
    std::unique_ptr<Role> clone() const override {
        return std::make_unique<Judge>(*this);
    }
};
