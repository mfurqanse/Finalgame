#include "stronghold.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>

// Utility functions
int getValidInt(const std::string& prompt) {
    int input;
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        try {
            input = std::stoi(line);
            if (input < 0) throw InsufficientResourcesException("Negative input not allowed");
            break;
        }
        catch (...) {
            std::cout << RED << "Invalid input. Enter a valid number.\n" << RESET;
        }
    }
    return input;
}

std::string getValidString(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

// General class
General::General(const std::string& name, double loyalty) : name(name), loyalty(loyalty), corrupted(false) {}
std::string General::getName() const { return name; }
bool General::isCorrupted() const { return corrupted; }
void General::setCorrupted(bool val) { corrupted = val; }

// King class
King::King(const std::string& name, double approval, const std::string& style)
    : name(name), approval(approval), style(style), corrupted(false) {
}
std::string King::getName() const { return name; }
bool King::isCorrupted() const { return corrupted; }
void King::setCorrupted(bool val) { corrupted = val; }

// Population class
Population::Population() : morale(0.85) {
    classes[0] = { "Peasants", 700, 0.75 };
    classes[1] = { "Merchants", 200, 0.85 };
    classes[2] = { "Nobility", 50, 0.95 };
    classes[3] = { "Military", 50, 0.9 };
}

void Population::adjustMorale(double delta) {
    morale += delta;
    if (morale < 0.0) morale = 0.0;
    if (morale > 1.0) morale = 1.0;
}

void Population::adjustClassSize(const std::string& className, int delta) {
    for (int i = 0; i < MAX_CLASSES; ++i) {
        if (classes[i].name == className) {
            classes[i].size += delta;
            if (classes[i].size < 0) classes[i].size = 0;
            return;
        }
    }
    throw InsufficientResourcesException("Class not found");
}

void Population::handleClassConflict() {
    if (morale < 0.4 && rand() % 10 < 3) {
        int loss = classes[0].size / 10;
        adjustClassSize("Peasants", -loss);
        adjustClassSize("Merchants", -loss / 2);
        adjustMorale(-0.15);
        std::cout << RED << "Class conflict! Peasants and Merchants riot, population decreases.\n" << RESET;
    }
}

int Population::getTotalSize() const {
    int total = 0;
    for (int i = 0; i < MAX_CLASSES; ++i) {
        total += classes[i].size;
    }
    return total;
}

double Population::getMorale() const { return morale; }
ResourcePair* Population::getClasses() { return classes; }

// Economy class
Economy::Economy(int initialGold) : gold(initialGold), progressiveTax(false), debtReliance(0) {}

void Economy::spend(int amount) {
    if (gold.get() < amount) throw InsufficientResourcesException("Insufficient gold");
    gold.adjust(-amount);
}

void Economy::collectTaxes(Population& pop) {
    int tax = progressiveTax ? static_cast<int>(pop.getTotalSize() * 0.1) : 100;
    gold.adjust(tax);
    pop.adjustMorale(-0.05);
    std::cout << GREEN << "Collected " << tax << " gold in taxes.\n" << RESET;
}

void Economy::triggerMarketCrash(Population& pop) {
    if (rand() % 15 == 0) {
        gold.adjust(-gold.get() / 3);
        pop.adjustMorale(-0.2);
        std::cout << RED << "Market crash! Gold reserves drop by a third, morale plummets.\n" << RESET;
    }
}

int Economy::getGold() const { return gold.get(); }
bool Economy::isProgressiveTax() const { return progressiveTax; }
int Economy::getDebtReliance() const { return debtReliance; }
void Economy::increaseDebtReliance(int amount) { debtReliance += amount; }

// Blacksmith class
Blacksmith::Blacksmith() : level(1), weaponsInStock(0), corrupted(false) {}

void Blacksmith::upgrade(Economy& econ) {
    int cost = 500 * level;
    econ.spend(cost);
    level++;
    std::cout << GREEN << "Blacksmith upgraded to level " << level << "!\n" << RESET;
}

void Blacksmith::produceWeapons(Resource<int>& iron, Resource<int>& wood, int count) {
    if (iron.get() < count * 10 || wood.get() < count * 5)
        throw InsufficientResourcesException("Insufficient resources for weapon production");
    iron.adjust(-count * 10);
    wood.adjust(-count * 5);
    std::cout << "Producing " << count << " weapons...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3 / level));
    weaponsInStock.adjust(count);
    std::cout << GREEN << "Produced " << count << " weapons!\n" << RESET;
}

void Blacksmith::useWeapons(int count) {
    if (weaponsInStock.get() < count) throw InsufficientResourcesException("Not enough weapons");
    weaponsInStock.adjust(-count);
}

int Blacksmith::getWeaponsInStock() const { return weaponsInStock.get(); }
int Blacksmith::getLevel() const { return level; }
bool Blacksmith::isCorrupted() const { return corrupted; }
void Blacksmith::setCorrupted(bool val) { corrupted = val; }

// Army class
Army::Army(int size, int weap) : soldiers(size), morale(0.8), weapons(weap), trainingDelay(0) {
    general = std::make_unique<General>("General Patton", 0.85);
}

void Army::train(int count, Population& pop, Resource<int>& iron, Blacksmith& blacksmith, double efficiency) {
    if (pop.getTotalSize() < count || iron.get() < count * 10)
        throw InsufficientResourcesException("Insufficient population or iron");
    if (blacksmith.getWeaponsInStock() < count)
        throw InsufficientResourcesException("Insufficient weapons in stock");
    if (trainingDelay > 0) {
        std::cout << YELLOW << "Training delayed by " << trainingDelay << " turns.\n" << RESET;
        return;
    }
    pop.adjustClassSize("Peasants", -count);
    pop.adjustClassSize("Military", count);
    iron.adjust(-count * 10);
    blacksmith.useWeapons(count);
    std::cout << "Training " << count << " soldiers...\n";
    std::this_thread::sleep_for(std::chrono::seconds(static_cast<int>(5 * efficiency * (getGeneral().isCorrupted() ? 1.5 : 1.0))));
    soldiers += count;
    morale = (morale < 1.0) ? morale + 0.05 : 1.0;
    trainingDelay = getGeneral().isCorrupted() ? 2 : 1;
    std::cout << GREEN << "Trained " << count << " soldiers!\n" << RESET;
}

void Army::useSpies(int count) {
    if (soldiers < count) throw InsufficientResourcesException("Not enough soldiers");
    soldiers -= count;
}

void Army::checkMorale(Economy& econ) {
    if (econ.getGold() < soldiers * 2) {
        morale -= 0.1;
        std::cout << RED << "Unpaid soldiers! Army morale drops.\n" << RESET;
    }
    if (morale < 0.3) {
        soldiers = (soldiers > soldiers / 10) ? soldiers - soldiers / 10 : 0;
        std::cout << RED << "Soldiers desert due to low morale!\n" << RESET;
    }
}

void Army::applyTrainingDelay() {
    if (trainingDelay > 0) {
        trainingDelay--;
        std::cout << YELLOW << "Training delay: " << trainingDelay << " turns remaining.\n" << RESET;
    }
}

General& Army::getGeneral() { return *general; }
const General& Army::getGeneral() const { return *general; }
int Army::getSize() const { return soldiers; }
int Army::getWeapons() const { return weapons; }
double Army::getMorale() const { return morale; }

// Politics class
Politics::Politics(const std::string& kingName) : currentKing(kingName), candidateCount(3), corrupted(false) {
    candidates[0] = std::make_unique<King>("Arthur", 0.8, "Diplomatic");
    candidates[1] = std::make_unique<King>("Eleanor", 0.75, "Economic");
    candidates[2] = std::make_unique<King>("Richard", 0.7, "Aggressive");
}

void Politics::holdElection(Population& pop, Economy& econ) {
    if (rand() % 10 == 0) {
        std::cout << RED << "Assassination! Current king killed, re-election triggered!\n" << RESET;
        currentKing = getCandidates()[rand() % getCandidateCount()]->getName();
        pop.adjustMorale(-0.2);
        return;
    }
    if (corrupted) {
        currentKing = getCandidates()[rand() % getCandidateCount()]->getName();
        std::cout << RED << "Corrupt election! King chosen randomly.\n" << RESET;
        return;
    }
    double votes[MAX_CANDIDATES] = { 0 };
    double totalVotes = 0;
    ResourcePair* classes = pop.getClasses();
    for (int i = 0; i < MAX_CLASSES; ++i) {
        for (int j = 0; j < getCandidateCount(); ++j) {
            double voteWeight = classes[i].satisfaction * classes[i].size;
            votes[j] += voteWeight;
            totalVotes += voteWeight;
        }
    }
    double maxVotes = 0;
    int winnerIndex = 0;
    for (int j = 0; j < getCandidateCount(); ++j) {
        if (votes[j] > maxVotes) {
            maxVotes = votes[j];
            winnerIndex = j;
        }
    }
    currentKing = getCandidates()[winnerIndex]->getName();
    pop.adjustMorale(0.05);
    std::cout << GREEN << "Election held! New king: " << currentKing << "\n" << RESET;
}

void Politics::bribe(Economy& econ, const std::string& candidate) {
    econ.spend(200);
    std::cout << YELLOW << "Bribed voters to favor " << candidate << ".\n" << RESET;
}

void Politics::blackmail(Economy& econ, const std::string& candidate) {
    econ.spend(300);
    std::cout << YELLOW << "Blackmailed voters to favor " << candidate << ", morale drops.\n" << RESET;
    econ.increaseDebtReliance(50);
}

void Politics::triggerRebellion(Population& pop, Economy& econ) {
    if (pop.getMorale() < 0.3 && rand() % 5 == 0) {
        pop.adjustClassSize("Peasants", -pop.getTotalSize() / 4);
        pop.adjustMorale(-0.2);
        econ.spend(econ.getGold() / 4);
        std::cout << RED << "Rebellion! Peasants revolt, treasury loses gold!\n" << RESET;
    }
}

std::unique_ptr<King>* Politics::getCandidates() { return candidates; }
int Politics::getCandidateCount() const { return candidateCount; }
std::string Politics::getCurrentKing() const { return currentKing; }
void Politics::setCorrupted(bool val) { corrupted = val; }

// Corruption class
Corruption::Corruption() : armyCorrupted(false), politicsCorrupted(false), blacksmithCorrupted(false) {}

void Corruption::checkCorruption() {
    if (rand() % 10 == 0) armyCorrupted = true;
    if (rand() % 15 == 0) politicsCorrupted = true;
    if (rand() % 12 == 0) blacksmithCorrupted = true;
}

void Corruption::audit(Economy& econ, Army& army, Politics& politics, Blacksmith& blacksmith) {
    try {
        econ.spend(200);
        if (armyCorrupted) {
            armyCorrupted = false;
            army.getGeneral().setCorrupted(false);
            std::cout << GREEN << "Army corruption cleared.\n" << RESET;
        }
        if (politicsCorrupted) {
            politicsCorrupted = false;
            politics.setCorrupted(false);
            for (int i = 0; i < politics.getCandidateCount(); ++i) {
                politics.getCandidates()[i]->setCorrupted(false);
            }
            std::cout << GREEN << "Politics corruption cleared.\n" << RESET;
        }
        if (blacksmithCorrupted) {
            blacksmithCorrupted = false;
            blacksmith.setCorrupted(false);
            std::cout << GREEN << "Blacksmith corruption cleared.\n" << RESET;
        }
    }
    catch (const InsufficientResourcesException& e) {
        throw CorruptionException("Audit failed: " + std::string(e.what()));
    }
}

// Bank class
Bank::Bank() : loan(0), interestRate(0.1), corrupted(false), landSeized(0) {}

void Bank::takeLoan(Economy& econ, int amount) {
    econ.spend(-amount);
    loan += amount;
    econ.increaseDebtReliance(amount / 2);
    std::cout << GREEN << "Took loan of " << amount << " gold.\n" << RESET;
}

void Bank::repayLoan(Economy& econ, int amount) {
    if (loan < amount) throw InsufficientResourcesException("Cannot repay more than loan");
    econ.spend(amount);
    loan -= amount;
    std::cout << GREEN << "Repaid " << amount << " gold.\n" << RESET;
}

void Bank::checkCorruption() {
    if (rand() % 20 == 0) {
        corrupted = true;
        std::cout << RED << "Bank corruption detected!\n" << RESET;
    }
}

void Bank::audit(Economy& econ) {
    try {
        econ.spend(100);
        corrupted = false;
        std::cout << GREEN << "Bank audit cleared corruption. Detailed report generated.\n" << RESET;
    }
    catch (const InsufficientResourcesException& e) {
        throw CorruptionException("Bank audit failed: " + std::string(e.what()));
    }
}

void Bank::seizeLand(Economy& econ, Map& map) {
    if (loan > 2000 && rand() % 5 == 0) {
        landSeized++;
        map.capture("Bank", rand() % GRID_SIZE, rand() % GRID_SIZE);
        econ.spend(econ.getGold() / 5);
        std::cout << RED << "Bank seized land due to unpaid loans!\n" << RESET;
    }
}

int Bank::getLoan() const { return loan; }
int Bank::getLandSeized() const { return landSeized; }
bool Bank::isCorrupted() const { return corrupted; }

// Diplomacy class
Diplomacy::Diplomacy() : allianceCount(0) {
    alliances[0] = { "", false, false, false };
    alliances[1] = { "", false, false, false };
}

void Diplomacy::formAlliance(const std::string& kingdom) {
    if (allianceCount < 2) {
        alliances[allianceCount++] = { kingdom, true, false, false };
        std::cout << GREEN << "Alliance formed with " << kingdom << "!\n" << RESET;
    }
    else {
        std::cout << RED << "Cannot form more alliances.\n" << RESET;
    }
}

void Diplomacy::breakAlliance(const std::string& kingdom) {
    for (int i = 0; i < allianceCount; ++i) {
        if (alliances[i].kingdom == kingdom) {
            alliances[i].active = false;
            alliances[i].trade = false;
            alliances[i].secureRoute = false;
            std::cout << YELLOW << "Alliance broken with " << kingdom << ".\n" << RESET;
            return;
        }
    }
    std::cout << RED << "No alliance with " << kingdom << ".\n" << RESET;
}

void Diplomacy::formTradeAgreement(const std::string& kingdom) {
    for (int i = 0; i < allianceCount; ++i) {
        if (alliances[i].kingdom == kingdom && alliances[i].active) {
            alliances[i].trade = true;
            std::cout << GREEN << "Trade agreement formed with " << kingdom << "!\n" << RESET;
            return;
        }
    }
    throw InsufficientResourcesException("No alliance exists");
}

void Diplomacy::establishSecureRoute(const std::string& kingdom) {
    for (int i = 0; i < allianceCount; ++i) {
        if (alliances[i].kingdom == kingdom && alliances[i].active) {
            alliances[i].secureRoute = true;
            std::cout << GREEN << "Secure trade route established with " << kingdom << "!\n" << RESET;
            return;
        }
    }
    throw InsufficientResourcesException("No alliance exists");
}

void Diplomacy::handleEspionageFailure(const std::string& sourceKingdom) {
    for (int i = 0; i < allianceCount; ++i) {
        if (alliances[i].kingdom == sourceKingdom) {
            alliances[i].active = false;
            alliances[i].trade = false;
            alliances[i].secureRoute = false;
            std::cout << RED << "Espionage detected! All alliances and trade agreements with "
                << sourceKingdom << " are broken!\n" << RESET;
            return;
        }
    }
}

bool Diplomacy::hasAlliance(const std::string& kingdom) const {
    for (int i = 0; i < allianceCount; ++i) {
        if (alliances[i].kingdom == kingdom && alliances[i].active) return true;
    }
    return false;
}

bool Diplomacy::hasSecureRoute(const std::string& kingdom) const {
    for (int i = 0; i < allianceCount; ++i) {
        if (alliances[i].kingdom == kingdom && alliances[i].secureRoute) return true;
    }
    return false;
}

int Diplomacy::getAllianceCount() const {
    int count = 0;
    for (int i = 0; i < allianceCount; ++i) {
        if (alliances[i].active) count++;
    }
    return count;
}

// Communication class
Communication::Communication() : messageCount(0) {}

void Communication::sendMessage(const std::string& recipient, const std::string& message, bool isFake) {
    if (messageCount < MAX_MESSAGES) {
        messages[messageCount++] = { recipient, message, isFake };
        std::cout << GREEN << "Message sent to " << recipient << ": " << message
            << (isFake ? " (fake)" : "") << "\n" << RESET;
    }
    else {
        std::cout << RED << "Message limit reached.\n" << RESET;
    }
}

void Communication::viewMessages(const std::string& kingdom) {
    std::cout << YELLOW << "Messages for " << kingdom << ":\n" << RESET;
    for (int i = 0; i < messageCount; ++i) {
        if (messages[i].recipient == kingdom) {
            std::cout << messages[i].content << (messages[i].isFake ? " (FAKE)" : "") << "\n";
        }
    }
}

void Communication::sendFakeTradeRequest(const std::string& recipient) {
    sendMessage(recipient, "Trade Request: 100 Iron for 200 Gold", true);
}

// Healthcare class
Healthcare::Healthcare() : level(1), isBuilding(false), satisfactionBoost(0.05), plagueReduction(0.1) {}

void Healthcare::build(Economy& econ, Resource<int>& wood, Resource<int>& stone) {
    if (isBuilding) throw InsufficientResourcesException("Hospital already under construction");
    if (econ.getGold() < 500 || wood.get() < 100 || stone.get() < 100)
        throw InsufficientResourcesException("Insufficient resources");
    econ.spend(500);
    wood.adjust(-100);
    stone.adjust(-100);
    isBuilding = true;
    std::cout << "Building hospital...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    level++;
    satisfactionBoost += 0.02;
    plagueReduction += 0.05;
    isBuilding = false;
    std::cout << GREEN << "Hospital built! Level: " << level << RESET << "\n";
}

void Healthcare::provideServices(Population& pop) {
    pop.adjustMorale(satisfactionBoost);
    std::cout << GREEN << "Healthcare services boosted morale by " << satisfactionBoost << RESET << "\n";
}

void Healthcare::manageHealthcare(int choice, Economy& econ, Resource<int>& wood, Resource<int>& stone, Population& pop) {
    if (choice == 1) {
        build(econ, wood, stone);
    }
    else if (choice == 2) {
        provideServices(pop);
    }
    else {
        throw InsufficientResourcesException("Invalid healthcare choice");
    }
}

int Healthcare::getLevel() const { return level; }
double Healthcare::getPlagueReduction() const { return plagueReduction; }

// Buildings class
Buildings::Buildings() : barracksLevel(0), isBuilding(false), trainingEfficiency(1.0) {}

void Buildings::buildBarracks(Economy& econ, Resource<int>& wood, Resource<int>& stone) {
    if (isBuilding) throw InsufficientResourcesException("Barracks already under construction");
    if (econ.getGold() < 400 || wood.get() < 150 || stone.get() < 150)
        throw InsufficientResourcesException("Insufficient resources");
    econ.spend(400);
    wood.adjust(-150);
    stone.adjust(-150);
    isBuilding = true;
    std::cout << "Building barracks...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    barracksLevel++;
    trainingEfficiency *= 0.9;
    isBuilding = false;
    std::cout << GREEN << "Barracks built! Level: " << barracksLevel << RESET << "\n";
}

void Buildings::manageBuildings(int choice, Economy& econ, Resource<int>& wood, Resource<int>& stone) {
    if (choice == 1) {
        buildBarracks(econ, wood, stone);
    }
    else {
        throw InsufficientResourcesException("Invalid buildings choice");
    }
}

int Buildings::getBarracksLevel() const { return barracksLevel; }
double Buildings::getTrainingEfficiency() const { return trainingEfficiency; }

// Weather class
Weather::Weather() : season("Spring"), currentWeather("Clear"), turnCount(0) {}

void Weather::updateWeather() {
    turnCount++;
    if (turnCount % 4 == 0) season = "Spring";
    else if (turnCount % 4 == 1) season = "Summer";
    else if (turnCount % 4 == 2) season = "Autumn";
    else season = "Winter";
    int randWeather = rand() % 10;
    if (randWeather < 3) currentWeather = "Clear";
    else if (randWeather < 6) currentWeather = "Rain";
    else if (randWeather < 8) currentWeather = "Snow";
    else currentWeather = "Flood";
    std::cout << YELLOW << "Season: " << season << ", Weather: " << currentWeather << RESET << "\n";
}

int Weather::getFoodImpact() const {
    if (currentWeather == "Flood") return -200;
    if (currentWeather == "Rain" && season == "Spring") return 150;
    return 0;
}

int Weather::getDelayImpact() const {
    return (currentWeather == "Snow") ? 1 : 0;
}

std::string Weather::getSeason() const { return season; }
std::string Weather::getWeather() const { return currentWeather; }

// Inflation class
Inflation::Inflation() : rate(1.0) {}

void Inflation::update(Economy& econ, Bank& bank) {
    if (econ.isProgressiveTax() || bank.getLoan() > 1000) rate += 0.05;
    if (econ.getDebtReliance() > 1000) rate += 0.1;
    if (rate > 2.0) {
        std::cout << RED << "Bankruptcy! Gold devalued, morale drops.\n" << RESET;
        econ.spend(static_cast<int>(econ.getGold() * 0.5));
        rate = 1.5;
    }
}

double Inflation::getRate() const { return rate; }

// Map class
Map::Map() {
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            grid[i][j] = '.';
        }
    }
    grid[0][0] = 'S';  // Stronghold
    grid[4][4] = 'I';  // Ironhold
}

