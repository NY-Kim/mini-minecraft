#include "lsystem.h"

LSystem::LSystem(Terrain* terrain) : mp_terrain(terrain)
{}

void LSystem::generateRiver()
{
    setExpansionGrammar();

    QString axiom = QString("F");
    QString expandStr = axiom;
    int iterNum = 3;

    for (int i = 0; i < iterNum; i++) {
        QString expanding = QString("");
        for (int j = 0; j < expandStr.length(); j++) {
            QChar currChar = expandStr.at(j);
            if (currChar == 'F') {
                expanding.append(charToRule['F']);
            } else {
                expanding.append(currChar);
            }
        }
        expandStr = expanding;
    }
    Turtle newTurtle = Turtle();
    newTurtle.position = glm::vec2(10, 10);
    std::cout << expandStr.toStdString() << std::endl;
    doOperations(newTurtle, expandStr);
}

void LSystem::setExpansionGrammar()
{
    charToRule.insert({'F', "F[-F]F[+F][F]"});


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
        turtle = (this->*charToDrawingOperation[currChar])(turtle);

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
//    std::cout<< "drawLine called!!!!" << std::endl;
    Turtle nextTurtle = turtle;
    float streamLength = 15.0f;
    float angRadian = turtle.orientation * M_PI / 180.0f;
    glm::vec2 moveDir = glm::vec2(cos(angRadian) * streamLength,
                                  sin(angRadian) * streamLength);
    glm::vec2 normalizedMoveDir = glm::normalize(moveDir);
    nextTurtle.position = nextTurtle.position + moveDir;

    //set water blocks
    for (int i = 0; i < streamLength; i++) {
        int x = (int)(turtle.position[0] + (normalizedMoveDir[0] * i));
        int z = (int)(turtle.position[1] + (normalizedMoveDir[1] * i));
        float height = mp_terrain->fbm((x / 64.0), (z / 64.0));
        height = pow(height, 3.f) * 52.0 + 128.0;
//        std::cout << int(turtle.position[0] + (normalizedMoveDir[0] * i)) << ", " << int(turtle.position[1] + (normalizedMoveDir[1] * i)) << std::endl;
        mp_terrain->setBlockAt(x, 128, z, WATER);
        for (int y = 129; y < 256; y++) {
            mp_terrain->setBlockAt(x, y, z, EMPTY);
        }
    }
    return nextTurtle;
}

Turtle LSystem::rotateLeft(Turtle turtle)
{
    //random angle between 15 ~ 85
    float rotAng = rand() % 70 + 15;
    turtle.orientation = turtle.orientation + rotAng;
    return turtle;
}

Turtle LSystem::rotateRight(Turtle turtle)
{
    //random angle between 15 ~ 35
    float rotAng = rand() % 70 + 15;
    turtle.orientation = turtle.orientation - rotAng;
    return turtle;
}

Turtle LSystem::saveState(Turtle turtle)
{
    turtleStack.push(turtle);
    Turtle newTurtle = turtle;
    newTurtle.recDepth++;
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
