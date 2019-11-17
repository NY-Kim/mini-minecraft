#include "player.h"
#include <iostream>

Player::Player()
    : camera(mkU<Camera>()), velocity(0.f, 0.f, 0.f),
      wasdPressed(std::make_tuple(false, false, false, false)), spacebarPressed(false),
      mouseMoved(false), lmbPressed(false), rmbPressed(false),
      godMode(true), onGround(true),
      qPressed(false), ePressed(false), fPressed(false)
{}

Player::~Player()
{}

void Player::keyEventUpdate(QKeyEvent *e) {
    // Modified from base code
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }

    // Check if key was pressed or released, then act accordingly
    if (e->type() == QEvent::KeyPress) {
        if (e->key() == Qt::Key_1) {
            camera->fovy += amount;
        } else if (e->key() == Qt::Key_2) {
            camera->fovy -= amount;
        } else if (e->key() == Qt::Key_W) {
            std::get<0>(wasdPressed) = true;
        } else if (e->key() == Qt::Key_A) {
            std::get<1>(wasdPressed) = true;
        } else if (e->key() == Qt::Key_S) {
            std::get<2>(wasdPressed) = true;
        } else if (e->key() == Qt::Key_D) {
            std::get<3>(wasdPressed) = true;
        } else if (e->key() == Qt::Key_Q && godMode) {
            qPressed = true;
        } else if (e->key() == Qt::Key_E && godMode) {
            ePressed = true;
        } else if (e->key() == Qt::Key_R) {
            *camera = Camera(camera->width, camera->height);
        } else if (e->key() == Qt::Key_F) {
            fPressed = true;;
        } else if (e->key() == Qt::Key_Space) {
            spacebarPressed = true;
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
        } else if (e->key() == Qt::Key_Q) {
            qPressed = false;
        } else if (e->key() == Qt::Key_E) {
            ePressed = false;
        }
    }
}

void Player::mouseEventUpdate(QMouseEvent *m) {
    if (m->type() == QEvent::MouseMove) {
        QPoint cursorXYChange = QPoint(camera->width / 2, camera->height / 2) - m->pos();
        camera->polar[0] += cursorXYChange.x() / 10.f;
        camera->polar[1] += cursorXYChange.y() / 10.f;
        mouseMoved = true;
    }
}