void Map::display() const {
    std::cout << YELLOW << "Map:\n";
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            std::cout << grid[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << RESET;
}

void Map::capture(const std::string& kingdom, int x, int y) {
    if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
        grid[x][y] = kingdom[0];
    }
}

void Map::enemyAttack(Resource<int>& resource) {
    if (rand() % 10 < 3) {
        int loss = resource.get() / 5;
        resource.adjust(-loss);
        std::cout << RED << "Enemy attack! Lost " << loss << " resources.\n" << RESET;
    }
}

// Market class
Market::Market(Inflation* inf) : inflation(inf), boycott(false), sanctions(false), smugglerActive(false), guildDemands(false) {
    prices[0] = { "Food", 2.0 };
    prices[1] = { "Iron", 5.0 };
    prices[2] = { "Wood", 3.0 };
    prices[3] = { "Stone", 4.0 };
}

void Market::updatePrices() {
    for (int i = 0; i < MAX_PRICES; ++i) {
        prices[i].value *= (0.9 + static_cast<double>(rand() % 21) / 100.0);
    }
    boycott = (rand() % 10 == 0);
    sanctions = (rand() % 15 == 0);
    smugglerActive = (rand() % 20 == 0);
    guildDemands = (rand() % 15 == 0);
    std::cout << YELLOW << "Market prices updated. Boycott: " << (boycott ? "Yes" : "No")
        << ", Sanctions: " << (sanctions ? "Yes" : "No")
        << ", Smugglers: " << (smugglerActive ? "Active" : "Inactive")
        << ", Guild Demands: " << (guildDemands ? "Active" : "Inactive") << "\n" << RESET;
}

