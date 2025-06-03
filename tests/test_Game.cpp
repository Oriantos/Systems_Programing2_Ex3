#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/Game.hpp"
#include "../include/Player.hpp"
#include "../include/Governor.hpp"
#include "../include/Spy.hpp"
#include "../include/Exceptions.hpp"

//
// Test addPlayer, players(), and duplicate name error.
//
TEST_CASE("Game: addPlayer and duplicate detection") {
    Game game;
    Player a("A", std::make_unique<Governor>(), &game);
    Player b("B", std::make_unique<Spy>(), &game);
    game.addPlayer(&a);
    game.addPlayer(&b);
    auto names = game.players();
    REQUIRE(names.size() == 2);
    CHECK(names[0] == "A");
    CHECK(names[1] == "B");

    // Duplicate name
    Player dup("A", std::make_unique<Spy>(), &game);
    CHECK_THROWS_AS(game.addPlayer(&dup), IllegalAction);
}

//
// Test turn(), nextTurn(), removePlayer(), and winner() in normal flows.
//
TEST_CASE("Game: turn, nextTurn cycle, removePlayer, winner") {
    Game game;
    Player a("A", std::make_unique<Governor>(), &game);
    Player b("B", std::make_unique<Spy>(), &game);
    Player c("C", std::make_unique<Baron>(), &game);
    game.addPlayer(&a);
    game.addPlayer(&b);
    game.addPlayer(&c);

    CHECK(game.turn() == "A");
    game.nextTurn();
    CHECK(game.turn() == "B");
    game.nextTurn();
    CHECK(game.turn() == "C");
    game.nextTurn();
    CHECK(game.turn() == "A");

    // Remove B
    game.removePlayer(&b);
    auto names = game.players();
    REQUIRE(names.size() == 2);
    CHECK(names[0] == "A");
    CHECK(names[1] == "C");

    // If winner when more than one remains
    CHECK_THROWS_AS(game.winner(), GameStillActive);

    // Remove C
    game.removePlayer(&c);
    auto names2 = game.players();
    REQUIRE(names2.size() == 1);
    CHECK(names2[0] == "A");
    CHECK(game.winner() == "A");
}

//
// Test coin pool methods: takeFromPool, returnToPool.
//
TEST_CASE("Game: coin pool and errors") {
    Game game;
    int initial = game.poolCoins();
    REQUIRE(initial == 50);

    game.takeFromPool(10);
    CHECK(game.poolCoins() == initial - 10);

    game.returnToPool(5);
    CHECK(game.poolCoins() == initial - 5);

    // Taking more than available → error
    CHECK_THROWS_AS(game.takeFromPool(1000), IllegalAction);
}

//
// Test register and blocking flows directly via Game methods.
//
TEST_CASE("Game: register and blocking Tax/Bribe/Arrest/Sanction/Coup") {
    Game game;
    Player gov("Gov", std::make_unique<Governor>(), &game);
    Player spy("Spy", std::make_unique<Spy>(), &game);
    Player briber("Briber", std::make_unique<Governor>(), &game); // treat as bribe-capable
    Player baron("Baron", std::make_unique<Baron>(), &game);
    Player general("Gen", std::make_unique<General>(), &game);
    Player judge("Judge", std::make_unique<Judge>(), &game);

    game.addPlayer(&gov);
    game.addPlayer(&spy);
    game.addPlayer(&briber);
    game.addPlayer(&baron);
    game.addPlayer(&general);
    game.addPlayer(&judge);

    // 1) Test Tax + blockTax
    spy.addCoins(3);
    game.nextTurn(); // spy’s turn
    // Temporarily give spy tax ability
    class Taxer : public Role {
    public:
        std::unique_ptr<Role> clone() const override { return std::make_unique<Taxer>(*this); }
        bool canTax() const override { return true; }
        void specialAction(Player& /*self*/, Player& /*target*/) override { }
        std::string name() const override { return "Taxer"; }
    };
    Player* tempSpy = new Player("Spy", std::make_unique<Taxer>(), &game);
    game.removePlayer(&spy);
    game.addPlayer(tempSpy);

    game.nextTurn(); // tempSpy’s turn
    tempSpy->tax();
    game.nextTurn(); // Gov’s turn
    gov.blockTax(*tempSpy);
    game.nextTurn(); // process blocked
    CHECK(tempSpy->coins() == 3); // unchanged
    delete tempSpy;

    // 2) Test Bribe + blockBribe
    briber.addCoins(4);
    game.nextTurn(); // briber’s turn
    briber.bribe();
    game.nextTurn(); // judge’s turn
    judge.blockBribe(briber);
    game.nextTurn(); // process blocked → pool gets back 4, briber has 0
    CHECK(briber.coins() == 0);

    // 3) Test Arrest + blockArrest
    baron.addCoins(2);
    game.nextTurn(); // baron’s turn
    baron.arrest(general);
    game.nextTurn(); // general’s turn
    spy.blockArrest(general);
    game.nextTurn(); // process blocked
    CHECK(general.coins() == 0);
    CHECK(baron.coins() == 2);

    // 4) Test Sanction + blockSanction
    baron.addCoins(3);
    game.nextTurn(); // baron’s turn
    baron.sanction(general);
    game.nextTurn(); // general’s turn
    judge.blockSanction(general); // not implemented, skip
    game.nextTurn(); // nothing happens

    // 5) Test Coup + blockCoup
    baron.addCoins(7);
    game.nextTurn(); // baron’s turn
    baron.coup(general);
    game.nextTurn(); // general’s turn
    general.addCoins(5);
    general.blockCoup(general);
    game.nextTurn(); // process blocked → general remains
    CHECK(game.players().size() == 6);
}

