#include "chunkloader.h"

ChunkLoader::ChunkLoader(std::vector<uPtr<Chunk>>* chunks, QString name, QMutex* mutex)
    : toWriteTo(chunks), name(name), mutex(mutex)
{}

void ChunkLoader::run() {

}
