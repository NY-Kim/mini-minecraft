#include "lsystem.h"

LSystem::LSystem(Terrain* terrain, QString riverMode) : mp_terrain(terrain), mode(riverMode)
{}

void LSystem::generateRiver()
{
    setExpansionGrammar();

    QString axiom;

    if (mode == QString("cave")) {
        axiom = QString("C");
    } else {
        axiom = QString("X");
    }
    QString expandStr = axiom;
    int iterNum;
    if (mode == QString("linear")) {
        iterNum = 3;
    } else if (mode == QString("delta")) { //delta river
        iterNum = 3;
    } else { //cave
        iterNum = 3;
    }
    //cave

    for (int i = 0; i < iterNum; i++) {
        QString expanding = QString("");
        for (int j = 0; j < expandStr.length(); j++) {
            QChar currChar = expandStr.at(j);
            if (currChar == 'F') {
                expanding.append(charToRule['F']);
            } else if (currChar == 'X') {
                if ((rand() % 10) > 5) {
                    expanding.append(charToRule['X']);
                } else {
                    expanding.append(charToRule['Z']);
                }
            } else if (currChar == 'Z') {
                expanding.append(charToRule['Z']);
            } else {
                expanding.append(currChar);
            }
        }
        expandStr = expanding;
    }
    Turtle newTurtle = Turtle();
    //linear river
    if (mode == QString("linear")) {
        newTurtle.position = glm::vec2(10, 10);
        newTurtle.orientation = rand() % 40 + (70);
        newTurtle.riverWidth = 10.0f;
    } else if (mode == QString("delta")) { //delta river
        newTurtle.position = glm::vec2(52, 15);
        newTurtle.orientation = rand() % 40 + (-10);
        newTurtle.riverWidth = 6.0f;
    } else { //cave
        newTurtle.position = glm::vec2(30, 20);
        newTurtle.orientation = rand() % 40 + (-10);
        newTurtle.riverWidth = 6.0f;
    }
    runOperations(newTurtle, expandStr);
}

void LSystem::setExpansionGrammar()
{
    if (mode == QString("linear")) {
        charToRule.insert({'F', "F[-F]+FX-Z"});
        charToRule.insert({'X', "F[-F][Z]F[+F]X[-F]F"});
        charToRule.insert({'Z', "F[+F]F-Z+F"});
    } else if (mode == QString("delta")) {
        charToRule.insert({'F', "F[-F]F[+F]"});
        charToRule.insert({'X', "F-[[X]+FX]+F[+FX]-X"});
        charToRule.insert({'Z', "F[+F]F[[-XF]+F]"});
    } else {
        charToRule.insert({'C', "C+CV-B"});
        charToRule.insert({'V', "+C-CBV+CV-BC"});
        charToRule.insert({'B', "C+CV-VC+B]"});
    }

    charToDrawingOperation.insert({'F', &LSystem::drawLineMoveForward});
    charToDrawingOperation.insert({'-', &LSystem::rotateLeft});
    charToDrawingOperation.insert({'+', &LSystem::rotateRight});
    charToDrawingOperation.insert({'[', &LSystem::saveState});
    charToDrawingOperation.insert({']', &LSystem::storeState});
    charToDrawingOperation.insert({'C', &LSystem::caveLineMoveForward});
}

void LSystem::runOperations(Turtle turtle, QString instruction)
{
    for (int i = 0; i < instruction.size(); i++) {
        QChar currChar = instruction.at(i);
        if (currChar != 'X' && currChar != 'Z') {
            turtle = (this->*charToDrawingOperation[currChar])(turtle);
        }
    }
}

void LSystem::carveTerrain(int x, int z)
{
    int currX = x;
    int currY = 128;
    int currZ = z;

    for (int p = 1; p <= 4; p++) {
        currX = x;
        currY = 128;
        currZ = z;
        int offsetX = 0;
        int offsetZ = 0;

        if (p == 1) {
            offsetZ = 1;
        } else if (p == 2) {
            offsetX = 1;
        } else if (p == 3) {
            offsetZ = -1;
        } else {
            offsetX = -1;
        }

        //check if terrain exists to carve-out. If not, create 64x64 terrain first
        if (mp_terrain->m_chunks.find(getOrigin(currX + offsetX, currZ + offsetZ))
                == mp_terrain->m_chunks.end()) {
            extendTerrain(currX + offsetX, currZ + offsetZ);
        }

        //carve out the terrain in x,z directions near river
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

            //check if terrain exists to carve-out. If not, create 64x64 terrain first
            if (mp_terrain->m_chunks.find(getOrigin(currX + offsetX, currZ + offsetZ))
                    == mp_terrain->m_chunks.end()) {
                extendTerrain(currX + offsetX, currZ + offsetZ);
            }
        }
    }
}

void LSystem::extendTerrain(int x, int z)
{
    glm::ivec2 origin = mp_terrain->terrOrigin(glm::vec3(x, 1.0f, z));
    int originX = int(origin[0]);
    int originZ = int(origin[1]);

    //create 64x64 terrain from the given coordinate
    for(int tempX = 0; tempX < 64; ++tempX)
    {
        for(int tempZ = 0; tempZ < 64; ++tempZ)
        {
            float height = mp_terrain->fbm(((originX + tempX) / (64.0)), ((originZ + tempZ) / (64.0)));
            height = pow(height, 3.f) * 32.0 + 128.0;
            for (int y = 0; y < height; y++) {
                if (y <= 128) {
                    mp_terrain->setBlockAt(originX + tempX, y, originZ + tempZ, STONE);
                } else {
                    mp_terrain->setBlockAt(originX + tempX, y, originZ + tempZ, DIRT);
                }
            }
            int y = (int)glm::floor(height);
            mp_terrain->setBlockAt(originX + tempX, y, originZ + tempZ, GRASS);
        }
    }
}

