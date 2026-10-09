
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <algorithm>
#include <cctype>
#include <limits>

using namespace std;

// ANSI COLORS
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define GOLD    "\033[93m"
#define BLUE    "\033[34m"
#define WHITE   "\033[37m"

const int MAP_SIZE = 10;
const int MAX_ENEMIES = 12;
const int MAX_ITEMS = 20;
const int MAX_FLOORS = 3;
const int MAX_HP = 100;
const int MAX_ARMOR = 100;

enum CellType {
    EMPTY, WALL, ENEMY, TREASURE, TRAP, EXIT, BOSS, CHEST
};

struct Item {
    string name;
    int effect;
};

struct Player {
    string name;
    string classType;
    string element;

    int hp = 100;
    int armor = 0;
    int attackPower = 10;
    int gold = 0;
    int x = 0;
    int y = 0;

    int healthPotions = 0;
    int strengthPotions = 0;

    Item inventory[MAX_ITEMS];
    int inventoryCount = 0;
};

struct Enemy {
    string type;
    int hp;
    int maxHP;
    int attackPower;
    int x;
    int y;
    int armorDrop;
    int goldDrop;
    bool alive;
};

struct DungeonCell {
    CellType type;
    bool explored;
};

DungeonCell dungeon[MAP_SIZE][MAP_SIZE];

Enemy enemies[MAX_ENEMIES];
int enemyCount = 0;

Enemy boss;
Player player;

int currentFloor = 1;
bool bossDefeated = false;

// FUNCTION PROTOTYPES
void showIntro();
void chooseClass();
void initializeMap();
void displayMap();
void displayHUD();
void displayBars();
void movePlayer(char move);
void handleCellEvent(int x, int y);
void battle(Enemy& enemy, bool isBoss = false);
void openChest();
void showInventory();
void equipWeapon();
void useSpecialAbility();
void usePotion();
void saveHighScore();
void nextFloor();
void generateEnemies();
void placeOnEmptyCell(CellType type, int& x, int& y);
void addItem(string name, int effect);
void enemyLoot(Enemy& enemy);
void clearScreen();
void pauseGame();
void printLegend();

// CONSOLE HELPERS
void clearScreen() {
    cout << "\033[2J\033[H";
}

void pauseGame() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void printLegend() {
    cout << "\n" << GREEN << "P" << RESET << " Player  "
         << RED << "G" << RESET << " Goblin  "
         << RED << "O" << RESET << " Orc  "
         << RED << "M" << RESET << " Mimic  "
         << RED << "D" << RESET << " Dragon\n";

    cout << YELLOW << "C" << RESET << " Chest  "
         << YELLOW << "T" << RESET << " Treasure  "
         << RED << "^" << RESET << " Trap  "
         << MAGENTA << "B" << RESET << " Boss  "
         << GOLD << "E" << RESET << " Exit\n";
}

// TITLE SCREEN
void showIntro() {
    clearScreen();

    cout << CYAN;
    cout << "========================================\n";
    cout << "          DUNGEON ESCAPE QUEST          \n";
    cout << "========================================\n";
    cout << "       ________________                 \n";
    cout << "      /                \\                \n";
    cout << "     |   DUNGEON GATE   |                \n";
    cout << "     |   [ ]      [ ]   |                \n";
    cout << "     |       ____       |                \n";
    cout << "     |__________________|                \n";
    cout << RESET;

    cout << "\n1. Start Game\n";
    cout << "2. How to Play\n";
    cout << "3. Exit\n";
    cout << "Choose: ";

    int choice;
    cin >> choice;

    if (choice == 2) {
        cout << "\nCONTROLS\n";
        cout << "W/A/S/D - Move\n";
        cout << "I       - Inventory\n";
        cout << "E       - Special ability\n";
        cout << "P       - Use a potion\n";
        cout << "Q       - Quit\n";
        cout << "\nDefeat the Ancient Dragon before using the exit.\n";
        cout << "Explore chests, collect loot, and survive three floors.\n";
        pauseGame();
        showIntro();
    }
    else if (choice == 3) {
        exit(0);
    }
}

