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
    std::vector<uPtr<Chunk>>* toWriteTo;
    QString name;
    QMutex* mutex;

public:
    ChunkLoader(int regenCase, std::vector<uPtr<Chunk>>* chunks, QString name, QMutex* mutex);
    void run() override;
};

#endif // CHUNKLOADER_H
