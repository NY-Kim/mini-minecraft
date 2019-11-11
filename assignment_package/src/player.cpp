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
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
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
    } else if (e->key() == Qt::Key_S) {
        camera->TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        camera->TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        camera->TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        camera->TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        camera->TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_R) {
        *camera = Camera(w, h);
    }
    camera->RecomputeAttributes();
}

void Player::mouseEventUpdate(QMouseEvent *m) {

}
