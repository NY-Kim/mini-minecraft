#include "lsystem.h"

LSystem::LSystem(Terrain* terrain) : mp_terrain(terrain)
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
            }else {
                expanding.append(currChar);
            }
        }
        expandStr = expanding;
    }
    Turtle newTurtle = Turtle();
    newTurtle.position = glm::vec2(-100, -120);
    newTurtle.orientation = 20.f;
    //std::cout << expandStr.toStdString() << std::endl;
    doOperations(newTurtle, expandStr);
}

void LSystem::setExpansionGrammar()
{
    charToRule.insert({'F', "F[-F]F[+F][F]"});
    charToRule.insert({'X', "F-[[X]+X]+F[+FX]-X"});

    charToDrawingOperation.insert({'F', &LSystem::drawLineMoveForward});
    charToDrawingOperation.insert({'-', &LSystem::rotateLeft});
    charToDrawingOperation.insert({'+', &LSystem::rotateRight});
    charToDrawingOperation.insert({'[', &LSystem::saveState});
    charToDrawingOperation.insert({']', &LSystem::storeState});
}

void LSystem::doOperations(Turtle turtle, QString instruction)
{
    for (int i = 0; i < instruction.size(); i++) {
        QChar currChar = instruction.at(i);
        if (currChar != 'X') {
            turtle = (this->*charToDrawingOperation[currChar])(turtle);
        }

//        if (currChar == 'F') {
//            turtle = drawLineMoveForward(turtle);
//        } else if (currChar == '-') {
//            turtle = rotateLeft(turtle);
//        } else if (currChar == '+') {
//            turtle = rotateRight(turtle);
//        } else if (currChar == '[') {
//            turtle = saveState(turtle);
//        } else if (currChar == ']') {
//            turtle = storeState(turtle);
//        }
    }
}

Turtle LSystem::drawLineMoveForward(Turtle turtle)
{
    Turtle nextTurtle = turtle;
    float streamLength = 20.0f;
    turtle.orientation = turtle.orientation + (rand() % 40 + (-20));
    float angRadian = turtle.orientation * M_PI / 180.0f;
    int streamWidth = (int)glm::floor((turtle.riverWidth / 2.0f));

    glm::vec2 moveDir = glm::vec2(cos(angRadian) * streamLength,
                                  sin(angRadian) * streamLength);
    glm::vec2 normalizedMoveDir = glm::normalize(moveDir);
    nextTurtle.position = nextTurtle.position + moveDir;

    //set water blocks
    for (int i = 0; i < streamLength; i++) {
        int x = (int)(turtle.position[0] + (normalizedMoveDir[0] * i));
        int z = (int)(turtle.position[1] + (normalizedMoveDir[1] * i));
        //carve away terrain

        for (int j = -streamWidth; j < streamWidth; j++) {
            float height = mp_terrain->fbm((x / 64.0), (z / 64.0));
            height = pow(height, 3.f) * 52.0 + 128.0;
            mp_terrain->setBlockAt(x, 128, z + j, WATER);
            mp_terrain->setBlockAt(x + j, 128, z + j, WATER);
            mp_terrain->setBlockAt(x + j, 128, z, WATER);

            //clear the blocks above river
            for (int y = 129; y < 256; y++) {
                mp_terrain->setBlockAt(x, y, z + j, EMPTY);
                mp_terrain->setBlockAt(x + j, y, z + j, EMPTY);
                mp_terrain->setBlockAt(x + j, y, z, EMPTY);
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
    newTurtle.riverWidth = newTurtle.riverWidth;// /2.0f;
    return newTurtle;
}

Turtle LSystem::storeState(Turtle turtle)
{
    if (!turtleStack.isEmpty()) {
        return turtleStack.pop();
    }
}

Turtle::Turtle() : position(glm::vec2()), orientation(0.0f), recDepth(1), riverWidth(5.0f)
{}
