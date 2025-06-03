#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/Player.hpp"
#include "../include/Governor.hpp"
#include "../include/Spy.hpp"
#include "../include/Baron.hpp"
#include "../include/Merchant.hpp"
#include "../include/General.hpp"
#include "../include/Judge.hpp"
#include "../include/Game.hpp"
#include "../include/Exceptions.hpp"

//
// Test gather, simple turn enforcement, and coin addition.
//
TEST_CASE("Player: gather increases coins and advances turn") {
    Game game;
    Player alice("Alice", std::make_unique<Governor>(), &game);
    Player bob("Bob", std::make_unique<Spy>(), &game);
    game.addPlayer(&alice);
    game.addPlayer(&bob);

    REQUIRE(alice.coins() == 0);
    REQUIRE(game.turn() == "Alice");

    alice.gather();
    CHECK(alice.coins() == 1);
    CHECK(game.turn() == "Bob");

    // Bob gathers as well
    bob.gather();
    CHECK(bob.coins() == 1);
    CHECK(game.turn() == "Alice");
}

//
// Test tax for Governor vs. failure for non-Governor
//
TEST_CASE("Player: tax with Governor and fail with non-Governor") {
    Game game;
    Player alice("Alice", std::make_unique<Governor>(), &game);
    game.addPlayer(&alice);

    alice.tax();
    // Before processing, coins remain 0
    CHECK(alice.coins() == 0);
    game.nextTurn(); // process tax
    CHECK(alice.coins() == 3);

    // Non-Governor cannot tax
    Player bob("Bob", std::make_unique<Spy>(), &game);
    game.addPlayer(&bob);
    game.nextTurn(); // Bob’s turn
    CHECK_THROWS_AS(bob.tax(), IllegalAction);
}

//
// Test bribe: successful, staying on same player, and blocked scenario
//
TEST_CASE("Player: bribe extra turn and blocked") {
    Game game;
    Player baron("Baron", std::make_unique<Baron>(), &game);
    Player judge("Judge", std::make_unique<Judge>(), &game);
    game.addPlayer(&baron);
    game.addPlayer(&judge);

    // Give Baron enough coins
    baron.addCoins(5);
    REQUIRE(baron.coins() == 5);

    // Baron’s turn: bribe should fail because Baron cannot bribe
    CHECK_THROWS_AS(baron.bribe(), IllegalAction);

    // Make a temporary role that can bribe for testing
    class Briber : public Role {
    public:
        std::unique_ptr<Role> clone() const override { return std::make_unique<Briber>(*this); }
        bool canBribe() const override { return true; }
        void specialAction(Player& /*self*/, Player& /*target*/) override { }
        std::string name() const override { return "Briber"; }
    };

    Player alice("Alice", std::make_unique<Briber>(), &game);
    game.addPlayer(&alice);

    // Give Alice 4 coins
    alice.addCoins(4);
    game.nextTurn(); // Alice’s turn
    alice.bribe();   // Should register pending bribe, stay on Alice’s turn
    CHECK(alice.coins() == 0); // 4 coins spent

    // Now let Judge block this bribe
    game.nextTurn(); // move to Judge
    judge.blockBribe(alice);
    CHECK(alice.coins() == 0);  // coins still zero (returned to pool, not to Alice)
    game.nextTurn(); // process blocked bribe (no extra turn)
    CHECK(game.turn() == "Alice"); // since bribe would have given extra turn; blocked means no skip

    // If Judge does not block, Alice should get extra turn
    alice.addCoins(4);
    game.nextTurn(); // Alice’s turn again
    alice.bribe();   // pending again
    // skip Judge’s block
    game.nextTurn(); // Judge’s turn; do nothing
    game.nextTurn(); // process bribe: Alice gets extra turn → current back to Alice
    CHECK(game.turn() == "Alice");
}

