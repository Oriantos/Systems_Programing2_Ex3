#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
// tests/test_Governor.cpp
#define DOCTEST_CONFIG_INCLUDE_GUARD
#include "doctest.h"
#include "../include/Game.hpp"
#include "../include/Player.hpp"
#include "../include/Governor.hpp"
#include "../include/Spy.hpp"
#include "../include/Exceptions.hpp"

//
// Test Governor’s canTax and blockTax functionality:
//
TEST_CASE("Governor: canTax and successful blockTax") {
    Game game;
    Player gov("Gov", std::make_unique<Governor>(), &game);
    Player bob("Bob", std::make_unique<Spy>(), &game);

    game.addPlayer(&gov);
    game.addPlayer(&bob);

    // Give Bob tax ability temporarily
    class Taxer3 : public Role {
    public:
        std::unique_ptr<Role> clone() const override { return std::make_unique<Taxer3>(*this); }
        bool canTax() const override { return true; }
        void specialAction(Player& /*self*/, Player& /*target*/) override { }
        std::string name() const override { return "Taxer3"; }
    };
    Player* tempBob = new Player("Bob", std::make_unique<Taxer3>(), &game);
    game.removePlayer(&bob);
    game.addPlayer(tempBob);

    tempBob->addCoins(0);
    game.nextTurn(); // tempBob’s turn
    tempBob->tax();
    game.nextTurn(); // gov’s turn
    gov.blockTax(*tempBob);
    game.nextTurn(); // process blocked
    CHECK(tempBob->coins() == 0);
    delete tempBob;
}

//
// Test that blockTax throws if no pending Tax
//
TEST_CASE("Governor: blockTax with no pending throws") {
    Game game;
    Player gov("Gov", std::make_unique<Governor>(), &game);
    Player bob("Bob", std::make_unique<Spy>(), &game);
    game.addPlayer(&gov);
    game.addPlayer(&bob);

    CHECK_THROWS_AS(gov.blockTax(bob), IllegalAction);
}
