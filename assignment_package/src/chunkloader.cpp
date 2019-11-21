#include "chunkloader.h"

ChunkLoader::ChunkLoader(int regenCase, std::vector<std::unique_ptr<Chunk>> *chunks, QString name, QMutex *mutex)
    : regenCase(regenCase), toWriteTo(chunks), name(name), mutex(mutex)
{}

void ChunkLoader::run() {

}