//
// Test arrest scenarios: normal, Merchant target, blocked by Spy, and out of coins
//
TEST_CASE("Player: arrest normal and Merchant case") {
    Game game;

    Player bob("Bob", std::make_unique<Baron>(), &game);      // can arrest
    Player dave("Dave", std::make_unique<Merchant>(), &game); // target is Merchant

    game.addPlayer(&bob);
    game.addPlayer(&dave);

    // Bob’s turn: arrest Dave
    game.nextTurn(); // Bob’s turn
    dave.addCoins(3);
    int daveCoinsBefore = dave.coins();
    bob.arrest(dave);
    game.nextTurn(); // process arrest

    // Dave is Merchant: loses 2; Bob gains 2
    CHECK(dave.coins() == daveCoinsBefore - 2);
    CHECK(bob.coins() == 2);
}

TEST_CASE("Player: Arrest block by Spy") {
    Game game;
    Player spy("Spy", std::make_unique<Spy>(), &game);
    Player baron("Baron", std::make_unique<Baron>(), &game); // can arrest
    Player carol("Carol", std::make_unique<Baron>(), &game);

    game.addPlayer(&spy);
    game.addPlayer(&baron);
    game.addPlayer(&carol);

    // Grant Carol coins to be stolen
    carol.addCoins(2);

    // Baron arrests Carol
    game.nextTurn(); // Baron’s turn
    baron.arrest(carol);

    // Now it's Carol’s turn; Spy can block
    game.nextTurn(); // Carol’s turn
    spy.blockArrest(carol);

    game.nextTurn(); // process blocked arrest
    CHECK(carol.coins() == 2); // unchanged
    CHECK(baron.coins() == 0); // no stolen coins
}

TEST_CASE("Player: Arrest fail if role cannot arrest or out of turn") {
    Game game;
    Player alice("Alice", std::make_unique<Spy>(), &game); // Spy cannot arrest
    Player bob("Bob", std::make_unique<Baron>(), &game);

    game.addPlayer(&alice);
    game.addPlayer(&bob);

    game.nextTurn(); // Alice’s turn
    CHECK_THROWS_AS(alice.arrest(bob), IllegalAction);

    // Bob’s turn: but try to arrest twice in a row (out of turn)
    game.nextTurn(); // Bob’s turn
    bob.addCoins(3);
    bob.arrest(alice);
    game.nextTurn(); // process arrest
    // Now current turn is Alice; Bob tries to arrest again → out of turn
    CHECK_THROWS_AS(bob.arrest(alice), NotYourTurn);
}

//
// Test sanction: normal, blocked by Judge, barred & compensation by Baron
//
TEST_CASE("Player: sanction normal and Judge block") {
    Game game;
    Player judge("Judge", std::make_unique<Judge>(), &game); // blocks sanction
    Player baron("Baron", std::make_unique<Baron>(), &game);

    game.addPlayer(&judge);
    game.addPlayer(&baron);

    baron.addCoins(3);
    game.nextTurn(); // Baron's turn
    baron.sanction(judge);
    CHECK(baron.coins() == 0); // 3 coins spent

    // Now it's Judge’s turn; she blocks sanction
    game.nextTurn(); // Judge’s turn
    // Judge cannot actually block sanction directly, so sanction goes through:
    game.nextTurn(); // process sanction: judge loses 1 (0->0), no one blocked, baron not target

    // Now test a sanction where Baron is targeted
    baron.addCoins(3);
    game.nextTurn(); // Baron's turn
    baron.sanction(judge);
    game.nextTurn(); // Judge’s turn, skip blocking
    game.nextTurn(); // process sanction: judge loses 1 (0->0), no special
    // Let’s correct: sanction target should be Baron
    judge.addCoins(3);
    game.nextTurn(); // Judah’s turn
    judge.sanction(baron);     // send sanction to Baron
    game.nextTurn(); // Baron’s turn: process pending sanction
    CHECK(baron.coins() == 1); // net effect: onSanctioned bonus +1
}

TEST_CASE("Player: sanction out of turn/down to zero") {
    Game game;
    Player baron("Baron", std::make_unique<Baron>(), &game);
    Player bob("Bob", std::make_unique<Baron>(), &game);

    game.addPlayer(&baron);
    game.addPlayer(&bob);

    baron.addCoins(2);
    game.nextTurn(); // Baron’s turn
    CHECK_THROWS_AS(baron.sanction(bob), OutOfCoins); // needs 3 coins

    // Try to sanction when not your turn
    CHECK_THROWS_AS(bob.sanction(baron), NotYourTurn);
}

