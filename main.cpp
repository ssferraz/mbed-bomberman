#include "mbed.h"
#include <chrono>
#include <cstdio>
#include <iostream>

using namespace std::chrono;

#include "bomb.h"
#include "joystick.h"
#include "player.h"

#define TARGET_TX USBTX
#define TARGET_RX USBRX
#define BLINKING_RATE 400ms
#define BAUD_RATE 384000
#define PLAYER_FLAG1 (1UL << 0)
#define PLAYER_FLAG2 (1UL << 1)
#define CLEAR_FLAG3 (1UL << 3)

#define WIDTH 38
#define HEIGHT 36
#define EXPLOSION_SIZE WIDTH

#define COLOR_WHITE "\033[97m"

#define BG_MAGENTA "\033[45m"
#define BG_CYAN "\033[46m"
#define BG_RED "\033[101m"
#define BG_YELLOW "\033[103m"

#define RESET "\033[0m"

#define BOMB_SIZE 1

#define BLANK_SPACE ' '
#define BOMB 'B'
#define BOMB_EXPLODED 'Y'
#define PLAYER1_REPRESENTATION '1'
#define PLAYER2_REPRESENTATION '2'
#define OBSTACLE 'X'
#define EDGE '#'

#define BOMB_TIMER 3000ms

Thread thread_player1;
Thread thread_player2;
Thread thread_clearBoad;
EventFlags event_flags;
DigitalOut v3(PB_5, 1);

char board[WIDTH][HEIGHT];
Joystick joystick1(PA_0, PA_1, PA_10);
Joystick joystick2(PA_4, PC_1, PB_3);

Player player1(1, 1, PLAYER1_REPRESENTATION, true, 0);
Player player2(WIDTH - 3, HEIGHT - 3, PLAYER2_REPRESENTATION, true, 0);

Bomb bomb1(1, 1, 0, false, Timer(), BOMB);
Bomb bomb2(WIDTH - 3, HEIGHT - 3, 0, false, Timer(), BOMB);

bool gameOver = false;

void initBoard() {
  // Preenchendo a matriz com espaços em branco
  for (int i = 0; i < WIDTH; ++i) {
    for (int j = 0; j < HEIGHT; ++j) {
      board[i][j] = BLANK_SPACE;
    }
  }
  // Preenchendo as bordas com #
  for (int j = 0; j < HEIGHT - 1; ++j) {
    board[0][j] = EDGE;         // borda superior
    board[WIDTH - 2][j] = EDGE; // borda inferior
  }

  for (int i = 0; i < WIDTH - 1; ++i) {
    board[i][0] = EDGE;          // borda esquerda
    board[i][HEIGHT - 2] = EDGE; // borda direita
  }

  // Preenchendo os obstáculos com X
  for (int i = 2; i < WIDTH - 2; i += 2) {
    for (int j = 2; j < HEIGHT - 2; j += 2) {
      board[i][j] = OBSTACLE;
    }
  }
  // Preenchendo os jogadores 1 e 2 em suas posições iniciais
  board[player1.getX()][player1.getY()] = player1.getRepresentation();
  board[player2.getX()][player2.getY()] = player2.getRepresentation();
}

// Função para pintar o background da posição do tabuleiro
void setBackgroundColor(const char *color) { printf("%s", color); }

// Função para limpar o terminal
void reset() {
  cout << "\033[2J\033[H";
  cout << "\033[H";
}

