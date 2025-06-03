#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "Judge.hpp"
#include "../include/Game.hpp"
#include "../include/Player.hpp"
#include "../include/Baron.hpp"
#include "../include/Exceptions.hpp"

//
// Test Baron’s invest (specialAction) success and insufficient-coins failure
//
TEST_CASE("Baron: invest and out of coins") {
    Game game;
    Player carol("Carol", std::make_unique<Baron>(), &game);
    game.addPlayer(&carol);

    // Not enough coins → OutOfCoins
    CHECK_THROWS_AS(carol.specialAction(carol, carol), OutOfCoins);

    // Give Carol 3 coins → invest
    carol.addCoins(3);
    CHECK_NOTHROW(carol.specialAction(carol, carol));
    CHECK(carol.coins() == 6);
}

//
// Test Baron’s onSanctioned compensation
//
TEST_CASE("Baron: onSanctioned gives +1") {
    Game game;
    Player baron("Baron", std::make_unique<Baron>(), &game);
    Player judge("Judge", std::make_unique<Judge>(), &game);

    game.addPlayer(&baron);
    game.addPlayer(&judge);

    // Judge sanctions Baron
    judge.addCoins(3);
    game.nextTurn(); // Judge’s turn
    judge.sanction(baron);
    game.nextTurn(); // Baron’s turn
    game.nextTurn(); // process sanction
    CHECK(baron.coins() == 1); // lost 1 then gained 1
}
