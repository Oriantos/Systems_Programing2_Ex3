#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/Game.hpp"
#include "../include/Player.hpp"
#include "../include/General.hpp"
#include "../include/Baron.hpp"
#include "../include/Exceptions.hpp"

//
// Test General’s blockCoup happy path and failure path
//
TEST_CASE("General: blockCoup success and failure") {
    Game game;
    Player general("Gen", std::make_unique<General>(), &game);
    Player carol("Carol", std::make_unique<Baron>(), &game);

    game.addPlayer(&general);
    game.addPlayer(&carol);

    carol.addCoins(7);
    game.nextTurn(); // Carol’s turn
    carol.coup(general);

    game.nextTurn(); // General’s turn
    general.addCoins(5);
    CHECK_NOTHROW(general.blockCoup(general));
    game.nextTurn(); // process blocked → general remains
    CHECK(game.players().size() == 2);

    CHECK_THROWS_AS(general.blockCoup(general), IllegalAction);

    // Test insufficient coins
    carol.addCoins(7);
    game.nextTurn(); // Carol’s turn again
    carol.coup(general);
    game.nextTurn(); // General’s turn
    CHECK_THROWS_AS(general.blockCoup(general), OutOfCoins);
}

//
// Test General’s onArrested refund
//
TEST_CASE("General: onArrested refund 1 coin") {
    Game game;
    Player general("Gen", std::make_unique<General>(), &game);
    Player baron("Baron", std::make_unique<Baron>(), &game);

    game.addPlayer(&general);
    game.addPlayer(&baron);

    baron.addCoins(3);
    game.nextTurn(); // baron’s turn
    baron.arrest(general);
    game.nextTurn(); // process arrest
    CHECK(general.coins() == 1);
}
