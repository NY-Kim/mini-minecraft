#pragma once

#include <la.h>
#include <QStack>
#include <iostream>
#include <map>
#include <cmath>
#include "terrain.h"

class Terrain;
class Chunk;

class Turtle
{
public:
    glm::vec2 position;
    float orientation;

    //recursion depth
    int recDepth;

    float riverWidth;

    //constructor
    Turtle();
};

class LSystem
{
public:
    Terrain* mp_terrain;

    //save and restore turtle states as traversing the grammar string
    QStack<Turtle> turtleStack;

    //mapping char to expanded string for the grammar expansion step
    std::map<QChar, QString> charToRule;

    //mapping char to function pointer(draw operations)
    typedef Turtle (LSystem::*Rule)(Turtle);
    std::map<QChar, Rule> charToDrawingOperation;

    LSystem(Terrain* terrain);

    //generating river
    void generateRiver();
    //setting expansion grammar
    void setExpansionGrammar();
    //do all the operations in the expanded string
    void doOperations(Turtle turtle, QString instruction);

    //drawing operations
    Turtle drawLineMoveForward(Turtle turtle);
    Turtle rotateLeft(Turtle turtle);
    Turtle rotateRight(Turtle turtle);
    Turtle saveState(Turtle turtle);
    Turtle storeState(Turtle turtle);
};
