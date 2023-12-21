// player.cpp
#include "player.h"

Player::Player(int x, int y, const char representation, bool hasBomb, int countHit)
    : x(x), y(y), representation(representation), hasBomb(hasBomb), countHit(countHit) {}

int Player::getX() {
    return x;
}

int Player::getY() {
    return y;
}

int Player::getCountHit() {
    return countHit;
}

void Player::setCountHit() {
    ++countHit;
}

bool Player::getHasBomb(){
    return hasBomb;
}

void Player::setHasBomb(bool bomb){
    hasBomb = bomb;
}

char Player::getRepresentation() {
    return representation;
}

void Player::moveUp() {
    --x;
}
void Player::moveDown() {
    ++x;
}
void Player::moveLeft() {
    --y;
}
void Player::moveRight() {
    ++y;
}
