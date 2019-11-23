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

public:
    ChunkLoader(int regenCase, std::vector<Chunk*> toModify, std::vector<Chunk*>* chunks, QString name, QMutex* mutex);
    void run() override;

    float noise2D(glm::vec2 n);
    float interpNoise2D(float x, float y);
    float fbm(float x, float y);
};

#endif // CHUNKLOADER_H