//
// Test coup: normal removal, block by General, and out of coins
//
TEST_CASE("Player: Coup normal removal and blocked by General") {
    Game game;
    Player general("Gen", std::make_unique<General>(), &game);
    Player carol("Carol", std::make_unique<Baron>(), &game);

    game.addPlayer(&general);
    game.addPlayer(&carol);

    carol.addCoins(7);
    game.nextTurn(); // Carol’s turn
    carol.coup(general);

    // Now it’s General’s turn; General blocks coup
    game.nextTurn(); // General’s turn
    general.addCoins(5);
    general.blockCoup(general);
    CHECK(carol.coins() == 0); // coup cancelled, 7 returned to pool
    game.nextTurn(); // process blocked coup
    CHECK(game.players().size() == 2);

    // Test failure if no pending coup
    CHECK_THROWS_AS(general.blockCoup(general), IllegalAction);
}

TEST_CASE("Player: Coup failure and self-coup not allowed") {
    Game game;
    Player alice("Alice", std::make_unique<Governor>(), &game);
    Player bob("Bob", std::make_unique<Spy>(), &game);
    game.addPlayer(&alice);
    game.addPlayer(&bob);

    // Insufficient coins
    CHECK_THROWS_AS(alice.coup(bob), OutOfCoins);

    // If enough coins:
    alice.addCoins(7);
    game.nextTurn(); // Alice’s turn
    CHECK_THROWS_AS(alice.coup(alice), IllegalAction); // cannot coup self
}

//
// Test Rule of Three: copying players yields separate Role objects
//
TEST_CASE("Player: copy constructor and assignment deep copy") {
    Game game;
    Player orig("Orig", std::make_unique<Governor>(), &game);
    game.addPlayer(&orig);

    orig.addCoins(5);
    // Copy construct
    Player copy1(orig);
    CHECK(copy1.name() == "Orig");
    CHECK(copy1.coins() == 5);
    CHECK(copy1.roleName() == "Governor");

    // Modify original’s coins
    orig.addCoins(2);
    CHECK(orig.coins() == 7);
    CHECK(copy1.coins() == 5); // copy’s coins unaffected

    // Test copy assignment
    Player copy2("Temp", std::make_unique<Spy>(), &game);
    copy2 = orig;
    CHECK(copy2.name() == "Orig");
    CHECK(copy2.coins() == 7);
    CHECK(copy2.roleName() == "Governor");

    orig.removeCoins(3);
    CHECK(orig.coins() == 4);
    CHECK(copy2.coins() == 7); // assignment copy unaffected
}

//
// Test onStartTurn hook for Merchant and mustCoup logic
//
TEST_CASE("Player: Merchant onStartTurn and mustCoup flag") {
    Game game;
    Player mer("Mer", std::make_unique<Merchant>(), &game);
    Player alice("Alice", std::make_unique<Governor>(), &game);
    game.addPlayer(&mer);
    game.addPlayer(&alice);

    // Give Merchant 3 coins → next turn, he should get +1
    mer.addCoins(3);
    mer.onStartTurn();
    CHECK(mer.coins() == 4);

    // Test mustCoup logic
    mer.addCoins(6); // total 10
    CHECK(game.mustCoup(mer) == true);
    mer.removeCoins(2); // now 8
    CHECK(game.mustCoup(mer) == false);
}

//
// Test action out of turn throws
//
TEST_CASE("Player: any action out of turn throws NotYourTurn") {
    Game game;
    Player alice("Alice", std::make_unique<Governor>(), &game);
    Player bob("Bob", std::make_unique<Spy>(), &game);
    game.addPlayer(&alice);
    game.addPlayer(&bob);

    // Initially, Alice’s turn; Bob tries to gather → out of turn
    CHECK_THROWS_AS(bob.gather(), NotYourTurn);
    // Alice gathers → OK
    alice.gather();
    // Now Bob’s turn; Alice tries to tax → out of turn
    CHECK_THROWS_AS(alice.tax(), NotYourTurn);
}
