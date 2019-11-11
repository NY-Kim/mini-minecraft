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
    glm::vec3 position;
    glm::vec3 velocity;
    uPtr<Camera> camera;
    std::tuple<bool, bool, bool, bool> wasdPressed;
    bool spacebarPressed;
    std::tuple<float, float> cursorXYChange;

    // Additional property member variables
    bool godMode;

    Player();
    virtual ~Player();

    // Function to update member variables based on key event
    void keyEventUpdate(QKeyEvent* e, unsigned int w, unsigned int h);

    // Function to update member variables based on mouse event
    void mouseEventUpdate(QMouseEvent* m);
};

#endif // PLAYER_H
