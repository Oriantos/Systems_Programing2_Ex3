#pragma once

#include "Role.hpp"
#include "Player.hpp"


/**
 * @brief The Baron role:
 *   - specialAction(): “Invest” — pay 3 coins and immediately gain 6.
 *   - onSanctioned(): if sanctioned, gain +1 coin compensation.
 */
class Baron : public Role, virtual Player {
public:
    Baron();
    ~Baron() override = default;

    /**
     * @brief Pay 3 coins to gain 6. Throws OutOfCoins if < 3 coins.
     * @param self   The Player performing the invest.
     * @param target Ignored (Baron’s invest does not target another player).
     */
    void specialAction(Player& self, Player& /*target*/) override;

    /**
     * @brief When sanctioned, the Baron gains +1 coin instead of losing a coin.
     * @param self The Player (Baron) who was sanctioned.
     */
    void onSanctioned(Player& self) override;

    /// @copydoc Role::name
    std::string name() const override { return "Baron"; }

    /// @copydoc Role::clone
    std::unique_ptr<Role> clone() const override {
        return std::make_unique<Baron>(*this);
    }
};