// CLASS SELECTION
void chooseClass() {
    cout << "\nChoose your class:\n";
    cout << "1. Warrior - Starts with 20 armor\n";
    cout << "2. Mage - Choose Fire, Water, or Earth\n";
    cout << "3. Rogue - Starts with 50 gold\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;

    if (choice == 1) {
        player.classType = "Warrior";
        player.armor = 20;
    }
    else if (choice == 2) {
        player.classType = "Mage";

        cout << "\nChoose your element:\n";
        cout << "1. Fire - Bonus damage\n";
        cout << "2. Water - Bonus health\n";
        cout << "3. Earth - Bonus armor\n";
        cout << "Choice: ";

        int power;
        cin >> power;

        if (power == 1) {
            player.element = "Fire";
            player.attackPower += 3;
        }
        else if (power == 2) {
            player.element = "Water";
            player.hp = 120;
        }
        else {
            player.element = "Earth";
            player.armor = 15;
        }
    }
    else if (choice == 3) {
        player.classType = "Rogue";
        player.gold = 50;
    }
    else {
        player.classType = "Adventurer";
    }

    cout << GREEN << "\nClass selected: "
         << player.classType;

    if (!player.element.empty())
        cout << " (" << player.element << ")";

    cout << RESET << "\n";
}

// PLACE OBJECT ON A RANDOM EMPTY CELL
void placeOnEmptyCell(CellType type, int& x, int& y) {
    int attempts = 0;

    do {
        x = rand() % MAP_SIZE;
        y = rand() % MAP_SIZE;
        attempts++;
    } while ((dungeon[x][y].type != EMPTY ||
             (x == 0 && y == 0)) && attempts < 1000);

    if (dungeon[x][y].type == EMPTY)
        dungeon[x][y].type = type;
}

// ENEMY GENERATION
void generateEnemies() {
    enemyCount = 0;

    for (int i = 0; i < 7; i++) {
        int x, y;
        placeOnEmptyCell(ENEMY, x, y);

        int type = rand() % 4;

        Enemy e;
        e.x = x;
        e.y = y;
        e.alive = true;

        if (type == 0) {
            e.type = "Goblin";
            e.hp = 30 + currentFloor * 5;
            e.attackPower = 5 + currentFloor;
            e.armorDrop = 5;
            e.goldDrop = 10;
        }
        else if (type == 1) {
            e.type = "Orc";
            e.hp = 50 + currentFloor * 10;
            e.attackPower = 10 + currentFloor * 2;
            e.armorDrop = 10;
            e.goldDrop = 20;
        }
        else if (type == 2) {
            e.type = "Mimic";
            e.hp = 40 + currentFloor * 8;
            e.attackPower = 8 + currentFloor * 2;
            e.armorDrop = 8;
            e.goldDrop = 25;
        }
        else {
            e.type = "Dragon";
            e.hp = 80 + currentFloor * 15;
            e.attackPower = 15 + currentFloor * 2;
            e.armorDrop = 15;
            e.goldDrop = 40;
        }

        e.maxHP = e.hp;
        enemies[enemyCount++] = e;
    }
}

// MAP GENERATION
void initializeMap() {
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            dungeon[i][j].type = EMPTY;
            dungeon[i][j].explored = true;
        }
    }

    player.x = 0;
    player.y = 0;
    bossDefeated = false;

    // Generate traps, treasure and chests.
    for (int i = 0; i < 5; i++) {
        int x, y;
        placeOnEmptyCell(TRAP, x, y);
    }

    for (int i = 0; i < 3; i++) {
        int x, y;
        placeOnEmptyCell(TREASURE, x, y);
    }

    for (int i = 0; i < 5; i++) {
        int x, y;
        placeOnEmptyCell(CHEST, x, y);
    }

    generateEnemies();

    // Boss and exit are fixed near the end of the map.
    dungeon[MAP_SIZE - 2][MAP_SIZE - 2].type = BOSS;
    dungeon[MAP_SIZE - 1][MAP_SIZE - 1].type = EXIT;

    boss.type = "Ancient Dragon";
    boss.hp = 200 + (currentFloor - 1) * 75;
    boss.maxHP = boss.hp;
    boss.attackPower = 25 + (currentFloor - 1) * 5;
    boss.x = MAP_SIZE - 2;
    boss.y = MAP_SIZE - 2;
    boss.armorDrop = 30;
    boss.goldDrop = 100;
    boss.alive = true;

    // Keep the starting tile safe.
    dungeon[0][0].type = EMPTY;
}