double Market::getPrice(const std::string& resource) const {
    for (int i = 0; i < MAX_PRICES; ++i) {
        if (prices[i].resource == resource) {
            double price = prices[i].value * inflation->getRate();
            if (boycott) price *= 1.5;
            if (sanctions) price *= 1.3;
            if (smugglerActive) price *= 0.8;
            return price;
        }
    }
    throw InsufficientResourcesException("Resource not found");
}

void Market::buyResource(Economy& econ, const std::string& resource, int amount, Resource<int>& res) {
    double cost = getPrice(resource) * amount;
    econ.spend(static_cast<int>(cost));
    res.adjust(amount);
    std::cout << GREEN << "Bought " << amount << " " << resource << " for " << cost << " gold.\n" << RESET;
}

void Market::handleSmuggler(Economy& econ, Resource<int>& resource) {
    if (smugglerActive) {
        resource.adjust(100);
        econ.spend(50);
        std::cout << GREEN << "Smugglers delivered 100 illegal goods for 50 gold.\n" << RESET;
    }
}

void Market::handleGuildDemands(Economy& econ, Population& pop) {
    if (guildDemands) {
        econ.spend(200);
        pop.adjustMorale(-0.05);
        std::cout << RED << "Trader guild demands met, cost 200 gold, morale drops.\n" << RESET;
    }
}