// Função para limpar a explosão
void clearExplosion() {
  while (true) {
    event_flags.wait_any(CLEAR_FLAG3);
    for (int i = 0; i < WIDTH; ++i) {
      for (int j = 0; j < HEIGHT; ++j) {
        if (board[i][j] == BOMB_EXPLODED || board[i][j] == BOMB) {
          board[i][j] = BLANK_SPACE;
        }
      }
    }
    ThisThread::sleep_for(2000ms);
  }
}
// Função para mostrar o tabuleiro
void showBoard() {
  for (int i = 0; i < WIDTH; ++i) {
    for (int j = 0; j < HEIGHT; ++j) {
      if (board[i][j] == PLAYER1_REPRESENTATION) {
        setBackgroundColor(BG_MAGENTA);
      } else if (board[i][j] == PLAYER2_REPRESENTATION) {
        setBackgroundColor(BG_CYAN);
      } else if (board[i][j] == BOMB) {
        setBackgroundColor(BG_RED);
      } else if ((bomb1.isExploded() || bomb2.isExploded()) &&
                 board[i][j] == BOMB_EXPLODED) {
        setBackgroundColor(BG_YELLOW);
      } else {
        setBackgroundColor(RESET);
      }
      cout << board[i][j] << BLANK_SPACE;
    }
    cout << endl;
  }
}
// Função para plantar bomba no tabuleiro
void plantBomb(Player &player, Bomb &bomb) {
  bomb.setX(player.getX());
  bomb.setY(player.getY());
  bomb.setExploded(false);
  bomb.setBombPlated(true);
  bomb.timer.reset();
  bomb.timer.start();
  board[bomb.getX()][bomb.getY()] = BOMB;
  bomb.setExplosionSize(EXPLOSION_SIZE);
}

// Função que verifica se o jogador está na explosão
bool inExplosion(Player &player, Bomb &bomb) {
  return bomb.isExploded() &&
         (player.getX() == bomb.getX() || player.getY() == bomb.getY());
}

// Função para explodir a bomba
void igniteBomb(Player &player, Bomb &bomb) {
  if (bomb.isBombPlanted()) {
    if (bomb.timer.read_ms() >=
        duration_cast<milliseconds>(BOMB_TIMER).count()) {
      for (int i = bomb.getX() - 1;
           i >= bomb.getX() - bomb.getExplosionSize() && i >= 0; --i) {
        if (board[i][bomb.getY()] == OBSTACLE || board[i][bomb.getY()] == EDGE) {
          break;
        } else {
          board[i][bomb.getY()] = BOMB_EXPLODED;
        }
      }
      for (int i = (bomb.getX() + BOMB_SIZE);
           i <= bomb.getX() + bomb.getExplosionSize() && i < WIDTH; ++i) {
        if (board[i][bomb.getY()] == OBSTACLE || board[i][bomb.getY()] == EDGE) {
          break;
        } else {
          board[i][bomb.getY()] = BOMB_EXPLODED;
        }
      }

      for (int i = bomb.getY() - 1;
           i >= bomb.getY() - bomb.getExplosionSize() && i >= 0; --i) {
        if (board[bomb.getX()][i] == OBSTACLE || board[bomb.getX()][i] == EDGE) {
          break;
        } else {
          board[bomb.getX()][i] = BOMB_EXPLODED;
        }
      }
      for (int i = bomb.getY() + BOMB_SIZE;
           i <= bomb.getY() + bomb.getExplosionSize() && i < HEIGHT; ++i) {
        if (board[bomb.getX()][i] == OBSTACLE || board[bomb.getX()][i] == EDGE) {
          break;
        } else {
          board[bomb.getX()][i] = BOMB_EXPLODED;
        }
      }
      bomb.setExploded(true);
      bomb.setBombPlated(false);
    }

    bool hitPlayer1 = inExplosion(player1, bomb);
    bool hitPlayer2 = inExplosion(player2, bomb);

    if (hitPlayer1) {
      player1.setCountHit();
      gameOver = true;
    }
    if (hitPlayer2) {
      player2.setCountHit();
      gameOver = true;
    }
  }
}

