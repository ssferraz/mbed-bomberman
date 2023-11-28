#include "mbed.h"
#include <iostream>

#define BOARD_WIDTH 76
#define BOARD_HEIGHT 48
#define PLAYER_SIZE 2
#define BOMB_SIZE 2
#define EXPLOSION_SIZE BOARD_WIDTH
#define BLINKING_RATE 10ms

AnalogIn joystickXPlayer1(PA_7);
AnalogIn joystickYPlayer1(PA_6);
AnalogIn joystickXPlayer2(PC_0);
AnalogIn joystickYPlayer2(PC_1);

DigitalIn button(PC_14);

char board[BOARD_HEIGHT][BOARD_WIDTH];

struct Player {
    int x, y;
    bool hasBomb;
};

struct Bomb {
    int x, y;
    int explosionSize;
    bool exploded;
};

Player player1 = {2, 2, false};
Player player2 = {BOARD_WIDTH - 3, BOARD_HEIGHT - 3, false};
Bomb bomb = {0, 0, 0, false};

void initializeBoard() {
    memset(board, ' ', sizeof(board));

    for (int i = 0; i < BOARD_HEIGHT; ++i) {
        board[i][0] = '#';
        board[i][BOARD_WIDTH - 1] = '#';
    }
    for (int j = 0; j < BOARD_WIDTH; ++j) {
        board[0][j] = '#';
        board[BOARD_HEIGHT - 1][j] = '#';
    }

    for (int i = 4; i < BOARD_HEIGHT - 4; ++i) {
        for (int j = 4; j < BOARD_WIDTH - 4; ++j) {
            if (i % 2 == 0 && j % 2 == 0) {
                board[i][j] = '#';
            }
        }
    }

    board[player1.y][player1.x] = 'P';
    board[player2.y][player2.x] = 'P';
}

void placeBomb(Player &player) {
    if (!player.hasBomb) {
        bomb.x = player.x;
        bomb.y = player.y;
        bomb.explosionSize = EXPLOSION_SIZE;
        bomb.exploded = false;
        player.hasBomb = true;
    }
}

void explodeBomb() {
    if (bomb.exploded) {
        return;
    }

    if (player1.x >= bomb.x && player1.x < bomb.x + BOMB_SIZE &&
        player1.y >= bomb.y && player1.y < bomb.y + BOMB_SIZE) {
        printf("Jogador 1 foi atingido pela explosao da bomba! Jogo encerrado.\n");
    }

    if (player2.x >= bomb.x && player2.x < bomb.x + BOMB_SIZE &&
        player2.y >= bomb.y && player2.y < bomb.y + BOMB_SIZE) {
        printf("Jogador 2 foi atingido pela explosao da bomba! Jogo encerrado.\n");
    }

    for (int i = bomb.x - 1; i >= bomb.x - bomb.explosionSize && i >= 0; --i) {
        board[bomb.y][i] = 'Y';
    }
    for (int i = bomb.x + BOMB_SIZE; i <= bomb.x + bomb.explosionSize && i < BOARD_WIDTH; ++i) {
        board[bomb.y][i] = 'Y';
    }

    for (int i = bomb.y - 1; i >= bomb.y - bomb.explosionSize && i >= 0; --i) {
        board[i][bomb.x] = 'Y';
    }
    for (int i = bomb.y + BOMB_SIZE; i <= bomb.y + bomb.explosionSize && i < BOARD_HEIGHT; ++i) {
        board[i][bomb.x] = 'Y';
    }

    bomb.exploded = true;
}

void printBoard() {
   /* std::cout  << "\033[2J" << std::endl;*/
    for (int i = 0; i < BOARD_HEIGHT; ++i) {
        for (int j = 0; j < BOARD_WIDTH; ++j) {
            if (board[i][j] == 'Y') {
                std::cout << "\033[1;33m" << board[i][j] << "\033[0m";
            } else {
                std::cout << board[i][j];
            }
        }
        std::cout << "\n";
    }
}

bool buttonPressed() {
    return !button;
}

bool isPlayerInsideExplosion(Player player, Bomb bomb) {
    return player.x >= bomb.x && player.x < bomb.x + bomb.explosionSize &&
           player.y >= bomb.y && player.y < bomb.y + bomb.explosionSize;
}

void movePlayer(Player &player, float xValue, float yValue) {
    float angle = atan2(yValue - 0.5, xValue - 0.5);
    int dx = static_cast<int>(cos(angle) * 2);
    int dy = static_cast<int>(sin(angle) * 2);

    int newPlayerX = player.x + dx;
    int newPlayerY = player.y + dy;

    if (board[newPlayerY][newPlayerX] != '#' &&
        board[newPlayerY + PLAYER_SIZE - 1][newPlayerX + PLAYER_SIZE - 1] != '#') {
        board[player.y][player.x] = ' ';
        player.x = newPlayerX;
        player.y = newPlayerY;
        board[player.y][player.x] = 'P';
    }
}

int main() {
    BufferedSerial pc(USBTX, USBRX, 384000); 
    initializeBoard();

    while (true) {
        float xValuePlayer1 = joystickXPlayer1.read();
        float yValuePlayer1 = joystickYPlayer1.read();

        movePlayer(player1, xValuePlayer1, yValuePlayer1);

        float xValuePlayer2 = joystickXPlayer2.read();
        float yValuePlayer2 = joystickYPlayer2.read();
        movePlayer(player2, xValuePlayer2, yValuePlayer2);

        if (buttonPressed() && !player1.hasBomb) {
            placeBomb(player1);
        }
        if (buttonPressed() && !player2.hasBomb) {
            placeBomb(player2);
        }

        explodeBomb();

        if (isPlayerInsideExplosion(player1, bomb) && !player1.hasBomb) {
            printf("Jogador 1 foi atingido pela explosao da bomba! Jogo encerrado.\n");
            break;
        }
        if (isPlayerInsideExplosion(player2, bomb) && !player2.hasBomb) {
            printf("Jogador 2 foi atingido pela explosao da bomba! Jogo encerrado.\n");
            break;
        }

        printBoard();

        
        ThisThread::sleep_for(BLINKING_RATE);
    }

    return 0;
}