bool Market::isSmugglerActive() const { return smugglerActive; }

// Espionage class
Espionage::Espionage() : lastAction("None") {}

void Espionage::spyMission(Kingdom& source, Kingdom& target) {
    Economy& econ = source.getEconomy();
    Army& army = source.getArmy();
    if (econ.getGold() < 100 || army.getSize() < 5)
        throw InsufficientResourcesException("Insufficient resources for spying");
    econ.spend(100);
    army.useSpies(5);
    std::cout << "Sending spies to " << target.getName() << "...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3 + source.getWeather().getDelayImpact()));
    double successChance = 0.7 * (target.getPopulation().getMorale() < 0.5 ? 1.2 : 1.0);
    if (static_cast<double>(rand()) / RAND_MAX < successChance) {
        std::cout << GREEN << "Spy mission successful! Target status:\n" << RESET;
        target.printStatus();
        lastAction = "Spy Mission";
    }
    else {
        std::cout << RED << "Spy mission failed! Spies detected.\n" << RESET;
        target.getDiplomacy().handleEspionageFailure(source.getName());
        lastAction = "Failed Spy Mission";
    }
}

void Espionage::sabotageWeapons(Kingdom& source, Kingdom& target) {
    Economy& econ = source.getEconomy();
    Army& army = source.getArmy();
    if (econ.getGold() < 150 || army.getSize() < 10)
        throw InsufficientResourcesException("Insufficient resources for sabotage");
    econ.spend(150);
    army.useSpies(10);
    std::cout << "Attempting to sabotage " << target.getName() << "'s weapons...\n";
    std::this_thread::sleep_for(std::chrono::seconds(4 + source.getWeather().getDelayImpact()));
    double successChance = 0.6 * (target.getBlacksmith().isCorrupted() ? 1.3 : 1.0);
    if (static_cast<double>(rand()) / RAND_MAX < successChance) {
        int weaponsLost = target.getBlacksmith().getWeaponsInStock() / 2;
        target.getBlacksmith().useWeapons(weaponsLost);
        std::cout << GREEN << "Sabotage successful! Destroyed " << weaponsLost << " weapons.\n" << RESET;
        lastAction = "Sabotage Weapons";
    }
    else {
        std::cout << RED << "Sabotage failed! Spies detected.\n" << RESET;
        target.getDiplomacy().handleEspionageFailure(source.getName());
        lastAction = "Failed Sabotage";
    }
}