void readPlayer1() {
  while (true) {
    event_flags.wait_any(PLAYER_FLAG1);
    float player1x = joystick1.readX();
    float player1y = joystick1.readY();
    // Player 1 moveu o joystick para esquerda
    if (player1x < 0.2) {
      // Verifica se a próxima posição do movimento não é um obstáculo ou uma
      // bomba
      if (board[player1.getX()][player1.getY() - 1] != EDGE &&
          board[player1.getX()][player1.getY() - 1] != OBSTACLE &&
          board[player1.getX()][player1.getY() - 1] != BOMB) {
        // Verifica se a próxima posição não é um oponente
        if (board[player1.getX()][player1.getY() - 1] != PLAYER2_REPRESENTATION) {
          // Apaga o jogador da posição atual e move ele para próxima posição
          if (board[player1.getX()][player1.getY()] != BOMB) {
            board[player1.getX()][player1.getY()] = BLANK_SPACE;
          }
          player1.moveLeft();
          board[player1.getX()][player1.getY()] = player1.getRepresentation();
        }
      }
      // Player 1 moveu o joystick para direita
    } else if (player1x > 0.8) {
      // Verifica se a próxima posição do movimento não é um obstáculo ou uma
      // bomba
      if (board[player1.getX()][player1.getY() + 1] != EDGE &&
          board[player1.getX()][player1.getY() + 1] != OBSTACLE &&
          board[player1.getX()][player1.getY() + 1] != BOMB) {
        // Verifica se a próxima posição não é um oponente
        if (board[player1.getX()][player1.getY() + 1] != PLAYER2_REPRESENTATION) {
          // Apaga o jogador da posição atual e move ele para próxima posição
          if (board[player1.getX()][player1.getY()] != BOMB) {
            board[player1.getX()][player1.getY()] = BLANK_SPACE;
          }
          player1.moveRight();
          board[player1.getX()][player1.getY()] = player1.getRepresentation();
        }
      }
    }
    // Player 1 moveu o joystick para cima
    if (player1y < 0.2) {
      // Verifica se a próxima posição do movimento não é um obstáculo ou uma
      // bomba
      if (board[player1.getX() - 1][player1.getY()] != EDGE &&
          board[player1.getX() - 1][player1.getY()] != OBSTACLE &&
          board[player1.getX() - 1][player1.getY()] != BOMB) {
        // Verifica se a próxima posição não é um oponente
        if (board[player1.getX() - 1][player1.getY()] != PLAYER2_REPRESENTATION) {
          // Apaga o jogador da posição atual e move ele para próxima posição
          if (board[player1.getX()][player1.getY()] != BOMB) {
            board[player1.getX()][player1.getY()] = BLANK_SPACE;
          }
          player1.moveUp();
          board[player1.getX()][player1.getY()] = player1.getRepresentation();
        }
      }
      // Player 1 moveu o joystick para baixo
    } else if (player1y > 0.8) {
      // Verifica se a próxima posição do movimento não é um obstáculo ou uma
      // bomba
      if (board[player1.getX() + 1][player1.getY()] != EDGE &&
          board[player1.getX() + 1][player1.getY()] != OBSTACLE &&
          board[player1.getX() + 1][player1.getY()] != BOMB) {
        // Verifica se a próxima posição não é um oponente
        if (board[player1.getX() + 1][player1.getY()] != PLAYER2_REPRESENTATION) {
          // Apaga o jogador da posição atual e move ele para próxima posição
          if (board[player1.getX()][player1.getY()] != BOMB) {
            board[player1.getX()][player1.getY()] = BLANK_SPACE;
          }
          player1.moveDown();
          board[player1.getX()][player1.getY()] = player1.getRepresentation();
        }
      }
    }
    // Player 1 clicou para plantar a bomba
    if (joystick1.isButtonPressed() && player1.getHasBomb()) {
      plantBomb(player1, bomb1);
    }
    igniteBomb(player1, bomb1);
    event_flags.set(CLEAR_FLAG3);
    ThisThread::sleep_for(500ms);
    event_flags.set(PLAYER_FLAG1);
  }
}

