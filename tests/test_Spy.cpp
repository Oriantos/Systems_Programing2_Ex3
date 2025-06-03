#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/Game.hpp"
#include "../include/Player.hpp"
#include "../include/Spy.hpp"
#include "../include/Governor.hpp"
#include "../include/Baron.hpp"
#include "../include/Exceptions.hpp"

//
// Test Spy’s specialAction just prints coins; we’ll ensure no exception thrown:
//
TEST_CASE("Spy: specialAction prints coins") {
    Game game;
    Player spy("Spy", std::make_unique<Spy>(), &game);
    Player carol("Carol", std::make_unique<Governor>(), &game);

    game.addPlayer(&spy);
    game.addPlayer(&carol);

    carol.addCoins(5);
    CHECK_NOTHROW(spy.specialAction(spy, carol));
}

//
// Test Spy’s blockArrest when pending vs. no pending
//
TEST_CASE("Spy: blockArrest success and failure") {
    Game game;
    Player spy("Spy", std::make_unique<Spy>(), &game);
    Player baron("Baron", std::make_unique<Baron>(), &game); // can arrest
    Player carol("Carol", std::make_unique<Baron>(), &game);

    game.addPlayer(&spy);
    game.addPlayer(&baron);
    game.addPlayer(&carol);

    carol.addCoins(2);

    // Baron arrests Carol
    game.nextTurn(); // Baron’s turn
    baron.arrest(carol);

    // Next, Carol’s turn: Spy blocks
    game.nextTurn(); // Carol’s turn
    CHECK_NOTHROW(spy.blockArrest(carol));
    game.nextTurn(); // process blocked
    CHECK(carol.coins() == 2);
    CHECK(baron.coins() == 0);

    // No pending Arrest now; blockArrest should throw
    CHECK_THROWS_AS(spy.blockArrest(carol), IllegalAction);
}
