#include "lsystem.h"

LSystem::LSystem(Terrain* terrain, QString riverMode) : mp_terrain(terrain), mode(riverMode)
{}

void LSystem::generateRiver()
{
    setExpansionGrammar();

    QString axiom = QString("X");//F");
    QString expandStr = axiom;
    int iterNum = 3;

    for (int i = 0; i < iterNum; i++) {
        QString expanding = QString("");
        for (int j = 0; j < expandStr.length(); j++) {
            QChar currChar = expandStr.at(j);
            if (currChar == 'F') {
                expanding.append(charToRule['F']);
            } else if (currChar == 'X') {
                expanding.append(charToRule['X']);
            } else if (currChar == 'Z') {
                expanding.append(charToRule['Z']);
            } else {
                expanding.append(currChar);
            }
        }
        expandStr = expanding;
    }
    Turtle newTurtle = Turtle();
    //    newTurtle.position = glm::vec2(8, 6);
    //linear river
    if (mode == QString("linear")) {
        newTurtle.position = glm::vec2(100, 120);
        newTurtle.orientation = 170.f;
        newTurtle.riverWidth = 12.0f;
    } else { //delta river
        newTurtle.position = glm::vec2(-100, -120);
        newTurtle.orientation = 20.f;
        newTurtle.riverWidth = 6.0f;
    }
    runOperations(newTurtle, expandStr);
}

void LSystem::setExpansionGrammar()
{
    if (mode == QString("linear")) {
        charToRule.insert({'F', "F[-F]F[+F]"});
        charToRule.insert({'X', "F-[[X]+X]+F[+FX]-X"});
    } else {
        charToRule.insert({'F', "FF+X-Z"});
        charToRule.insert({'X', "F-FX+[F]-F"});
        charToRule.insert({'Z', "F+FF-Z-F"});
    }

    charToDrawingOperation.insert({'F', &LSystem::drawLineMoveForward});
    charToDrawingOperation.insert({'-', &LSystem::rotateLeft});
    charToDrawingOperation.insert({'+', &LSystem::rotateRight});
    charToDrawingOperation.insert({'[', &LSystem::saveState});
    charToDrawingOperation.insert({']', &LSystem::storeState});
}

void LSystem::runOperations(Turtle turtle, QString instruction)
{
    for (int i = 0; i < instruction.size(); i++) {
        QChar currChar = instruction.at(i);
        if (currChar != 'X') {
            turtle = (this->*charToDrawingOperation[currChar])(turtle);
        }
    }
}

void LSystem::carveTerrain(int x, int z)
{
    int currX = x;
    int currY = 128;
    int currZ = z;

    for (int p = 1; p <= 8; p++) {
        currX = x;
        currY = 128;
        currZ = z;
        int offsetX = 0;
        int offsetZ = 0;
//        if ((p == 1) || (p == 2) || (p == 8)) {
//            offsetZ = 1;
//        }
//        if ((p == 2) || (p == 3) || (p == 4)) {
//            offsetX = 1;
//        }
//        if ((p == 4) || (p == 5) || (p == 6)) {
//            offsetZ = -1;
//        }
//        if ((p == 6) || (p == 7) || (p == 8)) {
//            offsetX = -1;
//        }
        if (p == 1) {
            offsetZ = 1;
        } else if (p == 2) {
            offsetX = 1;
        } else if (p == 3) {
            offsetZ = -1;
        } else {
            offsetX = -1;
        }


        while((mp_terrain->getBlockAt(currX + offsetX, currY, currZ + offsetZ) != EMPTY)
              && (mp_terrain->getBlockAt(currX + offsetX, currY, currZ + offsetZ) != WATER)
              && (mp_terrain->getBlockAt(currX + offsetX, currY, currZ + offsetZ) != GRASS)) {

            mp_terrain->setBlockAt(currX + offsetX, currY + 1, currZ + offsetZ, GRASS);
            for (int i = currY + 2; i < 256; i++) {
                mp_terrain->setBlockAt(currX + offsetX, i, currZ + offsetZ, EMPTY);
            }
            currX = currX + offsetX;
            currZ = currZ + offsetZ;
            currY++;
        }
    }
}

Turtle LSystem::drawLineMoveForward(Turtle turtle)
{
    Turtle nextTurtle = turtle;
    float streamLength = 20.0f;
    turtle.orientation = turtle.orientation + (rand() % 40 + (-20));
    float angRadian = turtle.orientation * M_PI / 180.0f;
    int streamWidth = (int)glm::floor((fmax(2, (turtle.riverWidth / turtle.recDepth))/ 2.0f));

    glm::vec2 moveDir = glm::vec2(cos(angRadian) * streamLength,
                                  sin(angRadian) * streamLength);
    glm::vec2 normalizedMoveDir = glm::normalize(moveDir);
    nextTurtle.position = nextTurtle.position + moveDir;

    //set water blocks
    for (int i = 0; i < streamLength; i++) {
        int x = (int)(turtle.position[0] + (normalizedMoveDir[0] * i));
        int z = (int)(turtle.position[1] + (normalizedMoveDir[1] * i));
        for (int j = 0; j <= streamWidth; j++) {
            for (int k = 0; k <= (streamWidth - j); k++) {
                mp_terrain->setBlockAt(x + j, 128, z + k, WATER);
                mp_terrain->setBlockAt(x + j, 128, z - k, WATER);
                mp_terrain->setBlockAt(x - j, 128, z + k, WATER);
                mp_terrain->setBlockAt(x - j, 128, z - k, WATER);

                //clear the blocks above river
                for (int y = 129; y < 256; y++) {
                    mp_terrain->setBlockAt(x + j, y, z + k, EMPTY);
                    mp_terrain->setBlockAt(x + j, y, z - k, EMPTY);
                    mp_terrain->setBlockAt(x - j, y, z + k, EMPTY);
                    mp_terrain->setBlockAt(x - j, y, z - k, EMPTY);
                }
                carveTerrain(x + j, z + k);
                carveTerrain(x + j, z - k);
                carveTerrain(x - j, z + k);
                carveTerrain(x - j, z - k);
            }
        }
    }
    return nextTurtle;
}

Turtle LSystem::rotateLeft(Turtle turtle)
{
    //random angle between 15 ~ 65
    float rotAng = rand() % 50 + 15;
    turtle.orientation = turtle.orientation + rotAng;
    return turtle;
}

Turtle LSystem::rotateRight(Turtle turtle)
{
    //random angle between 15 ~ 65
    float rotAng = rand() % 50 + 15;

    turtle.orientation = turtle.orientation - rotAng;
    return turtle;
}

Turtle LSystem::saveState(Turtle turtle)
{
    turtleStack.push(turtle);
    Turtle newTurtle = turtle;
    newTurtle.recDepth++;
    newTurtle.riverWidth = newTurtle.riverWidth;
    return newTurtle;
}

Turtle LSystem::storeState(Turtle turtle)
{
    if (!turtleStack.isEmpty()) {
        return turtleStack.pop();
    }
}

Turtle::Turtle() : position(glm::vec2()), orientation(0.0f), recDepth(1)
{}