// HEALTH AND ARMOR BARS
void displayBars() {
    int maxHP = player.element == "Water" ? 120 : MAX_HP;
    int hp = max(0, min(player.hp, maxHP));
    int hpBars = hp * 20 / maxHP;

    cout << GREEN << "HP:    [" << RESET;
    for (int i = 0; i < 20; i++) {
        if (i < hpBars) cout << GREEN << "#" << RESET;
        else cout << "-";
    }
    cout << "] " << player.hp << "/" << maxHP << "\n";

    int armorBars = min(player.armor, MAX_ARMOR) * 20 / MAX_ARMOR;

    cout << CYAN << "Armor: [" << RESET;
    for (int i = 0; i < 20; i++) {
        if (i < armorBars) cout << CYAN << "#" << RESET;
        else cout << "-";
    }
    cout << "] " << player.armor << "/" << MAX_ARMOR << "\n";
}

// PLAYER HUD
void displayHUD() {
    cout << "\n+--------------------------------------+\n";
    cout << "|             PLAYER STATUS            |\n";
    cout << "+--------------------------------------+\n";

    cout << "Name: " << player.name
         << " | Class: " << player.classType;

    if (!player.element.empty())
        cout << " (" << player.element << ")";

    cout << "\nFloor: " << currentFloor << "/" << MAX_FLOORS
         << " | Gold: " << player.gold << "\n";

    cout << "Weapon damage: " << player.attackPower
         << " | Health potions: " << player.healthPotions
         << " | Strength potions: " << player.strengthPotions << "\n";

    displayBars();

    cout << "+--------------------------------------+\n";
}

// DISPLAY MAP
void displayMap() {
    clearScreen();

    cout << CYAN;
    cout << "========================================\n";
    cout << "          DUNGEON ESCAPE QUEST          \n";
    cout << "========================================\n";
    cout << RESET;

    cout << "Floor " << currentFloor << "\n\n";

    cout << "+";
    for (int j = 0; j < MAP_SIZE; j++)
        cout << "---";
    cout << "+\n";

    for (int i = 0; i < MAP_SIZE; i++) {
        cout << "|";

        for (int j = 0; j < MAP_SIZE; j++) {
            if (player.x == i && player.y == j) {
                cout << GREEN << " P " << RESET;
                continue;
            }

            CellType type = dungeon[i][j].type;

            if (type == ENEMY) {
                bool found = false;

                for (int e = 0; e < enemyCount; e++) {
                    if (enemies[e].alive &&
                        enemies[e].x == i && enemies[e].y == j) {

                        found = true;

                        if (enemies[e].type == "Goblin")
                            cout << RED << " G " << RESET;
                        else if (enemies[e].type == "Orc")
                            cout << RED << " O " << RESET;
                        else if (enemies[e].type == "Mimic")
                            cout << RED << " M " << RESET;
                        else
                            cout << RED << " D " << RESET;

                        break;
                    }
                }

                if (!found) cout << " . ";
            }
            else if (type == BOSS && !bossDefeated) {
                cout << GOLD << " B " << RESET;
            }
            else if (type == EXIT) {
                cout << GOLD << " E " << RESET;
            }
            else if (type == CHEST) {
                cout << YELLOW << " C " << RESET;
            }
            else if (type == TRAP) {
                cout << RED << " ^ " << RESET;
            }
            else if (type == TREASURE) {
                cout << YELLOW << " T " << RESET;
            }
            else if (type == WALL) {
                cout << WHITE << " # " << RESET;
            }
            else {
                cout << " . ";
            }
        }

        cout << "|\n";
    }

    cout << "+";
    for (int j = 0; j < MAP_SIZE; j++)
        cout << "---";
    cout << "+\n";

    printLegend();
    displayHUD();
}

// ADD WEAPON TO INVENTORY
void addItem(string name, int effect) {
    if (player.inventoryCount >= MAX_ITEMS) {
        cout << RED << "Inventory full! The item was lost.\n" << RESET;
        return;
    }

    player.inventory[player.inventoryCount].name = name;
    player.inventory[player.inventoryCount].effect = effect;
    player.inventoryCount++;

    cout << YELLOW << "Added to inventory: " << name
         << " (Effect: " << effect << " damage)\n" << RESET;
}

// CHEST LOOT
void openChest() {
    cout << YELLOW << "\nYou opened a chest!\n" << RESET;

    int roll = rand() % 100;

    // 70% Wooden Sword, 10% Dragon Slayer.
    if (roll < 70) {
        addItem("Wooden Sword", 10);
    }
    else if (roll < 80) {
        addItem("The Dragon Slayer", 35);
    }
    else if (roll < 90) {
        player.armor = min(MAX_ARMOR, player.armor + 30);
        cout << CYAN << "You found armor! +30 Armor!\n" << RESET;
        displayBars();
    }
    else if (rand() % 2 == 0) {
        player.healthPotions++;
        cout << GREEN << "You found a Health Potion! (+20 HP)\n" << RESET;
    }
    else {
        player.strengthPotions++;
        cout << MAGENTA << "You found a Strength Potion! (+5 damage)\n" << RESET;
    }

    dungeon[player.x][player.y].type = EMPTY;
}

