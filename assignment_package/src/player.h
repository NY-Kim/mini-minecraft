#ifndef PLAYER_H
#define PLAYER_H

#include "la.h"
#include "camera.h"
#include "smartpointerhelp.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <tuple>

class Player
{
public:
    // Player property member variables specified in writeup
    uPtr<Camera> camera;
    glm::vec3 position;
    glm::vec3 velocity;
    std::tuple<bool, bool, bool, bool> wasdPressed;
    bool spacebarPressed;
    QPoint cursorXYChange;
    bool lmbPressed;
    bool rmbPressed;

    // Additional property member variables 
    bool godMode;
    bool onGround;

    Player();
    virtual ~Player();

    // Function to update member variables based on key event
    void keyEventUpdate(QKeyEvent* e);

    // Function to update member variables based on mouse event
    void mouseEventUpdate(QMouseEvent* m);
};

#endif // PLAYER_H
