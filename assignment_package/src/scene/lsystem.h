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

    float yVal();

    //constructor
    Turtle();
};

class LSystem
{
public:
    Terrain* mp_terrain;

    QString mode;

    //save and restore turtle states as traversing the grammar string
    QStack<Turtle> turtleStack;

    //mapping char to expanded string for the grammar expansion step
    std::map<QChar, QString> charToRule;

    //mapping char to function pointer(draw operations)
    typedef Turtle (LSystem::*Rule)(Turtle);
    std::map<QChar, Rule> charToDrawingOperation;

    LSystem(Terrain* terrain, QString riverMode);

    QString biomeType;

    //generating river
    void generateRiver();
    //setting expansion grammar
    void setExpansionGrammar();
    //run all the operations in the expanded string
    void runOperations(Turtle turtle, QString instruction);
    //helper function for carving out terrain around the river
    void carveTerrain(int x, int z);
    //helper function for generating terrain when creating river and carving-out
    void extendTerrain(int x, int z);

    //drawing operations
    Turtle drawLineMoveForward(Turtle turtle);
    Turtle rotateLeft(Turtle turtle);
    Turtle rotateRight(Turtle turtle);
    Turtle saveState(Turtle turtle);
    Turtle storeState(Turtle turtle);
    Turtle caveLineMoveForward(Turtle turtle);

    //mb deciding noise function
    float mb_noise2D(glm::vec2 n);
    float mb_interpNoise2D(float x, float y);
    float mb_fbm(float x, float y);

    //fbm functions
    float noise2D(glm::vec2 n);
    float interpNoise2D(float x, float y);
    float fbm(float x, float y);
    float modGrass(float x, float y);
    float modMustafar(float x, float y);
    float modSnow(float x, float y);
    float modCanyon(float x, float y);

    //getting the overall height
    float overallHeight(float x, float z, glm::vec2 moistBump);
    float bilerp(float biome1, float biome2, float bump);
    float canyonHeight(float x, float z);
    float grasslandHeight(float x, float z);
    float snowlandHeight(float x, float z);
    float mustafarHeight(float x, float z);

    //setting blocks for biome
    void setCanyon(float x, float z, float height);
    void setGrassland(float x, float z, float height);
    void setSnowland(float x, float z, float height);
    void setMustafar(float x, float z, float height);
};
