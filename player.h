
// player.h
#ifndef PLAYER_H
#define PLAYER_H

#include "mbed.h"
#include <string>

class Player {
private: 
    int x, y;
    char representation;
    bool hasBomb;
    int countHit;

public:
    Player(int x, int y, const char representation, bool hasBomb, int countHit);
    int getX();
    int getY();
    int getCountHit();
    void setCountHit();
    bool getHasBomb();
    void setHasBomb(bool bomb);
    char getRepresentation();
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
};

#endif