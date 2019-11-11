#include "player.h"

Player::Player()
    : camera(mkU<Camera>()), wasdPressed(std::make_tuple(false, false, false, false)),
      cursorXYChange(std::make_tuple(0.f, 0.f))
{}

Player::~Player()
{}

void Player::keyEventUpdate(QKeyEvent *e, unsigned int w, unsigned int h) {
    // Modified from base code
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }

    // Check if key was pressed or released, then act accordingly
    if (e->type() == QEvent::KeyPress) {
        if (e->key() == Qt::Key_Right) {
            camera->RotateAboutUp(-amount);
        } else if (e->key() == Qt::Key_Left) {
            camera->RotateAboutUp(amount);
        } else if (e->key() == Qt::Key_Up) {
            camera->RotateAboutRight(-amount);
        } else if (e->key() == Qt::Key_Down) {
            camera->RotateAboutRight(amount);
        } else if (e->key() == Qt::Key_1) {
            camera->fovy += amount;
        } else if (e->key() == Qt::Key_2) {
            camera->fovy -= amount;
        } else if (e->key() == Qt::Key_W) {
            camera->TranslateAlongLook(amount);
            std::get<0>(wasdPressed) = true;
        } else if (e->key() == Qt::Key_A) {
            camera->TranslateAlongRight(-amount);
            std::get<1>(wasdPressed) = true;
        } else if (e->key() == Qt::Key_S) {
            camera->TranslateAlongLook(-amount);
            std::get<2>(wasdPressed) = true;
        } else if (e->key() == Qt::Key_D) {
            camera->TranslateAlongRight(amount);
            std::get<3>(wasdPressed) = true;
        } else if (e->key() == Qt::Key_Q) {
            camera->TranslateAlongUp(-amount);
        } else if (e->key() == Qt::Key_E) {
            camera->TranslateAlongUp(amount);
        } else if (e->key() == Qt::Key_R) {
            *camera = Camera(w, h);
        }
        camera->RecomputeAttributes();
    }
    else {
        if (e->key() == Qt::Key_W) {
            std::get<0>(wasdPressed) = false;
        } else if (e->key() == Qt::Key_A) {
            std::get<1>(wasdPressed) = false;
        } else if (e->key() == Qt::Key_S) {
            std::get<2>(wasdPressed) = false;
        } else if (e->key() == Qt::Key_D) {
            std::get<3>(wasdPressed) = false;
        }
    }
}

void Player::mouseEventUpdate(QMouseEvent *m) {

}
