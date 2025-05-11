#ifndef STRONGHOLD_H
#define STRONGHOLD_H

#include <string>
#include <memory>
#include <exception>

const int MAX_CLASSES = 4;
const int MAX_CANDIDATES = 3;
const int MAX_MESSAGES = 10;
const int MAX_ALLIANCES = 2;
const int MAX_PRICES = 4;
const int GRID_SIZE = 5;

// ANSI color codes
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"
#define BOLD "\033[1m"

// Forward declarations
class Kingdom;
class Map;

// Utility functions
int getValidInt(const std::string& prompt);
std::string getValidString(const std::string& prompt);

// Custom exceptions
class InsufficientResourcesException : public std::exception {
    std::string message;
public:
    InsufficientResourcesException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class CorruptionException : public std::exception {
    std::string message;
public:
    CorruptionException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

// Resource class
template <typename T>
class Resource {
    T value;
public:
    Resource(T initial = T()) : value(initial) {}
    void adjust(T delta) { value += delta; if (value < 0) value = 0; }
    T get() const { return value; }
};

// General class
class General {
    std::string name;
    double loyalty;
    bool corrupted;
public:
    General(const std::string& name, double loyalty);
    std::string getName() const;
    bool isCorrupted() const;
    void setCorrupted(bool val);
};

// King class
class King {
    std::string name;
    double approval;
    std::string style;
    bool corrupted;
public:
    King(const std::string& name, double approval, const std::string& style);
    std::string getName() const;
    bool isCorrupted() const;
    void setCorrupted(bool val);
};

// Population class
struct ResourcePair {
    std::string name = "";
    int size = 0;
    double satisfaction = 0.0;
};

class Population {
    double morale;
    ResourcePair classes[MAX_CLASSES];
public:
    Population();
    void adjustMorale(double delta);
    void adjustClassSize(const std::string& className, int delta);
    void handleClassConflict();
    int getTotalSize() const;
    double getMorale() const;
    ResourcePair* getClasses();
};

// Economy class
class Economy {
    Resource<int> gold;
    bool progressiveTax;
    int debtReliance;
public:
    Economy(int initialGold);
    void spend(int amount);
    void collectTaxes(Population& pop);
    void triggerMarketCrash(Population& pop);
    int getGold() const;
    bool isProgressiveTax() const;
    int getDebtReliance() const;
    void increaseDebtReliance(int amount);
};

// Blacksmith class
class Blacksmith {
    int level;
    Resource<int> weaponsInStock;
    bool corrupted;
public:
    Blacksmith();
    void upgrade(Economy& econ);
    void produceWeapons(Resource<int>& iron, Resource<int>& wood, int count);
    void useWeapons(int count);
    int getWeaponsInStock() const;
    int getLevel() const;
    bool isCorrupted() const;
    void setCorrupted(bool val);
};

// Army class
class Army {
    int soldiers;
    double morale;
    int weapons;
    int trainingDelay;
    std::unique_ptr<General> general;
public:
    Army(int size, int weap);
    void train(int count, Population& pop, Resource<int>& iron, Blacksmith& blacksmith, double efficiency);
    void useSpies(int count);
    void checkMorale(Economy& econ);
    void applyTrainingDelay();
    General& getGeneral();
    const General& getGeneral() const;
    int getSize() const;
    int getWeapons() const;
    double getMorale() const;
};

// Politics class
class Politics {
    std::string currentKing;
    int candidateCount;
    std::unique_ptr<King> candidates[MAX_CANDIDATES];
    bool corrupted;
public:
    Politics(const std::string& kingName);
    void holdElection(Population& pop, Economy& econ);
    void bribe(Economy& econ, const std::string& candidate);
    void blackmail(Economy& econ, const std::string& candidate);
    void triggerRebellion(Population& pop, Economy& econ);
    std::unique_ptr<King>* getCandidates();
    int getCandidateCount() const;
    std::string getCurrentKing() const;
    void setCorrupted(bool val);
};

// Corruption class
class Corruption {
    bool armyCorrupted;
    bool politicsCorrupted;
    bool blacksmithCorrupted;
public:
    Corruption();
    void checkCorruption();
    void audit(Economy& econ, Army& army, Politics& politics, Blacksmith& blacksmith);
};

// Bank class
class Bank {
    int loan;
    double interestRate;
    bool corrupted;
    int landSeized;
public:
    Bank();
    void takeLoan(Economy& econ, int amount);
    void repayLoan(Economy& econ, int amount);
    void checkCorruption();
    void audit(Economy& econ);
    void seizeLand(Economy& econ, Map& map);
    int getLoan() const;
    int getLandSeized() const;
    bool isCorrupted() const;
};

// Diplomacy class
struct Alliance {
    std::string kingdom = "";
    bool active = false;
    bool trade = false;
    bool secureRoute = false;
};

class Diplomacy {
    Alliance alliances[MAX_ALLIANCES];
    int allianceCount;
public:
    Diplomacy();
    void formAlliance(const std::string& kingdom);
    void breakAlliance(const std::string& kingdom);
    void formTradeAgreement(const std::string& kingdom);
    void establishSecureRoute(const std::string& kingdom);
    void handleEspionageFailure(const std::string& sourceKingdom);
    bool hasAlliance(const std::string& kingdom) const;
    bool hasSecureRoute(const std::string& kingdom) const;
    int getAllianceCount() const;
};

// Communication class
struct Message {
    std::string recipient = "";
    std::string content = "";
    bool isFake = false;
};

class Communication {
    Message messages[MAX_MESSAGES];
    int messageCount;
public:
    Communication();
    void sendMessage(const std::string& recipient, const std::string& message, bool isFake);
    void viewMessages(const std::string& kingdom);
    void sendFakeTradeRequest(const std::string& recipient);
};

// Healthcare class
class Healthcare {
    int level;
    bool isBuilding;
    double satisfactionBoost;
    double plagueReduction;
public:
    Healthcare();
    void build(Economy& econ, Resource<int>& wood, Resource<int>& stone);
    void provideServices(Population& pop);
    void manageHealthcare(int choice, Economy& econ, Resource<int>& wood, Resource<int>& stone, Population& pop);
    int getLevel() const;
    double getPlagueReduction() const;
};

// Buildings class
class Buildings {
    int barracksLevel;
    bool isBuilding;
    double trainingEfficiency;
public:
    Buildings();
    void buildBarracks(Economy& econ, Resource<int>& wood, Resource<int>& stone);
    void manageBuildings(int choice, Economy& econ, Resource<int>& wood, Resource<int>& stone);
    int getBarracksLevel() const;
    double getTrainingEfficiency() const;
};

// Weather class
class Weather {
    std::string season;
    std::string currentWeather;
    int turnCount;
public:
    Weather();
    void updateWeather();
    int getFoodImpact() const;
    int getDelayImpact() const;
    std::string getSeason() const;
    std::string getWeather() const;
};

// Inflation class
class Inflation {
    double rate;
public:
    Inflation();
    void update(Economy& econ, Bank& bank);
    double getRate() const;
};

// Map class
class Map {
    char grid[GRID_SIZE][GRID_SIZE];
public:
    Map();
    void display() const;
    void capture(const std::string& kingdom, int x, int y);
    void enemyAttack(Resource<int>& resource);
};

// Market class
struct Price {
    std::string resource = "";
    double value = 0.0;
};

class Market {
    Inflation* inflation;
    bool boycott;
    bool sanctions;
    bool smugglerActive;
    bool guildDemands;
    Price prices[MAX_PRICES];
public:
    Market(Inflation* inf);
    void updatePrices();
    double getPrice(const std::string& resource) const;
    void buyResource(Economy& econ, const std::string& resource, int amount, Resource<int>& res);
    void handleSmuggler(Economy& econ, Resource<int>& resource);
    void handleGuildDemands(Economy& econ, Population& pop);
    bool isSmugglerActive() const;
};

// Espionage class
class Espionage {
    std::string lastAction;
public:
    Espionage();
    void spyMission(Kingdom& source, Kingdom& target);
    void sabotageWeapons(Kingdom& source, Kingdom& target);
    void stealGold(Kingdom& source, Kingdom& target);
};

// Smuggling class
class Smuggling {
    std::string lastAction;
public:
    Smuggling();
    void smuggleGoods(Kingdom& source, Kingdom& target);
};

// Kingdom class
class Kingdom {
    std::string name;
    Resource<int> food, iron, wood, stone;
    std::unique_ptr<Population> population;
    std::unique_ptr<Economy> economy;
    std::unique_ptr<Army> army;
    std::unique_ptr<Bank> bank;
    std::unique_ptr<Politics> politics;
    std::unique_ptr<Blacksmith> blacksmith;
    std::unique_ptr<Diplomacy> diplomacy;
    std::unique_ptr<Communication> communication;
    std::unique_ptr<Healthcare> healthcare;
    std::unique_ptr<Buildings> buildings;
    std::unique_ptr<Weather> weather;
    std::unique_ptr<Inflation> inflation;
    std::unique_ptr<Corruption> corruption;
    std::unique_ptr<Map> map;
    std::unique_ptr<Market> market;

public:
    Kingdom(const std::string& kingdomName, const std::string& kingName);
    void playTurn();
    void randomEvent();
    void trainArmy(int count);
    void holdElection();
    void manageLoanOrAudit(int choice, int amount);
    void buyResource(const std::string& resource, int amount);
    void manageDiplomacy(const std::string& kingdom, int choice);
    void bribeOrBlackmail(int choice, const std::string& candidate);
    void sendMessage(const std::string& recipient, const std::string& message);
    void sendFakeTradeRequest(const std::string& recipient);
    void viewMessages();
    void upgradeBlacksmith();
    void produceWeapons(int count);
    void conductEspionage(int action, Kingdom& target);
    void conductSmuggling(Kingdom& target);
    void manageHealthcare(int choice);
    void manageBuildings(int choice);
    void saveState(const std::string& filename) const;
    void loadState(const std::string& filename);
    void saveScore() const;
    int calculateScore() const;
    void printStatus() const;

    Bank& getBank();
    const Bank& getBank() const;
    Resource<int>& getIron();
    const Resource<int>& getIron() const;
    Economy& getEconomy();
    const Economy& getEconomy() const;
    Population& getPopulation();
    const Population& getPopulation() const;
    Army& getArmy();
    const Army& getArmy() const;
    Blacksmith& getBlacksmith();
    const Blacksmith& getBlacksmith() const;
    Diplomacy& getDiplomacy();
    const Diplomacy& getDiplomacy() const;
    Weather& getWeather();
    const Weather& getWeather() const;
    Market& getMarket();
    const Market& getMarket() const;
    std::string getName() const;
};

// Validation class
class Validation {
public:
    static void validateKingdom(const Kingdom& kingdom);
};

#endif