//
// Test processPending: ensure repeated calls to nextTurn handle pending in order.
//
TEST_CASE("Game: processPending with mixed actions sequence") {
    Game game;
    Player a("A", std::make_unique<Governor>(), &game);
    Player b("B", std::make_unique<Spy>(), &game);
    Player c("C", std::make_unique<Baron>(), &game);
    game.addPlayer(&a);
    game.addPlayer(&b);
    game.addPlayer(&c);

    // A gathers
    a.gather();      // A: +1, turn→B
    CHECK(a.coins() == 1);

    // B taxes
    // Temporarily give b tax ability
    class Taxer2 : public Role {
    public:
        std::unique_ptr<Role> clone() const override { return std::make_unique<Taxer2>(*this); }
        bool canTax() const override { return true; }
        void specialAction(Player& /*self*/, Player& /*target*/) override { }
        std::string name() const override { return "Taxer2"; }
    };
    Player* tempB = new Player("B", std::make_unique<Taxer2>(), &game);
    game.removePlayer(&b);
    game.addPlayer(tempB);

    tempB->addCoins(0);
    tempB->tax();    // pending tax
    CHECK(tempB->coins() == 0);
    game.nextTurn(); // C’s turn
    game.nextTurn(); // process tax → B: +2
    CHECK(tempB->coins() == 2);

    delete tempB;
}

//
// Test multiple removePlayer calls and index adjustments:
//
TEST_CASE("Game: removePlayer adjust currentIndex properly") {
    Game game;
    Player a("A", std::make_unique<Governor>(), &game);
    Player b("B", std::make_unique<Spy>(), &game);
    Player c("C", std::make_unique<Baron>(), &game);
    Player d("D", std::make_unique<Merchant>(), &game);
    game.addPlayer(&a);
    game.addPlayer(&b);
    game.addPlayer(&c);
    game.addPlayer(&d);

    // Turn order: A → B → C → D → A ...
    CHECK(game.turn() == "A");
    game.nextTurn();
    CHECK(game.turn() == "B");

    // Remove B while it's B’s turn
    game.removePlayer(&b);
    auto names = game.players();
    REQUIRE(names == std::vector<std::string>{"A", "C", "D"});

    // Next turn should be C (since B was removed)
    CHECK(game.turn() == "C");

    game.nextTurn(); // → D
    CHECK(game.turn() == "D");

    // Remove A out-of-turn
    game.removePlayer(&a);
    auto names2 = game.players();
    REQUIRE(names2 == std::vector<std::string>{"C", "D"});

    // Current index adjustments: if index was past removed index, shift back
    CHECK(game.turn() == "D"); // because currentIndex was pointing at D
}

//
// Test winner() throws when more than one remains, returns correct name otherwise.
//
TEST_CASE("Game: winner logic for multiple and single") {
    Game game;
    Player a("A", std::make_unique<Governor>(), &game);
    Player b("B", std::make_unique<Spy>(), &game);
    Player c("C", std::make_unique<Baron>(), &game);
    game.addPlayer(&a);
    game.addPlayer(&b);
    game.addPlayer(&c);

    CHECK_THROWS_AS(game.winner(), GameStillActive);

    game.removePlayer(&b);
    CHECK_THROWS_AS(game.winner(), GameStillActive);

    game.removePlayer(&a);
    CHECK(game.winner() == "C");
}
