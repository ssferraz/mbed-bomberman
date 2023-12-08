#include "mbed.h"
#include "joystick.h"
#include <iostream>
#include <chrono>

using namespace std::chrono;

#define BLINKING_RATE 1000ms
#define BOMB_TIMER 3000ms

#define WIDTH 19
#define HEIGHT 18
#define SIZE 2
#define BOMB 2
#define EXPLOSION WIDTH

DigitalOut led(LED1);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
DigitalOut mypin(PB_5, 1);
Joystick joystick1(PA_0, PA_1, PA_10);
Joystick joystick2(PA_4, PC_1, PB_3);
BufferedSerial pc(USBTX, USBRX, 384000);

FileHandle *mbed::mbed_override_console(int fd) {
    return &pc;
}

void clearConsole()
{
    pc.write("\033[2J\033[1;1H", 10);
}

char board[HEIGHT][WIDTH];

struct Player {
    int x, y;
    bool hasBomb;
};

struct Bomb {
    int x, y;
    int explosionSize;
    bool exploded;
    Timer timer;
    char representation;
};

Player p1 = {2, 2, false};
Player p2 = {WIDTH - 3, HEIGHT - 3, false};
Bomb bomb = {0, 0, 0, false, Timer(), 'B'};

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
        bomb.timer.reset();
        bomb.timer.start();
        player.hasBomb = true;
        board[bomb.y][bomb.x] = 'B';
    }
}

bool inExplosion(Player player, Bomb bomb) {
    return bomb.exploded &&
           player.x >= bomb.x && player.x < bomb.x + bomb.explosionSize &&
           player.y >= bomb.y && player.y < bomb.y + bomb.explosionSize;
}

void igniteBomb() {
    if (bomb.exploded) {
        board[bomb.y][bomb.x] = 'E';
        return;
    }

    if (bomb.timer.read_ms() >= duration_cast<milliseconds>(BOMB_TIMER).count()) {

        bool hitPlayer1 = inExplosion(p1, bomb) && !p1.hasBomb;
        bool hitPlayer2 = inExplosion(p2, bomb) && !p2.hasBomb;

        if (hitPlayer1 || hitPlayer2) {
            printf("Jogo acabou.\n");
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
}

void clearExplosion() {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (board[i][j] == 'Y') {
                board[i][j] = ' ';
            }
        }
    }
}

void showBoard() {
    clearConsole();
    for (int i = HEIGHT - 1; i >= 0; --i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (board[i][j] == 'B') {
                std::cout << "\033[1;34m" << board[i][j] << "\033[0m";
            } else if (board[i][j] == 'E') {
                std::cout << "\033[1;31m" << board[i][j] << "\033[0m";
            } else if (bomb.exploded && board[i][j] == 'Y') {
                std::cout << "\033[1;33m" << board[i][j] << "\033[0m";
            } else {
                std::cout << board[i][j];
            }
        }
        std::cout << std::endl;
    }
    clearExplosion();
}

bool positionsEqual(int x1, int y1, int x2, int y2) {
    return x1 == x2 && y1 == y2;
}

void movePlayer(Player &player, int dx, int dy, const Player &otherPlayer) {
    int newX = player.x + dx;
    int newY = player.y + dy;

    if (board[newY][newX] != '#' &&
        board[newY + SIZE - 1][newX + SIZE - 1] != '#' &&
        !positionsEqual(newX, newY, otherPlayer.x, otherPlayer.y)) {
        board[player.y][player.x] = ' ';
        player.x = newX;
        player.y = newY;
        board[player.y][player.x] = 'P';
    }
}


int main() {
    Thread evt;
    evt.start(callback(&queue, &EventQueue::dispatch_forever));

    initBoard();

    while (true) {
        showBoard();

        int dxJoy1 = 0, dyJoy1 = 0;
        int dxJoy2 = 0, dyJoy2 = 0;

        if (joystick1.readX() < 0.2) {
            dxJoy1 = -1;
        } else if (joystick1.readX() > 0.8) {
            dxJoy1 = 1;
        }
        if (joystick1.readY() < 0.2) {
            dyJoy1 = -1;
        } else if (joystick1.readY() > 0.8) {
            dyJoy1 = 1;
        }

        if (joystick2.readX() < 0.2) {
            dxJoy2 = -1;
        } else if (joystick2.readX() > 0.8) {
            dxJoy2 = 1;
        }
        if (joystick2.readY() < 0.2) {
            dyJoy2 = -1;
        } else if (joystick2.readY() > 0.8) {
            dyJoy2 = 1;
        }

        movePlayer(p1, dxJoy1, dyJoy1, p2);
        movePlayer(p2, dxJoy2, dyJoy2, p1);  

        if (joystick1.isButtonPressed() && !p1.hasBomb) {
            plantBomb(p1);
        }
        if (joystick2.isButtonPressed() && !p2.hasBomb) {
            plantBomb(p2);
        }

        igniteBomb();

        if (inExplosion(p1, bomb) && !p2.hasBomb) {
            printf("Jogador 1 se deu mal! Jogo acabou.\n");
            showBoard();
            break;
        }
        if (inExplosion(p2, bomb) && !p1.hasBomb) {
            printf("Jogador 2 se deu mal! Jogo acabou.\n");
            showBoard();
            break;
        }

        ThisThread::sleep_for(BLINKING_RATE);
    }

    return 0;
}
