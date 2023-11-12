#include "mbed.h"
#include <iostream>

#define BLINKING_RATE     1000ms

#define WIDTH 76
#define HEIGHT 72
#define SIZE 2
#define BOMB 2
#define EXPLOSION WIDTH

AnalogIn joyX1(PC_0);
AnalogIn joyY1(PC_1);
AnalogIn joyX2(PA_7);
AnalogIn joyY2(PA_6);

DigitalIn btn1(PC_14);
DigitalIn btn2(PB_10);

char board[HEIGHT][WIDTH];

struct Player {
    int x, y;
    bool hasBomb;
};

struct Bomb {
    int x, y;
    int explosionSize;
    bool exploded;
};

Player p1 = {2, 2, false};
Player p2 = {WIDTH - 3, HEIGHT - 3, false};
Bomb bomb = {0, 0, 0, false};

void initBoard() {
    memset(board, ' ', sizeof(board));

    for (int i = 0; i < HEIGHT; ++i) {
        board[i][0] = '#';
        board[i][WIDTH - 1] = '#';
    }
    for (int j = 0; j < WIDTH; ++j) {
        board[0][j] = '#';
        board[HEIGHT - 1][j] = '#';
    }

    for (int i = 4; i < HEIGHT - 4; ++i) {
        for (int j = 4; j < WIDTH - 4; ++j) {
            if (i % 2 == 0 && j % 2 == 0) {
                board[i][j] = '#';
            }
        }
    }

    board[p1.y][p1.x] = 'P';
    board[p2.y][p2.x] = 'P';
}

void plantBomb(Player &player) {
    if (!player.hasBomb) {
        bomb.x = player.x;
        bomb.y = player.y;
        bomb.explosionSize = EXPLOSION;
        bomb.exploded = false;
        player.hasBomb = true;
    }
}

void igniteBomb() {
    if (bomb.exploded) {
        return;
    }

    if (p1.x >= bomb.x && p1.x < bomb.x + BOMB &&
        p1.y >= bomb.y && p1.y < bomb.y + BOMB) {
        printf("Jogador 1 se deu mal! Jogo acabou.\n");
    }

    if (p2.x >= bomb.x && p2.x < bomb.x + BOMB &&
        p2.y >= bomb.y && p2.y < bomb.y + BOMB) {
        printf("Jogador 2 se deu mal! Jogo acabou.\n");
    }

    for (int i = bomb.x - 1; i >= bomb.x - bomb.explosionSize && i >= 0; --i) {
        board[bomb.y][i] = 'Y';
    }
    for (int i = bomb.x + BOMB; i <= bomb.x + bomb.explosionSize && i < WIDTH; ++i) {
        board[bomb.y][i] = 'Y';
    }

    for (int i = bomb.y - 1; i >= bomb.y - bomb.explosionSize && i >= 0; --i) {
        board[i][bomb.x] = 'Y';
    }
    for (int i = bomb.y + BOMB; i <= bomb.y + bomb.explosionSize && i < HEIGHT; ++i) {
        board[i][bomb.x] = 'Y';
    }

    bomb.exploded = true;
}

void showBoard() {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (board[i][j] == 'Y') {
                std::cout << "\033[1;33m" << board[i][j] << "\033[0m";
            } else {
                std::cout << board[i][j];
            }
        }
        std::cout << std::endl;
    }
}

bool pressButton1() {
    return !btn1;
}

bool pressButton2() {
    return !btn2;
}

bool inExplosion(Player player, Bomb bomb) {
    return player.x >= bomb.x && player.x < bomb.x + bomb.explosionSize &&
           player.y >= bomb.y && player.y < bomb.y + bomb.explosionSize;
}

int main() {
    initBoard();

    while (true) {
        float xJoy1 = joyX1.read();
        float yJoy1 = joyY1.read();
        float aJoy1 = atan2(yJoy1 - 0.5, xJoy1 - 0.5);

        int dxJoy1 = static_cast<int>(cos(aJoy1) * 2);
        int dyJoy1 = static_cast<int>(sin(aJoy1) * 2);

        int nxJoy1 = p1.x + dxJoy1;
        int nyJoy1 = p1.y + dyJoy1;

        if (board[nyJoy1][nxJoy1] != '#' &&
            board[nyJoy1 + SIZE - 1][nxJoy1 + SIZE - 1] != '#') {
            board[p1.y][p1.x] = ' ';
            p1.x = nxJoy1;
            p1.y = nyJoy1;
            board[p1.y][p1.x] = 'P';
        }

        float xJoy2 = joyX2.read();
        float yJoy2 = joyY2.read();
        float aJoy2 = atan2(yJoy2 - 0.5, xJoy2 - 0.5);

        int dxJoy2 = static_cast<int>(cos(aJoy2) * 2);
        int dyJoy2 = static_cast<int>(sin(aJoy2) * 2);

        int nxJoy2 = p2.x + dxJoy2;
        int nyJoy2 = p2.y + dyJoy2;

        if (board[nyJoy2][nxJoy2] != '#' &&
            board[nyJoy2 + SIZE - 1][nxJoy2 + SIZE - 1] != '#') {
            board[p2.y][p2.x] = ' ';
            p2.x = nxJoy2;
            p2.y = nyJoy2;
            board[p2.y][p2.x] = 'P';
        }

        if (pressButton1() && !p1.hasBomb) {
            plantBomb(p1);
        }
        if (pressButton2() && !p2.hasBomb) {
            plantBomb(p2);
        }

        igniteBomb();

        if (inExplosion(p1, bomb) && !p1.hasBomb) {
            printf("Jogador 1 se deu mal! Jogo acabou.\n");
            break;
        }
        if (inExplosion(p2, bomb) && !p2.hasBomb) {
            printf("Jogador 2 se deu mal! Jogo acabou.\n");
            break;
        }

        showBoard();

        ThisThread::sleep_for(BLINKING_RATE);
    }

    return 0;
}
