// bomb.cpp
#include "bomb.h"

Bomb::Bomb(int x, int y, int explosionSize, bool exploded, Timer timer, char representation)
    : x(x), y(y), explosionSize(explosionSize), exploded(exploded), timer(timer), representation(representation) {
    }

int Bomb::getX() {
    return x;
}

int Bomb::getY() {
    return y;
}

int Bomb::getExplosionSize() {
    return explosionSize;
}

bool Bomb::isExploded() {
    return exploded;
}

bool Bomb::isBombPlanted() {
    return isPlanted;
}

void Bomb::setBombPlated(bool isBombPlanted){
    isPlanted = isBombPlanted;
}

char Bomb::getRepresentation() {
    return representation;
}

void Bomb::setX(int newX) {
    x = newX;
}

void Bomb::setY(int newY) {
    y = newY;
}

void Bomb::setExplosionSize(int newSize) {
    explosionSize = newSize;
}

void Bomb::setExploded(bool isExploded) {
    exploded = isExploded;
}

void Bomb::setRepresentation(char newRepresentation) {
    representation = newRepresentation;
}