void readPlayer2() {
  while (true) {
    event_flags.wait_any(PLAYER_FLAG2);
    float player2x = joystick2.readX();
    float player2y = joystick2.readY();
    // Player 2 moveu o joystick para esquerda
    if (player2x < 0.2) {
      // Verifica se a próxima posição do movimento não é um obstáculo ou uma
      // bomba
      if (board[player2.getX()][player2.getY() - 1] != EDGE &&
          board[player2.getX()][player2.getY() - 1] != OBSTACLE &&
          board[player2.getX()][player2.getY() - 1] != BOMB) {
        // Verifica se a próxima posição não é um oponente
        if (board[player2.getX()][player2.getY() - 1] != PLAYER1_REPRESENTATION) {
          // Apaga o jogador da posição atual e move ele para próxima posição
          if (board[player2.getX()][player2.getY()] != BOMB) {
            board[player2.getX()][player2.getY()] = BLANK_SPACE;
          }
          player2.moveLeft();
          board[player2.getX()][player2.getY()] = player2.getRepresentation();
        }
      }
      // Player 2 moveu o joystick para direita
    } else if (player2x > 0.8) {
      // Verifica se a próxima posição do movimento não é um obstáculo ou uma
      // bomba
      if (board[player2.getX()][player2.getY() + 1] != EDGE &&
          board[player2.getX()][player2.getY() + 1] != OBSTACLE &&
          board[player2.getX()][player2.getY() + 1] != BOMB) {
        // Verifica se a próxima posição não é um oponente
        if (board[player2.getX()][player2.getY() + 1] != PLAYER1_REPRESENTATION) {
          // Apaga o jogador da posição atual e move ele para próxima posição
          if (board[player2.getX()][player2.getY()] != BOMB) {
            board[player2.getX()][player2.getY()] = BLANK_SPACE;
          }
          player2.moveRight();
          board[player2.getX()][player2.getY()] = player2.getRepresentation();
        }
      }
    }
    // Player 2 moveu o joystick para cima
    if (player2y < 0.2) {
      // Verifica se a próxima posição do movimento não é um obstáculo ou uma
      // bomba
      if (board[player2.getX() - 1][player2.getY()] != EDGE &&
          board[player2.getX() - 1][player2.getY()] != OBSTACLE &&
          board[player2.getX() - 1][player2.getY()] != BOMB) {
        // Verifica se a próxima posição não é um oponente
        if (board[player2.getX() - 1][player2.getY()] != PLAYER1_REPRESENTATION) {
          // Apaga o jogador da posição atual e move ele para próxima posição
          if (board[player2.getX()][player2.getY()] != BOMB) {
            board[player2.getX()][player2.getY()] = BLANK_SPACE;
          }
          player2.moveUp();
          board[player2.getX()][player2.getY()] = player2.getRepresentation();
        }
      }
      // Player 2 moveu o joystick para baixo
    } else if (player2y > 0.8) {
      // Verifica se a próxima posição do movimento não é um obstáculo ou uma
      // bomba
      if (board[player2.getX() + 1][player2.getY()] != EDGE &&
          board[player2.getX() + 1][player2.getY()] != OBSTACLE &&
          board[player2.getX() + 1][player2.getY()] != BOMB) {
        // Verifica se a próxima posição não é um oponente
        if (board[player2.getX() + 1][player2.getY()] != PLAYER1_REPRESENTATION) {
          // Apaga o jogador da posição atual e move ele para próxima posição
          if (board[player2.getX()][player2.getY()] != BOMB) {
            board[player2.getX()][player2.getY()] = BLANK_SPACE;
          }
          player2.moveDown();
          board[player2.getX()][player2.getY()] = player2.getRepresentation();
        }
      }
    }
    // Player 2 clicou para plantar a bomba
    if (joystick2.isButtonPressed() && player2.getHasBomb()) {
      plantBomb(player2, bomb2);
    }
    igniteBomb(player2, bomb2);
    event_flags.set(CLEAR_FLAG3);
    ThisThread::sleep_for(500ms);
    event_flags.set(PLAYER_FLAG2);
  }
}

int main() {
  BufferedSerial serial_port(TARGET_TX, TARGET_RX, BAUD_RATE);
  initBoard();
  thread_player1.start(callback(readPlayer1));
  thread_player2.start(callback(readPlayer2));
  thread_clearBoad.start(callback(clearExplosion));
  do {
    reset();
    showBoard();
    ThisThread::sleep_for(BLINKING_RATE);
    if (player1.getCountHit() > 0) {
      printf("Jogador 1 se deu mal! Jogo acabou.\n");
    }
    if (player2.getCountHit() > 0) {
      printf("Jogador 2 se deu mal! Jogo acabou.\n");
    }
    event_flags.set(PLAYER_FLAG1);
    event_flags.set(PLAYER_FLAG2);
  } while (!gameOver);
}
