#include "stronghold.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(10000, '\n');
}

int getValidChoice(int min, int max, const std::string& prompt) {
    int choice;
    while (true) {
        std::cout << prompt;
        if (std::cin >> choice && choice >= min && choice <= max) {
            clearInputBuffer();
            return choice;
        }
        std::cout << RED << "Invalid choice. Please enter a number between " << min << " and " << max << ".\n" << RESET;
        clearInputBuffer();
    }
}

void displayMenu() {
    std::cout << BOLD << "\n=== Stronghold Game Menu ===\n" << RESET;
    std::cout << "1. Play Turn\n";
    std::cout << "2. Train Army\n";
    std::cout << "3. Hold Election\n";
    std::cout << "4. Manage Loan or Audit\n";
    std::cout << "5. Buy Resource\n";
    std::cout << "6. Manage Diplomacy\n";
    std::cout << "7. Bribe or Blackmail\n";
    std::cout << "8. Send Message\n";
    std::cout << "9. Send Fake Trade Request\n";
    std::cout << "10. View Messages\n";
    std::cout << "11. Upgrade Blacksmith\n";
    std::cout << "12. Produce Weapons\n";
    std::cout << "13. Conduct Espionage\n";
    std::cout << "14. Conduct Smuggling\n";
    std::cout << "15. Manage Healthcare\n";
    std::cout << "16. Manage Buildings\n";
    std::cout << "17. Save Game State\n";
    std::cout << "18. Load Game State\n";
    std::cout << "19. Save Score\n";
    std::cout << "20. Exit\n";
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    std::cout << GREEN << "Welcome to Stronghold!\n" << RESET;
    std::cout << "Player 1:\n";
    std::string kingdomName1 = getValidString("Enter your kingdom's name: ");
    std::string kingName1 = getValidString("Enter your king's name: ");
    std::cout << "Player 2:\n";
    std::string kingdomName2 = getValidString("Enter your kingdom's name (e.g., Ironhold): ");
    std::string kingName2 = getValidString("Enter your king's name: ");

    Kingdom player1(kingdomName1, kingName1);
    Kingdom player2(kingdomName2, kingName2);
    bool player1Turn = true;
    int turnCount = 1;

    while (true) {
        std::cout << BOLD << "\n=== Turn " << turnCount << " ===\n" << RESET;
        Kingdom& currentPlayer = player1Turn ? player1 : player2;
        Kingdom& otherPlayer = player1Turn ? player2 : player1;
        std::string playerLabel = player1Turn ? "Player 1" : "Player 2";

        std::cout << GREEN << playerLabel << "'s Turn (" << currentPlayer.getName() << ")\n" << RESET;
        std::cout << YELLOW << "Status of both kingdoms:\n" << RESET;
        player1.printStatus();
        player2.printStatus();

        displayMenu();
        int choice = getValidChoice(1, 20, "Enter your choice (1-20): ");

        try {
            switch (choice) {
            case 1: // Play Turn
                currentPlayer.playTurn();
                break;

            case 2: { // Train Army
                int count = getValidChoice(1, 100, "Enter number of soldiers to train (1-100): ");
                currentPlayer.trainArmy(count);
                break;
            }

            case 3: // Hold Election
                currentPlayer.holdElection();
                break;

            case 4: { // Manage Loan or Audit
                std::cout << "1. Take Loan\n2. Repay Loan\n3. Audit Corruption\n";
                int subChoice = getValidChoice(1, 3, "Choose action (1-3): ");
                if (subChoice == 1 || subChoice == 2) {
                    int amount = getValidChoice(1, 10000, "Enter amount (1-10000): ");
                    currentPlayer.manageLoanOrAudit(subChoice, amount);
                }
                else {
                    currentPlayer.manageLoanOrAudit(subChoice, 0);
                }
                break;
            }

            case 5: { // Buy Resource
                std::cout << "Resources: Food, Iron, Wood, Stone\n";
                std::string resource = getValidString("Enter resource to buy: ");
                int amount = getValidChoice(1, 1000, "Enter amount to buy (1-1000): ");
                currentPlayer.buyResource(resource, amount);
                break;
            }

            case 6: { // Manage Diplomacy
                std::string targetKingdom = getValidString("Enter target kingdom (e.g., " + otherPlayer.getName() + "): ");
                std::cout << "1. Form Alliance\n2. Break Alliance\n3. Form Trade Agreement\n4. Establish Secure Route\n";
                int subChoice = getValidChoice(1, 4, "Choose action (1-4): ");
                currentPlayer.manageDiplomacy(targetKingdom, subChoice);
                break;
            }

            case 7: { // Bribe or Blackmail
                std::cout << "1. Bribe\n2. Blackmail\n";
                int subChoice = getValidChoice(1, 2, "Choose action (1-2): ");
                std::string candidate = getValidString("Enter candidate name: ");
                currentPlayer.bribeOrBlackmail(subChoice, candidate);
                break;
            }

            case 8: { // Send Message
                std::string recipient = getValidString("Enter recipient kingdom (e.g., " + otherPlayer.getName() + "): ");
                std::string message = getValidString("Enter message: ");
                currentPlayer.sendMessage(recipient, message);
                break;
            }

            case 9: { // Send Fake Trade Request
                std::string recipient = getValidString("Enter recipient kingdom (e.g., " + otherPlayer.getName() + "): ");
                currentPlayer.sendFakeTradeRequest(recipient);
                break;
            }

            case 10: // View Messages
                currentPlayer.viewMessages();
                break;

            case 11: // Upgrade Blacksmith
                currentPlayer.upgradeBlacksmith();
                break;

            case 12: { // Produce Weapons
                int count = getValidChoice(1, 50, "Enter number of weapons to produce (1-50): ");
                currentPlayer.produceWeapons(count);
                break;
            }

            case 13: { // Conduct Espionage
                std::cout << "1. Spy Mission\n2. Sabotage Weapons\n3. Steal Gold\n";
                int subChoice = getValidChoice(1, 3, "Choose espionage action (1-3): ");
                currentPlayer.conductEspionage(subChoice, otherPlayer);
                break;
            }

            case 14: { // Conduct Smuggling
                currentPlayer.conductSmuggling(otherPlayer);
                break;
            }

            case 15: { // Manage Healthcare
                std::cout << "1. Build Hospital\n2. Provide Services\n";
                int subChoice = getValidChoice(1, 2, "Choose healthcare action (1-2): ");
                currentPlayer.manageHealthcare(subChoice);
                break;
            }

            case 16: { // Manage Buildings
                std::cout << "1. Build Barracks\n";
                int subChoice = getValidChoice(1, 1, "Choose building action (1): ");
                currentPlayer.manageBuildings(subChoice);
                break;
            }

            case 17: { // Save Game State
                std::string filename = getValidString("Enter save file name: ");
                currentPlayer.saveState(filename);
                break;
            }

            case 18: { // Load Game State
                std::string filename = getValidString("Enter save file name: ");
                currentPlayer.loadState(filename);
                break;
            }

            case 19: // Save Score
                currentPlayer.saveScore();
                break;

            case 20: // Exit
                std::cout << GREEN << "Thank you for playing Stronghold!\n" << RESET;
                return 0;

            default:
                std::cout << RED << "Invalid choice.\n" << RESET;
            }
        }
        catch (const InsufficientResourcesException& e) {
            std::cout << RED << "Error: " << e.what() << "\n" << RESET;
        }
        catch (const CorruptionException& e) {
            std::cout << RED << "Error: " << e.what() << "\n" << RESET;
        }
        catch (const std::exception& e) {
            std::cout << RED << "Unexpected error: " << e.what() << "\n" << RESET;
        }

        player1Turn = !player1Turn;
        if (player1Turn) turnCount++;
    }

    return 0;
}