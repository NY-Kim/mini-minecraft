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

    float noise2D(glm::vec2 n);
    float interpNoise2D(float x, float y);
    float fbm(float x, float y);
    float modFbm(float x, float y);
    float modFbm2(float x, float y);
    float modFbm3(float x, float y);

    //
    float overallHeight(float x, float z, glm::vec2 moistBump);
    float overallHeight2(float bilerp1, float bilerp2, float moist);
    // mustafa-canyon, grassland-snowland
    float bilerp(float biome1, float biome2, float bump);
    float canyonHeight(float x, float z);
    float grasslandHeight(float x, float z);
    float snowlandHeight(float x, float z);
    float mustafarHeight(float x, float z);
};

#endif // CHUNKLOADER_H