// ENEMY DROPS
void enemyLoot(Enemy& enemy) {
    cout << YELLOW << "\nLoot from " << enemy.type << ":\n" << RESET;

    player.gold += enemy.goldDrop;
    cout << GOLD << "+" << enemy.goldDrop << " gold!\n" << RESET;

    if (enemy.armorDrop > 0) {
        player.armor = min(MAX_ARMOR,
                           player.armor + enemy.armorDrop);
        cout << CYAN << "+" << enemy.armorDrop
             << " armor!\n" << RESET;
    }

    int roll = rand() % 100;

    if (roll < 20) {
        player.healthPotions++;
        cout << GREEN << "The enemy dropped a Health Potion!\n" << RESET;
    }
    else if (roll < 35) {
        player.strengthPotions++;
        cout << MAGENTA << "The enemy dropped a Strength Potion!\n" << RESET;
    }
}

// BATTLE SYSTEM
void battle(Enemy& enemy, bool isBoss) {
    cout << "\n";

    if (isBoss)
        cout << GOLD << "ANCIENT DRAGON BOSS BATTLE!\n" << RESET;
    else
        cout << RED << "You encountered a " << enemy.type << "!\n" << RESET;

    while (player.hp > 0 && enemy.hp > 0) {
        cout << "\nYour HP: " << player.hp
             << " | Armor: " << player.armor << "\n";

        cout << enemy.type << " HP: "
             << enemy.hp << "/" << enemy.maxHP << "\n";

        cout << "1. Attack\n";
        cout << "2. Use special ability\n";
        cout << "3. Use health potion\n";
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (choice == 1) {
            enemy.hp -= player.attackPower;

            cout << GREEN << "You deal "
                 << player.attackPower << " damage!\n" << RESET;
        }
        else if (choice == 2) {
            if (player.classType == "Warrior") {
                int damage = player.attackPower + 10;
                enemy.hp -= damage;
                player.armor = min(MAX_ARMOR, player.armor + 5);
                cout << GREEN << "Power Strike! Damage: "
                     << damage << ". +5 Armor!\n" << RESET;
            }
            else if (player.classType == "Mage") {
                int damage = 15;

                if (player.element == "Fire") {
                    damage = 25;
                    cout << RED << "Fire Blast!\n" << RESET;
                }
                else if (player.element == "Water") {
                    player.hp = min(120, player.hp + 20);
                    cout << CYAN << "Water Heal! +20 HP!\n" << RESET;
                }
                else if (player.element == "Earth") {
                    player.armor = min(MAX_ARMOR, player.armor + 20);
                    cout << YELLOW << "Earth Shield! +20 Armor!\n" << RESET;
                }

                enemy.hp -= damage;
                cout << "Elemental damage: " << damage << "\n";
            }
            else {
                enemy.hp -= player.attackPower + 8;
                cout << MAGENTA << "Rogue's Critical Strike!\n" << RESET;
            }
        }
        else if (choice == 3) {
            if (player.healthPotions > 0) {
                player.hp = min(player.element == "Water" ? 120 : MAX_HP,
                                player.hp + 20);
                player.healthPotions--;
                cout << GREEN << "Restored 20 HP!\n" << RESET;
            }
            else {
                cout << RED << "No health potions!\n" << RESET;
                continue;
            }
        }
        else {
            cout << "Invalid choice!\n";
            continue;
        }

        if (enemy.hp <= 0)
            break;

        int damage = enemy.attackPower;

        cout << RED << enemy.type << " attacks for "
             << damage << " damage!\n" << RESET;

        int absorbed = min(player.armor, damage);
        player.armor -= absorbed;
        damage -= absorbed;
        player.hp -= damage;

        cout << "Armor absorbed " << absorbed << " damage.\n";
        displayBars();
    }

    if (player.hp <= 0) {
        cout << RED << "\nYou were defeated!\n" << RESET;
        return;
    }

    enemy.hp = 0;
    enemy.alive = false;

    cout << GREEN << "\nYou defeated " << enemy.type << "!\n" << RESET;

    enemyLoot(enemy);

    if (isBoss) {
        bossDefeated = true;
        dungeon[boss.x][boss.y].type = EMPTY;

        cout << GOLD;
        cout << "\n====================================\n";
        cout << "      ANCIENT DRAGON DEFEATED!       \n";
        cout << "       THE EXIT IS NOW OPEN!         \n";
        cout << "====================================\n";
        cout << RESET;
    }

    displayBars();
}