void Espionage::stealGold(Kingdom& source, Kingdom& target) {
    Economy& econ = source.getEconomy();
    Army& army = source.getArmy();
    if (econ.getGold() < 200 || army.getSize() < 15)
        throw InsufficientResourcesException("Insufficient resources for theft");
    econ.spend(200);
    army.useSpies(15);
    std::cout << "Attempting to steal gold from " << target.getName() << "...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5 + source.getWeather().getDelayImpact()));
    double successChance = 0.5 * (target.getBank().isCorrupted() ? 1.4 : 1.0);
    if (static_cast<double>(rand()) / RAND_MAX < successChance) {
        int goldStolen = target.getEconomy().getGold() / 4;
        target.getEconomy().spend(goldStolen);
        source.getEconomy().spend(-goldStolen);
        std::cout << GREEN << "Theft successful! Stole " << goldStolen << " gold.\n" << RESET;
        lastAction = "Steal Gold";
    }
    else {
        std::cout << RED << "Theft failed! Spies detected.\n" << RESET;
        target.getDiplomacy().handleEspionageFailure(source.getName());
        lastAction = "Failed Theft";
    }
}

// Smuggling class
Smuggling::Smuggling() : lastAction("None") {}

void Smuggling::smuggleGoods(Kingdom& source, Kingdom& target) {
    Economy& econ = source.getEconomy();
    if (econ.getGold() < 100)
        throw InsufficientResourcesException("Insufficient gold for smuggling");
    if (!source.getDiplomacy().hasSecureRoute(target.getName()))
        throw InsufficientResourcesException("No secure route for smuggling");
    econ.spend(100);
    std::cout << "Smuggling goods to " << target.getName() << "...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3 + source.getWeather().getDelayImpact()));
    double successChance = 0.8 * (target.getMarket().isSmugglerActive() ? 1.2 : 1.0);
    if (static_cast<double>(rand()) / RAND_MAX < successChance) {
        int goods = 200;
        source.getIron().adjust(goods);
        target.getIron().adjust(-goods / 2);
        std::cout << GREEN << "Smuggling successful! Gained " << goods << " iron.\n" << RESET;
        lastAction = "Smuggle Goods";
    }
    else {
        std::cout << RED << "Smuggling failed! Goods seized.\n" << RESET;
        econ.spend(50);
        lastAction = "Failed Smuggling";
    }
}