Turtle LSystem::drawLineMoveForward(Turtle turtle)
{
    Turtle nextTurtle = turtle;
    float streamLength;
        if (mode == QString("linear")) {
            streamLength = 5.0f;
        } else {
            streamLength = 10.0f;
        }

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

        //draw river
        for (int j = 0; j <= streamWidth; j++) {
            for (int k = 0; k <= (streamWidth - j); k++) {
                if (mp_terrain->m_chunks.find(getOrigin(x + j, z + k)) == mp_terrain->m_chunks.end()) {
                    extendTerrain(x + j, z + k);
                }
                mp_terrain->setBlockAt(x + j, 128, z + k, WATER);

                if (mp_terrain->m_chunks.find(getOrigin(x + j, z - k)) == mp_terrain->m_chunks.end()) {
                    extendTerrain(x + j, z - k);
                }
                mp_terrain->setBlockAt(x + j, 128, z - k, WATER);

                if (mp_terrain->m_chunks.find(getOrigin(x - j, z + k)) == mp_terrain->m_chunks.end()) {
                    extendTerrain(x - j, z + k);
                }
                mp_terrain->setBlockAt(x - j, 128, z + k, WATER);

                if (mp_terrain->m_chunks.find(getOrigin(x - j, z - k)) == mp_terrain->m_chunks.end()) {
                    extendTerrain(x - j, z - k);
                }
                mp_terrain->setBlockAt(x - j, 128, z - k, WATER);

                //clear the blocks above river
                for (int y = 129; y < 256; y++) {
                    mp_terrain->setBlockAt(x + j, y, z + k, EMPTY);
                    mp_terrain->setBlockAt(x + j, y, z - k, EMPTY);
                    mp_terrain->setBlockAt(x - j, y, z + k, EMPTY);
                    mp_terrain->setBlockAt(x - j, y, z - k, EMPTY);
                }
                //carve-out the terrain near river
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
    float rotAng;
    //random angle between 15 ~ 35 for linear, 15 ~ 55 for delta
    if (mode == QString("linear")) {
        rotAng = rand() % 20 + 15;
    } else {
        rotAng = rand() % 40 + 15;
    }
    turtle.orientation = turtle.orientation + rotAng;
    return turtle;
}

Turtle LSystem::rotateRight(Turtle turtle)
{
    float rotAng;
    //random angle between 15 ~ 35 for linear, 15 ~ 55 for delta
    if (mode == QString("linear")) {
        rotAng = rand() % 20 + 15;
    } else {
        rotAng = rand() % 40 + 15;
    }
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

Turtle LSystem::caveLineMoveForward(Turtle turtle)
{
    Turtle nextTurtle = turtle;
    float streamLength;
        if (mode == QString("linear")) {
            streamLength = 5.0f;
        } else {
            streamLength = 10.0f;
        }

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

        //draw river
        for (int yd = 0; yd <= streamWidth; yd++) {
            for (int xd = 0; xd <= streamWidth; xd++) {
                for (int zd = 0; zd <= (streamWidth - xd); zd++) {
                    if (mp_terrain->m_chunks.find(getOrigin(x + xd, z + zd)) == mp_terrain->m_chunks.end()) {
                        extendTerrain(x + xd, z + zd);
                    }
                    mp_terrain->setBlockAt(x + xd, 128, z + zd, WATER);

                    if (mp_terrain->m_chunks.find(getOrigin(x + xd, z - zd)) == mp_terrain->m_chunks.end()) {
                        extendTerrain(x + xd, z - zd);
                    }
                    mp_terrain->setBlockAt(x + xd, 128, z - zd, WATER);

                    if (mp_terrain->m_chunks.find(getOrigin(x - xd, z + zd)) == mp_terrain->m_chunks.end()) {
                        extendTerrain(x - xd, z + zd);
                    }
                    mp_terrain->setBlockAt(x - xd, 128, z + zd, WATER);

                    if (mp_terrain->m_chunks.find(getOrigin(x - xd, z - zd)) == mp_terrain->m_chunks.end()) {
                        extendTerrain(x - xd, z - zd);
                    }
                    mp_terrain->setBlockAt(x - xd, 128, z - zd, WATER);

                    //clear the blocks above river
                    for (int y = 129; y < 256; y++) {
                        mp_terrain->setBlockAt(x + xd, y, z + zd, EMPTY);
                        mp_terrain->setBlockAt(x + xd, y, z - zd, EMPTY);
                        mp_terrain->setBlockAt(x - xd, y, z + zd, EMPTY);
                        mp_terrain->setBlockAt(x - xd, y, z - zd, EMPTY);
                    }
                    //carve-out the terrain near river
                    carveTerrain(x + xd, z + zd);
                    carveTerrain(x + xd, z - zd);
                    carveTerrain(x - xd, z + zd);
                    carveTerrain(x - xd, z - zd);
                }
            }
        }
    }
    return nextTurtle;
}

Turtle::Turtle() : position(glm::vec2()), orientation(0.0f), recDepth(1)
{}