// CELL EVENTS
void handleCellEvent(int x, int y) {
    DungeonCell& cell = dungeon[x][y];

    if (cell.type == CHEST) {
        openChest();
    }
    else if (cell.type == TREASURE) {
        int gold = rand() % 31 + 10;
        player.gold += gold;

        cout << GOLD << "You found " << gold
             << " gold!\n" << RESET;

        cell.type = EMPTY;
    }
    else if (cell.type == TRAP) {
        int damage = rand() % 26 + 5;

        cout << RED << "\nTRAP! You take "
             << damage << " damage!\n" << RESET;

        int absorbed = min(player.armor, damage);
        player.armor -= absorbed;
        damage -= absorbed;
        player.hp -= damage;

        cout << "Armor absorbed " << absorbed << " damage.\n";
        displayBars();

        cell.type = EMPTY;
    }
    else if (cell.type == ENEMY) {
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].alive &&
                enemies[i].x == x && enemies[i].y == y) {
                battle(enemies[i]);
                if (!enemies[i].alive)
                    cell.type = EMPTY;
                break;
            }
        }
    }
    else if (cell.type == BOSS && !bossDefeated) {
        battle(boss, true);
    }
    else if (cell.type == EXIT) {
        if (!bossDefeated) {
            cout << RED
                 << "The exit is sealed! Defeat the Ancient Dragon first!\n"
                 << RESET;
        }
        else {
            cout << GREEN << "Exit unlocked!\n" << RESET;
        }
    }
}

// MOVEMENT
void movePlayer(char move) {
    int nx = player.x;
    int ny = player.y;

    if (move == 'W') nx--;
    else if (move == 'S') nx++;
    else if (move == 'A') ny--;
    else if (move == 'D') ny++;
    else return;

    if (nx < 0 || nx >= MAP_SIZE ||
        ny < 0 || ny >= MAP_SIZE) {
        cout << RED << "You cannot leave the map!\n" << RESET;
        return;
    }

    if (dungeon[nx][ny].type == WALL) {
        cout << RED << "A wall blocks your way!\n" << RESET;
        return;
    }

    if (dungeon[nx][ny].type == EXIT && !bossDefeated) {
        cout << RED << "Defeat the Ancient Dragon to unlock the exit!\n" << RESET;
        return;
    }

    player.x = nx;
    player.y = ny;

    handleCellEvent(nx, ny);
}

// INVENTORY
void equipWeapon() {
    if (player.inventoryCount == 0) {
        cout << RED << "You have no weapons in your inventory.\n" << RESET;
        return;
    }

    cout << "\nAvailable weapons:\n";

    for (int i = 0; i < player.inventoryCount; i++) {
        cout << i + 1 << ". " << player.inventory[i].name
             << " (Damage: " << player.inventory[i].effect << ")\n";
    }

    cout << "Choose weapon number (0 to cancel): ";

    int choice;
    cin >> choice;

    if (choice == 0)
        return;

    if (choice < 1 || choice > player.inventoryCount) {
        cout << RED << "Invalid choice.\n" << RESET;
        return;
    }

    Item selected = player.inventory[choice - 1];

    player.attackPower = selected.effect;

    cout << GREEN << "Equipped " << selected.name
         << "! Damage is now " << player.attackPower << ".\n" << RESET;

    for (int i = choice - 1; i < player.inventoryCount - 1; i++)
        player.inventory[i] = player.inventory[i + 1];

    player.inventoryCount--;
}

void usePotion() {
    cout << "\n1. Health Potion (+20 HP)\n";
    cout << "2. Strength Potion (+5 damage)\n";
    cout << "0. Cancel\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;

    if (choice == 1 && player.healthPotions > 0) {
        int maxHP = player.element == "Water" ? 120 : MAX_HP;
        player.hp = min(maxHP, player.hp + 20);
        player.healthPotions--;
        cout << GREEN << "Health restored!\n" << RESET;
        displayBars();
    }
    else if (choice == 2 && player.strengthPotions > 0) {
        player.attackPower += 5;
        player.strengthPotions--;
        cout << GREEN << "Attack power increased by 5!\n" << RESET;
    }
    else if (choice == 0) {
        return;
    }
    else {
        cout << RED << "You don't have that potion!\n" << RESET;
    }
}

