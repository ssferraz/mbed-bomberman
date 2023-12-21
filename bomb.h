// bomb.h
#ifndef BOMB_H
#define BOMB_H

#include "mbed.h"

class Bomb{
private:
    int x, y;
    int explosionSize;
    bool exploded;
    bool isPlanted;
    char representation;
public:
    Bomb(int x, int y, int explosionSize, bool exploded, Timer timer,  char representation);
    Timer timer;
    int getX();
    int getY();
    bool isBombPlanted();
    int getExplosionSize();
    bool isExploded();
    char getRepresentation();

    void setX(int x);
    void setY(int y);
    void setExplosionSize(int size);
    void setBombPlated(bool isBombPlanted);
    void setExploded(bool isExploded);
    void setRepresentation(char representation);
};

#endif