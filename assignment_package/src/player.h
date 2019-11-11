#ifndef PLAYER_H
#define PLAYER_H

#include "la.h"
#include "camera.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <tuple>

class Player
{
public:
    // Player property member variables specified in writeup
    glm::vec3 position;
    glm::vec3 velocity;
    Camera* camera;
    std::tuple<bool, bool, bool, bool> wasdPressed;
    bool spacebarPressed;
    std::tuple<float, float> cursorXYChange;

    // Additional property member variables
    bool godMode;

    Player(Camera* c);
    virtual ~Player();

    // Function to update member variables based on key event
    void keyEventUpdate(QKeyEvent* e);

    // Function to update member variables based on mouse event
    void mouseEventUpdate(QMouseEvent* m);
};

#endif // PLAYER_H
