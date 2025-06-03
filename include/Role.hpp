#pragma  once

#include <string>
#include <memory>
#include "Exceptions.hpp"
#include "ActionType.hpp"

class Player;   // forward declaration



/**
 * @brief Abstract base class for all roles in the Coup-style game.
 * 
 * Each concrete role overrides which actions it can take, which actions
 * it can block, and any special hooks (e.g., onArrested, onSanctioned).
 */
class Role {
public:
    virtual ~Role() = default;

    /**
     * @brief Create a deep copy of this Role.
     * @return A unique_ptr owning a newly allocated copy of this Role.
     */
    virtual std::unique_ptr<Role> clone() const = 0;

    /** @name Abilities (override in subclasses) */
    ///@{
    virtual bool canGather()    const { return true; }   ///< Everyone can gather.
    virtual bool canTax()       const { return false; }
    virtual bool canBribe()     const { return false; }
    virtual bool canArrest()    const { return false; }
    virtual bool canSanction()  const { return false; }
    virtual bool canCoup()      const { return false; }   ///< Anyone can Coup if ≥7 coins.
    ///@}

    /** @name Blocking powers (by default, illegal) */
    ///@{
    virtual void blockGather(Player& /*target*/)   { throw IllegalAction("Cannot block gather"); }
    virtual void blockTax(Player& /*target*/)      { throw IllegalAction("Cannot block tax"); }
    virtual void blockBribe(Player& /*target*/)    { throw IllegalAction("Cannot block bribe"); }
    virtual void blockArrest(Player& /*target*/)   { throw IllegalAction("Cannot block arrest"); }
    virtual void blockSanction(Player& /*target*/) { throw IllegalAction("Cannot block sanction"); }
    virtual void blockCoup(Player& /*target*/)     { throw IllegalAction("Cannot block coup"); }
    ///@}

    /** @name Hooks called when certain actions resolve (default no-op) */
    ///@{
    /**
     * @brief Called when this role’s owner has been successfully arrested.
     * @param self The Player who owns this role.
     */
    virtual void onArrested(Player& self)   { (void)self; }

    /**
     * @brief Called when this role’s owner has been successfully sanctioned.
     * @param self The Player who owns this role.
     */
    virtual void onSanctioned(Player& self) { (void)self; }

    /**
     * @brief Called at the start of the owner’s turn (e.g., Merchant bonus).
     * @param self The Player who owns this role.
     */
    virtual void onStartTurn(Player& self)  { (void)self; }
    ///@}

    /**
     * @brief Perform the role’s special action (e.g., Baron’s invest).
     * @param self   The Player performing the action.
     * @param target The target Player (may be self or another).
     */
    virtual void specialAction(Player& self, Player& target) = 0;

    /**
     * @brief Returns the role’s name as a string (for display).
     * @return The name of the role.
     */
    virtual std::string name() const = 0;
};
