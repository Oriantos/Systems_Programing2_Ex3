#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/Game.hpp"
#include "../include/Player.hpp"
#include "../include/Judge.hpp"
#include "../include/Baron.hpp"
#include "../include/Exceptions.hpp"

//
// Test Judge’s blockBribe and sanction blocking behavior
//
TEST_CASE("Judge: blockBribe and sanction logic") {
    Game game;
    Player judge("Judge", std::make_unique<Judge>(), &game);
    Player baron("Baron", std::make_unique<Baron>(), &game);

    game.addPlayer(&judge);
    game.addPlayer(&baron);

    // Temporarily give baron bribe ability
    class Briber : public Role {
    public:
        std::unique_ptr<Role> clone() const override { return std::make_unique<Briber>(*this); }
        bool canBribe() const override { return true; }
        void specialAction(Player& /*self*/, Player& /*target*/) override { }
        std::string name() const override { return "Briber"; }
    };

    Player alice("Alice", std::make_unique<Briber>(), &game);
    game.addPlayer(&alice);

    alice.addCoins(4);
    game.nextTurn(); // Alice’s turn
    alice.bribe();
    CHECK(alice.coins() == 0);
    game.nextTurn(); // judge’s turn

    CHECK_NOTHROW(judge.blockBribe(alice));
    CHECK(alice.coins() == 0);
    game.nextTurn();

    // Test sanction on judge
    alice.addCoins(3);
    game.nextTurn(); // Alice’s turn
    alice.sanction(judge);
    game.nextTurn(); // Judge’s turn
    game.nextTurn(); // process
    CHECK(judge.coins() == 0);

    // Test blockSanction: judge blocking no pending throws
    CHECK_THROWS_AS(judge.blockSanction(judge), IllegalAction);
}

//
// Test failure if no pending bribe to block
//
TEST_CASE("Judge: blockBribe no pending throws") {
    Game game;
    Player judge("Judge", std::make_unique<Judge>(), &game);
    Player baron("Baron", std::make_unique<Baron>(), &game);
    game.addPlayer(&judge);
    game.addPlayer(&baron);

    CHECK_THROWS_AS(judge.blockBribe(baron), IllegalAction);
}
