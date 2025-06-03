#pragma once

#include "Role.hpp"
#include "Player.hpp"

/**
 * @brief The Merchant role:
 *   - onStartTurn(): at the beginning of their turn, if they have ≥3 coins, gain +1 coin.
 *   - onArrested(): if arrested, lose 2 coins instead of 1 (or all if <2).
 */
class Merchant : public Role {
public:
    Merchant();
    ~Merchant() override = default;

    /**
     * @brief Called at the start of this player’s turn.
     * If they have at least 3 coins, they receive +1 automatic bonus.
     * @param self The Player (Merchant) whose turn is starting.
     */
    void onStartTurn(Player& self) override;

    /**
     * @brief Called when this player is arrested.
     * If they have ≥2 coins, they lose 2; otherwise, they lose as many as they have.
     * @param self The Player (Merchant) who was arrested.
     */
    void onArrested(Player& self) override;

    /// @copydoc Role::specialAction — no extra code for Merchant.
    void specialAction(Player& /*self*/, Player& /*target*/) override { }

    /// @copydoc Role::name
    std::string name() const override { return "Merchant"; }

    /// @copydoc Role::clone
    std::unique_ptr<Role> clone() const override {
        return std::make_unique<Merchant>(*this);
    }
};
