#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include "../include/Game.hpp"
#include "../include/Player.hpp"
#include "../include/Governor.hpp"
#include "../include/Spy.hpp"
#include "../include/Baron.hpp"
#include "../include/General.hpp"
#include "../include/Judge.hpp"
#include "../include/Merchant.hpp"

/**
 * @brief Entry point for the Coup-style game with an SFML GUI.
 * 
 * - Displays each player’s name, role, and coin count.
 * - Highlights the current turn’s player in red.
 * - At the top, shows keyboard controls for each action/block.
 * - At the bottom, shows a “Message:” line with success or exception.
 * 
 * Keyboard mappings:
 *   1 → Gather
 *   2 → Tax
 *   3 → Bribe
 *   4 → Arrest (then press target index key, e.g. “2” for second player)
 *   5 → Sanction (then press target index)
 *   6 → Coup (then press target index)
 *   Q → Governor blocks Tax on current player (if pending)
 *   W → Spy blocks Arrest on current player (if pending)
 *   E → Baron “onSanctioned” compensation
 *   R → General blocks Coup on current player (if pending)
 *   T → Judge blocks Bribe on current player (if pending)
 */
int main() {
    // Load TrueType font (ensure arial.ttf is in project root)
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error: could not load font 'arial.ttf'. Exiting.\n";
        return 1;
    }

    // Create Game & Players
    Game game;
    Player* alice = new Player("Alice", std::make_unique<Governor>(), &game);
    Player* bob   = new Player("Bob",   std::make_unique<Spy>(),      &game);
    Player* carol = new Player("Carol", std::make_unique<Baron>(),    &game);
    Player* dave  = new Player("Dave",  std::make_unique<Merchant>(), &game);
    Player* eve   = new Player("Eve",   std::make_unique<General>(),  &game);
    Player* frank = new Player("Frank", std::make_unique<Judge>(),    &game);

    game.addPlayer(alice);
    game.addPlayer(bob);
    game.addPlayer(carol);
    game.addPlayer(dave);
    game.addPlayer(eve);
    game.addPlayer(frank);

    sf::RenderWindow window(sf::VideoMode(800, 600), "Coup-Style Game (SFML GUI)");

    bool waitingForTarget = false;
    ActionType pendingAction = ActionType::Gather;
    std::vector<Player*> players;
    Player* currentPlayer = nullptr;
    std::string message = "Welcome to Coup GUI!";

    while (window.isOpen()) {
        // Update active player pointers & current player
        players.clear();
        for (auto& nm : game.players()) {
            for (auto* pnt : {alice,bob,carol,dave,eve,frank}) {
                if (pnt->name() == nm) {
                    players.push_back(pnt);
                    break;
                }
            }
        }
        currentPlayer = nullptr;
        for (auto* p : players) {
            if (p->name() == game.turn()) {
                currentPlayer = p;
                break;
            }
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                int code = event.key.code;

                // If not waiting for a target index:
                if (!waitingForTarget) {
                    if (code == sf::Keyboard::Num1) {
                        // Gather
                        try {
                            currentPlayer->gather();
                            message = currentPlayer->name() + " did Gather.";
                        } catch (const std::exception& e) {
                            message = e.what();
                        }
                    }
                    else if (code == sf::Keyboard::Num2) {
                        // Tax
                        try {
                            currentPlayer->tax();
                            message = currentPlayer->name() + " did Tax.";
                        } catch (const std::exception& e) {
                            message = e.what();
                        }
                    }
                    else if (code == sf::Keyboard::Num3) {
                        // Bribe
                        try {
                            currentPlayer->bribe();
                            message = currentPlayer->name() + " did Bribe.";
                        } catch (const std::exception& e) {
                            message = e.what();
                        }
                    }
                    else if (code == sf::Keyboard::Num4) {
                        // Arrest → select target
                        waitingForTarget = true;
                        pendingAction = ActionType::Arrest;
                        message = "Press target index (1–" + std::to_string(players.size()) + ")";
                    }
                    else if (code == sf::Keyboard::Num5) {
                        // Sanction → select target
                        waitingForTarget = true;
                        pendingAction = ActionType::Sanction;
                        message = "Press target index (1–" + std::to_string(players.size()) + ")";
                    }
                    else if (code == sf::Keyboard::Num6) {
                        // Coup → select target
                        waitingForTarget = true;
                        pendingAction = ActionType::Coup;
                        message = "Press target index (1–" + std::to_string(players.size()) + ")";
                    }
                    else if (code == sf::Keyboard::Q) {
                        // Governor blocks Tax on current player
                        try {
                            for (auto* pnt : {alice,bob,carol,dave,eve,frank}) {
                                if (pnt->roleName() == "Governor") {
                                    pnt->blockTax(*currentPlayer);
                                    message = "Governor blocked Tax on " + currentPlayer->name();
                                    break;
                                }
                            }
                        } catch (const std::exception& e) {
                            message = e.what();
                        }
                    }
                    else if (code == sf::Keyboard::W) {
                        // Spy blocks Arrest on current player
                        try {
                            for (auto* pnt : {alice,bob,carol,dave,eve,frank}) {
                                if (pnt->roleName() == "Spy") {
                                    pnt->blockArrest(*currentPlayer);
                                    message = "Spy blocked Arrest on " + currentPlayer->name();
                                    break;
                                }
                            }
                        } catch (const std::exception& e) {
                            message = e.what();
                        }
                    }
                    else if (code == sf::Keyboard::E) {
                        // Baron compensation (onSanctioned)
                        try {
                            for (auto* pnt : {alice,bob,carol,dave,eve,frank}) {
                                if (pnt->roleName() == "Baron") {
                                    pnt->handleSanctioned();
                                    message = "Baron received sanction compensation.";
                                    break;
                                }
                            }
                        } catch (const std::exception& e) {
                            message = e.what();
                        }
                    }
                    else if (code == sf::Keyboard::R) {
                        // General blocks Coup on current player
                        try {
                            for (auto* pnt : {alice,bob,carol,dave,eve,frank}) {
                                if (pnt->roleName() == "General") {
                                    pnt->blockCoup(*currentPlayer);
                                    message = "General blocked Coup on " + currentPlayer->name();
                                    break;
                                }
                            }
                        } catch (const std::exception& e) {
                            message = e.what();
                        }
                    }
                    else if (code == sf::Keyboard::T) {
                        // Judge blocks Bribe on current player
                        try {
                            for (auto* pnt : {alice,bob,carol,dave,eve,frank}) {
                                if (pnt->roleName() == "Judge") {
                                    pnt->blockBribe(*currentPlayer);
                                    message = "Judge blocked Bribe on " + currentPlayer->name();
                                    break;
                                }
                            }
                        } catch (const std::exception& e) {
                            message = e.what();
                        }
                    }
                }
                // If waiting for a target index:
                else {
                    int idx = -1;
                    if (code >= sf::Keyboard::Num1 && code <= sf::Keyboard::Num9) {
                        idx = code - sf::Keyboard::Num1; // zero-based
                    }
                    if (idx >= 0 && idx < static_cast<int>(players.size())) {
                        Player* target = players[idx];
                        if (target == currentPlayer) {
                            message = "Cannot target yourself. Action cancelled.";
                        } else {
                            try {
                                switch (pendingAction) {
                                    case ActionType::Arrest:
                                        currentPlayer->arrest(*target);
                                        message = currentPlayer->name() + " arrested " + target->name();
                                        break;
                                    case ActionType::Sanction:
                                        currentPlayer->sanction(*target);
                                        message = currentPlayer->name() + " sanctioned " + target->name();
                                        break;
                                    case ActionType::Coup:
                                        currentPlayer->coup(*target);
                                        message = currentPlayer->name() + " couped " + target->name();
                                        break;
                                    default:
                                        break;
                                }
                            } catch (const std::exception& e) {
                                message = e.what();
                            }
                        }
                        waitingForTarget = false;
                    }
                    else {
                        message = "Invalid target index. Action cancelled.";
                        waitingForTarget = false;
                    }
                }
            }
        }

        window.clear(sf::Color::White);

        // Draw action instructions at top
        {
            sf::Text txt;
            txt.setFont(font);
            txt.setCharacterSize(20);
            txt.setFillColor(sf::Color::Black);
            txt.setString(
                "1:Gather  2:Tax  3:Bribe  4:Arrest  5:Sanction  6:Coup\n"
                "Q:Gov blockTax  W:Spy blockArrest  E:Baron comp  R:Gen blockCoup  T:Judge blockBribe"
            );
            txt.setPosition(20.f, 5.f);
            window.draw(txt);
        }

        // Draw each player’s info (index, name, role, coins). Highlight current in red.
        float y = 100.f;
        for (size_t i = 0; i < players.size(); ++i) {
            Player* p = players[i];
            sf::Text txt;
            txt.setFont(font);
            txt.setCharacterSize(18);
            std::string line = std::to_string(i+1) + ") " + p->name()
                             + " [" + p->roleName() + "] : "
                             + std::to_string(p->coins()) + " coins";
            txt.setString(line);
            if (p == currentPlayer) {
                txt.setFillColor(sf::Color::Red);
            } else {
                txt.setFillColor(sf::Color::Black);
            }
            txt.setPosition(20.f, y);
            window.draw(txt);
            y += 30.f;
        }

        // Draw message at bottom
        {
            sf::Text txt;
            txt.setFont(font);
            txt.setCharacterSize(18);
            txt.setFillColor(sf::Color::Blue);
            txt.setString("Message: " + message);
            txt.setPosition(20.f, 500.f);
            window.draw(txt);
        }

        window.display();
    }

    // Clean up dynamically allocated players
    delete alice;
    delete bob;
    delete carol;
    delete dave;
    delete eve;
    delete frank;

    return 0;
}
