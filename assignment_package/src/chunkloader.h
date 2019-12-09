#ifndef CHUNKLOADER_H
#define CHUNKLOADER_H

#include <QRunnable>
#include <QString>
#include <QMutex>
#include <memory>

#include "scene/terrain.h"

#define uPtr std::unique_ptr

class ChunkLoader : public QRunnable
{
private:
    int regenCase;
    std::vector<Chunk*> chunks;
    std::vector<Chunk*>* toWriteTo;
    QString name;
    QMutex* mutex;
    QString biomeType;

public:
    ChunkLoader(int regenCase, std::vector<Chunk*> toModify, std::vector<Chunk*>* chunks, QString name, QMutex* mutex);
    void run() override;

    //mb deciding noise function
    float mb_noise2D(glm::vec2 n);
    float mb_interpNoise2D(float x, float y);
    float mb_fbm(float x, float y);

    float noise2D(glm::vec2 n);
    float interpNoise2D(float x, float y);
    float fbm(float x, float y);
    float modGrass(float x, float y);
    float modMustafar(float x, float y);
    float modSnow(float x, float y);
    float modCanyon(float x, float y);

    //getting height for biomes
    float overallHeight(float x, float z, glm::vec2 moistBump);
    // mustafa-canyon, grassland-snowland
    float bilerp(float biome1, float biome2, float bump);
    float canyonHeight(float x, float z);
    float grasslandHeight(float x, float z);
    float snowlandHeight(float x, float z);
    float mustafarHeight(float x, float z);

    //setting blocks for biome
    void setCanyon(float x, float z, float height, Chunk* chunk);
    void setGrassland(float x, float z, float height, Chunk* chunk);
    void setSnowland(float x, float z, float height, Chunk* chunk);
    void setMustafar(float x, float z, float height, Chunk* chunk);
};

#endif // CHUNKLOADER_H