// Kingdom class
Kingdom::Kingdom(const std::string& kingdomName, const std::string& kingName)
    : name(kingdomName), food(1000), iron(500), wood(800), stone(600) {
    population = std::make_unique<Population>();
    economy = std::make_unique<Economy>(1000);
    army = std::make_unique<Army>(100, 100);
    bank = std::make_unique<Bank>();
    politics = std::make_unique<Politics>(kingName);
    blacksmith = std::make_unique<Blacksmith>();
    diplomacy = std::make_unique<Diplomacy>();
    communication = std::make_unique<Communication>();
    healthcare = std::make_unique<Healthcare>();
    buildings = std::make_unique<Buildings>();
    weather = std::make_unique<Weather>();
    inflation = std::make_unique<Inflation>();
    corruption = std::make_unique<Corruption>();
    map = std::make_unique<Map>();
    market = std::make_unique<Market>(inflation.get());
}

void Kingdom::playTurn() {
    std::cout << BOLD << "=== Turn in " << name << " ===\n" << RESET;
    weather->updateWeather();
    food.adjust(weather->getFoodImpact());
    if (weather->getFoodImpact() < 0)
        std::cout << RED << "Weather reduced food by " << -weather->getFoodImpact() << "!\n" << RESET;
    else if (weather->getFoodImpact() > 0)
        std::cout << GREEN << "Weather increased food by " << weather->getFoodImpact() << "!\n" << RESET;
    economy->collectTaxes(*population);
    economy->triggerMarketCrash(*population);
    bank->checkCorruption();
    bank->seizeLand(*economy, *map);
    army->checkMorale(*economy);
    army->applyTrainingDelay();
    corruption->checkCorruption();
    inflation->update(*economy, *bank);
    population->handleClassConflict();
    politics->triggerRebellion(*population, *economy);
    map->enemyAttack(food);
    market->handleSmuggler(*economy, iron);
    market->handleGuildDemands(*economy, *population);
    randomEvent();
    Validation::validateKingdom(*this);
    printStatus();
}

void Kingdom::randomEvent() {
    int event = rand() % 10;
    switch (event) {
    case 0:
        population->adjustClassSize("Peasants", -static_cast<int>(population->getTotalSize() / 5 * (1 - healthcare->getPlagueReduction())));
        population->adjustMorale(-0.15);
        std::cout << RED << "Plague spreads! Population decreases significantly.\n" << RESET;
        break;
    case 1:
        economy->spend(economy->getGold() / 10);
        std::cout << RED << "Bandits raid the treasury!\n" << RESET;
        break;
    case 2:
        food.adjust(500);
        population->adjustMorale(0.1);
        std::cout << GREEN << "Bumper harvest! Food increases.\n" << RESET;
        break;
    case 3:
        food.adjust(-300);
        std::cout << RED << "Drought! Food supply decreases.\n" << RESET;
        break;
    case 4:
        market->updatePrices();
        std::cout << RED << "Sanctions imposed! Market prices increase.\n" << RESET;
        break;
    case 5:
        population->adjustMorale(-0.1);
        politics->getCandidates()[rand() % politics->getCandidateCount()]->setCorrupted(true);
        std::cout << RED << "Assassination attempt on king! Candidate corrupted.\n" << RESET;
        break;
    case 6:
        economy->triggerMarketCrash(*population);
        std::cout << RED << "Market crash! Prices soar.\n" << RESET;
        break;
    case 7:
        population->adjustMorale(-0.05);
        std::cout << RED << "Revolt risk rises!\n" << RESET;
        break;
    case 8:
        population->adjustClassSize("Nobility", -population->getClasses()[2].size / 2);
        population->adjustMorale(-0.1);
        std::cout << RED << "Noble uprising! Nobility population halved.\n" << RESET;
        break;
    case 9:
        army->getGeneral().setCorrupted(true);
        std::cout << RED << "Spy infiltration! General corrupted.\n" << RESET;
        break;
    }
}