void showInventory() {
    cout << "\n========== INVENTORY ==========\n";
    cout << "Health Potions: " << player.healthPotions << "\n";
    cout << "Strength Potions: " << player.strengthPotions << "\n";
    cout << "Gold: " << player.gold << "\n";
    cout << "Current damage: " << player.attackPower << "\n";

    cout << "\nOther Items:\n";

    if (player.inventoryCount == 0) {
        cout << "No weapons stored.\n";
    }
    else {
        for (int i = 0; i < player.inventoryCount; i++) {
            cout << i + 1 << ". " << player.inventory[i].name
                 << " (Effect: " << player.inventory[i].effect
                 << " damage)\n";
        }
    }

    cout << "\n1. Equip weapon\n";
    cout << "2. Use potion\n";
    cout << "0. Return\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;

    if (choice == 1)
        equipWeapon();
    else if (choice == 2)
        usePotion();
}

// SPECIAL ABILITY OUTSIDE BATTLE
void useSpecialAbility() {
    if (player.classType == "Warrior") {
        player.armor = min(MAX_ARMOR, player.armor + 15);
        cout << CYAN << "Warrior Shield! +15 armor!\n" << RESET;
    }
    else if (player.classType == "Mage") {
        if (player.element == "Fire") {
            cout << RED << "Fire magic ready! Use it in battle for extra damage.\n" << RESET;
        }
        else if (player.element == "Water") {
            player.hp = min(120, player.hp + 15);
            cout << CYAN << "Water Heal! +15 HP!\n" << RESET;
        }
        else {
            player.armor = min(MAX_ARMOR, player.armor + 15);
            cout << YELLOW << "Earth Shield! +15 armor!\n" << RESET;
        }
    }
    else if (player.classType == "Rogue") {
        int gold = rand() % 16 + 5;
        player.gold += gold;
        cout << MAGENTA << "Rogue found " << gold << " extra gold!\n" << RESET;
    }

    displayBars();
}

// HIGH SCORES
void saveHighScore() {
    ofstream file("highscores.txt", ios::app);

    if (file.is_open()) {
        file << player.name
             << " | Floor: " << currentFloor
             << " | HP: " << player.hp
             << " | Armor: " << player.armor
             << " | Gold: " << player.gold << "\n";
        file.close();
    }
}

// NEXT FLOOR
void nextFloor() {
    if (currentFloor >= MAX_FLOORS) {
        cout << GOLD;
        cout << "\n====================================\n";
        cout << "        DUNGEON CONQUERED!           \n";
        cout << "       YOU ESCAPED THE DUNGEON!      \n";
        cout << "====================================\n";
        cout << RESET;

        saveHighScore();
        exit(0);
    }

    currentFloor++;

    cout << CYAN << "\nDescending to floor "
         << currentFloor << "...\n" << RESET;

    initializeMap();
    pauseGame();
}

// MAIN
int main() {
    srand(static_cast<unsigned int>(time(0)));

    showIntro();

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\nEnter your player name: ";
    getline(cin, player.name);

    chooseClass();

    initializeMap();

    while (player.hp > 0) {
        displayMap();

        cout << "\nMove (W/A/S/D), Inventory (I), Ability (E), ";
        cout << "Potion (P), Quit (Q): ";

        char move;
        cin >> move;
        move = static_cast<char>(toupper(static_cast<unsigned char>(move)));

        if (move == 'Q') {
            cout << "Thanks for playing!\n";
            break;
        }
        else if (move == 'I') {
            showInventory();
        }
        else if (move == 'E') {
            useSpecialAbility();
            pauseGame();
        }
        else if (move == 'P') {
            usePotion();
            pauseGame();
        }
        else {
            movePlayer(move);
        }

        if (player.hp <= 0) {
            cout << RED << "\nGAME OVER! You have fallen in the dungeon.\n" << RESET;
            saveHighScore();
            break;
        }

        if (player.x == MAP_SIZE - 1 &&
            player.y == MAP_SIZE - 1 &&
            bossDefeated) {
            if (currentFloor == MAX_FLOORS) {
                nextFloor();
            }
            else {
                nextFloor();
            }
        }

        pauseGame();
    }

    return 0;
}