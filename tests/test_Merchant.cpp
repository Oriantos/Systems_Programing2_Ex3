#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/Game.hpp"
#include "../include/Player.hpp"
#include "../include/Merchant.hpp"
#include "../include/Governor.hpp"
#include "../include/Exceptions.hpp"

//
// Test Merchant’s onStartTurn bonus and onArrested penalty
//
TEST_CASE("Merchant: onStartTurn bonus and onArrested") {
    Game game;
    Player mer("Mer", std::make_unique<Merchant>(), &game);
    Player gov("Gov", std::make_unique<Governor>(), &game);
    game.addPlayer(&mer);
    game.addPlayer(&gov);

    // Give Mer 3 coins; onStartTurn → +1
    mer.addCoins(3);
    mer.onStartTurn();
    CHECK(mer.coins() == 4);

    // Arrest Mer: should lose 2
    gov.addCoins(3);
    game.nextTurn(); // Gov’s turn
    gov.arrest(mer);
    game.nextTurn(); // process arrest
    CHECK(mer.coins() == 2);
    CHECK(gov.coins() == 2);
}

//
// Test that Merchant’s onStartTurn does nothing when <3 coins.
//
TEST_CASE("Merchant: no bonus if <3 coins") {
    Game game;
    Player mer("Mer", std::make_unique<Merchant>(), &game);
    Player gov("Gov", std::make_unique<Governor>(), &game);
    game.addPlayer(&mer);
    game.addPlayer(&gov);

    mer.addCoins(2);
    mer.onStartTurn();
    CHECK(mer.coins() == 2);
}