void Kingdom::trainArmy(int count) {
    army->train(count, *population, iron, *blacksmith, buildings->getTrainingEfficiency());
}

void Kingdom::holdElection() {
    politics->holdElection(*population, *economy);
}

void Kingdom::manageLoanOrAudit(int choice, int amount) {
    if (choice == 1) {
        bank->takeLoan(*economy, amount);
    }
    else if (choice == 2) {
        bank->repayLoan(*economy, amount);
    }
    else if (choice == 3) {
        corruption->audit(*economy, *army, *politics, *blacksmith);
    }
}

void Kingdom::buyResource(const std::string& resource, int amount) {
    if (resource == "Food") market->buyResource(*economy, resource, amount, food);
    else if (resource == "Iron") market->buyResource(*economy, resource, amount, iron);
    else if (resource == "Wood") market->buyResource(*economy, resource, amount, wood);
    else if (resource == "Stone") market->buyResource(*economy, resource, amount, stone);
    else throw InsufficientResourcesException("Invalid resource");
}

void Kingdom::manageDiplomacy(const std::string& kingdom, int choice) {
    if (choice == 1) diplomacy->formAlliance(kingdom);
    else if (choice == 2) diplomacy->breakAlliance(kingdom);
    else if (choice == 3) diplomacy->formTradeAgreement(kingdom);
    else if (choice == 4) diplomacy->establishSecureRoute(kingdom);
}

void Kingdom::bribeOrBlackmail(int choice, const std::string& candidate) {
    if (choice == 1) politics->bribe(*economy, candidate);
    else if (choice == 2) politics->blackmail(*economy, candidate);
}

void Kingdom::sendMessage(const std::string& recipient, const std::string& message) {
    communication->sendMessage(recipient, message, false);
}

void Kingdom::sendFakeTradeRequest(const std::string& recipient) {
    communication->sendFakeTradeRequest(recipient);
}

void Kingdom::viewMessages() {
    communication->viewMessages(name);
}

void Kingdom::upgradeBlacksmith() {
    blacksmith->upgrade(*economy);
}

void Kingdom::produceWeapons(int count) {
    blacksmith->produceWeapons(iron, wood, count);
}

void Kingdom::conductEspionage(int action, Kingdom& target) {
    Espionage espionage;
    if (action == 1) espionage.spyMission(*this, target);
    else if (action == 2) espionage.sabotageWeapons(*this, target);
    else if (action == 3) espionage.stealGold(*this, target);
    else throw InsufficientResourcesException("Invalid espionage action");
}

void Kingdom::conductSmuggling(Kingdom& target) {
    Smuggling smuggling;
    smuggling.smuggleGoods(*this, target);
}

void Kingdom::manageHealthcare(int choice) {
    healthcare->manageHealthcare(choice, *economy, wood, stone, *population);
}

void Kingdom::manageBuildings(int choice) {
    buildings->manageBuildings(choice, *economy, wood, stone);
}

void Kingdom::saveState(const std::string& filename) const {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) throw std::runtime_error("Cannot open save file");
    file << "Kingdom: " << name << "\n";
    file << "Population: " << population->getTotalSize() << "\n";
    file << "Morale: " << population->getMorale() << "\n";
    file << "Gold: " << economy->getGold() << "\n";
    file << "Loan: " << bank->getLoan() << "\n";
    file << "LandSeized: " << bank->getLandSeized() << "\n";
    file << "Army: " << army->getSize() << "\n";
    file << "Weapons: " << army->getWeapons() << "\n";
    file << "Food: " << food.get() << "\n";
    file << "Iron: " << iron.get() << "\n";
    file << "Wood: " << wood.get() << "\n";
    file << "Stone: " << stone.get() << "\n";
    file << "BlacksmithLevel: " << blacksmith->getLevel() << "\n";
    file << "King: " << politics->getCurrentKing() << "\n";
    file << "Tax: " << (economy->isProgressiveTax() ? "Progressive" : "Flat") << "\n";
    file << "HealthcareLevel: " << healthcare->getLevel() << "\n";
    file << "BarracksLevel: " << buildings->getBarracksLevel() << "\n";
    file << "Inflation: " << inflation->getRate() << "\n";
    file << "\n";
    file.close();
    std::cout << GREEN << "Game state saved for " << name << "!\n" << RESET;
}

void Kingdom::loadState(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << YELLOW << "No save file found for " << name << ". Starting new game.\n" << RESET;
        return;
    }
    std::string line, savedName;
    while (std::getline(file, line)) {
        if (line.find("Kingdom: ") == 0) {
            savedName = line.substr(9);
            if (savedName != name) continue;
            std::cout << GREEN << "Game state loaded for " << name << "!\n" << RESET;
            break;
        }
    }
    file.close();
}

