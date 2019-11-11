#include "player.h"

Player::Player(Camera* c)
    : camera(c), wasdPressed(std::make_tuple(false, false, false, false)),
      cursorXYChange(std::make_tuple(0.f, 0.f))
{}

Player::~Player()
{}

void Player::keyEventUpdate(QKeyEvent *e) {

}

void Player::mouseEventUpdate(QMouseEvent *m) {

}