void Kingdom::saveScore() const {
    std::ofstream file("score.txt", std::ios::app);
    if (!file.is_open()) throw std::runtime_error("Cannot open score.txt");
    time_t now = time(nullptr);
#ifdef _MSC_VER
    char buffer[26];
    if (ctime_s(buffer, sizeof(buffer), &now) != 0) {
        throw std::runtime_error("Failed to format timestamp");
    }
#else
    char* buffer = ctime(&now);
    if (!buffer) {
        throw std::runtime_error("Failed to format timestamp");
    }
#endif
    file << buffer << "Kingdom: " << name << ", Score: " << calculateScore()
        << ", Gold: " << economy->getGold() << ", Army: " << army->getSize()
        << ", Morale: " << population->getMorale() << "\n";
    file.close();
    std::cout << GREEN << "Score saved to score.txt for " << name << "!\n" << RESET;
}

int Kingdom::calculateScore() const {
    int moraleScore = static_cast<int>(population->getMorale() * 300);
    int goldScore = std::min(1000, economy->getGold() / 10) * 250;
    int armyScore = army->getSize() * 2;
    int resourceScore = (food.get() + iron.get() + wood.get() + stone.get()) / 10;
    int diplomacyScore = diplomacy->getAllianceCount() * 50;
    int landPenalty = bank->getLandSeized() * 100;
    return moraleScore + goldScore + armyScore + resourceScore + diplomacyScore - landPenalty;
}

void Kingdom::printStatus() const {
    std::cout << YELLOW << "Kingdom Status (" << name << "):\n" << RESET;
    std::cout << "Population: " << population->getTotalSize() << ", Morale: " << population->getMorale() << "\n";
    ResourcePair* classes = population->getClasses();
    for (int i = 0; i < MAX_CLASSES; ++i) {
        std::cout << "  " << classes[i].name << ": " << classes[i].size << ", Satisfaction: " << classes[i].satisfaction << "\n";
    }
    std::cout << "Gold: " << economy->getGold() << ", Loan: " << bank->getLoan() << ", Debt Reliance: " << economy->getDebtReliance() << "\n";
    std::cout << "Army: " << army->getSize() << ", Morale: " << army->getMorale() << ", Weapons: " << army->getWeapons() << "\n";
    std::cout << "Resources: Food=" << food.get() << ", Iron=" << iron.get()
        << ", Wood=" << wood.get() << ", Stone=" << stone.get() << "\n";
    std::cout << "Blacksmith: Level=" << blacksmith->getLevel() << ", Weapons in stock=" << blacksmith->getWeaponsInStock() << "\n";
    std::cout << "Healthcare: Level=" << healthcare->getLevel() << ", Plague Reduction=" << healthcare->getPlagueReduction() * 100 << "%\n";
    std::cout << "Barracks: Level=" << buildings->getBarracksLevel() << ", Training Efficiency="
        << buildings->getTrainingEfficiency() * 100 << "%\n";
    std::cout << "Weather: " << weather->getSeason() << ", " << weather->getWeather() << "\n";
    std::cout << "Inflation: " << inflation->getRate() << "\n";
    std::cout << "King: " << politics->getCurrentKing() << "\n";
    std::cout << "Tax: " << (economy->isProgressiveTax() ? "Progressive" : "Flat") << "\n";
    std::cout << "Land Seized by Bank: " << bank->getLandSeized() << "\n";
    std::cout << "Score: " << calculateScore() << " points\n";
    map->display();
}

Bank& Kingdom::getBank() { return *bank; }
const Bank& Kingdom::getBank() const { return *bank; }
Resource<int>& Kingdom::getIron() { return iron; }
const Resource<int>& Kingdom::getIron() const { return iron; }
Economy& Kingdom::getEconomy() { return *economy; }
const Economy& Kingdom::getEconomy() const { return *economy; }
Population& Kingdom::getPopulation() { return *population; }
const Population& Kingdom::getPopulation() const { return *population; }
Army& Kingdom::getArmy() { return *army; }
const Army& Kingdom::getArmy() const { return *army; }
Blacksmith& Kingdom::getBlacksmith() { return *blacksmith; }
const Blacksmith& Kingdom::getBlacksmith() const { return *blacksmith; }
Diplomacy& Kingdom::getDiplomacy() { return *diplomacy; }
const Diplomacy& Kingdom::getDiplomacy() const { return *diplomacy; }
Weather& Kingdom::getWeather() { return *weather; }
const Weather& Kingdom::getWeather() const { return *weather; }
Market& Kingdom::getMarket() { return *market; }
const Market& Kingdom::getMarket() const { return *market; }
std::string Kingdom::getName() const { return name; }

// Validation class
void Validation::validateKingdom(const Kingdom& kingdom) {
    if (kingdom.getEconomy().getGold() < 0)
        std::cout << RED << "Warning: Negative gold detected!\n" << RESET;
    if (kingdom.getPopulation().getMorale() < 0 || kingdom.getPopulation().getMorale() > 1)
        std::cout << RED << "Warning: Invalid morale!\n" << RESET;
    if (kingdom.getArmy().getSize() < 0)
        std::cout << RED << "Warning: Negative army size!\n" << RESET;
    if (kingdom.getBank().getLoan() > 5000)
        std::cout << RED << "Warning: Excessive loan detected!\n" << RESET;
}