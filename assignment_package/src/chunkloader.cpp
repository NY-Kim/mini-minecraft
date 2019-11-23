#include "chunkloader.h"
#include <iostream>

ChunkLoader::ChunkLoader(int regenCase, uPtr<Chunk> toModify, std::vector<uPtr<Chunk>> *chunks, QString name, QMutex *mutex)
    : regenCase(regenCase), chunk(std::move(toModify)), toWriteTo(chunks), name(name), mutex(mutex)
{}

void ChunkLoader::run() {
    // Step 1. Build FBM blocks
    std::cout << name.toStdString() << " is creating FBM data." << std::endl;
    int originX = chunk->position[0];
    int originZ = chunk->position[1];

    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            float height = fbm(((originX + x) / (64.0)), ((originZ + z) / (64.0)));
            height = pow(height, 3.f) * 52.0 + 128.0;
            glm::ivec2 chunk_xz = getChunkCoordinates(originX + x, originZ + z);
            for (int y = 127; y < height; y++) {
                if (y <= 128) {
                    chunk->getBlockAt(chunk_xz[0], y, chunk_xz[1]) = STONE;
                } else {
                    chunk->getBlockAt(chunk_xz[0], y, chunk_xz[1]) = DIRT;
                }
            }
            int y = (int)glm::floor(height);
            chunk->getBlockAt(chunk_xz[0], y, chunk_xz[1]) = GRASS;
        }
    }

    // Step 2. Build VBO vectors without passing to GPU
    std::cout << name.toStdString() << " is creating VBOs." << std::endl;
    chunk->createVBOs();

    // Step 3. Lock mutex, push chunk onto MyGL vector, then unlock
    std::cout << name.toStdString() << " is attempting to lock mutex." << std::endl;
    mutex->lock();
    std::cout << name.toStdString() << " has locked the mutex." << std::endl;
    std::cout << name.toStdString() << " is pushing back to vector." << std::endl;
    toWriteTo->push_back(std::move(chunk));
    mutex->unlock();
    std::cout << name.toStdString() << " is finished." << std::endl;
}

float ChunkLoader::noise2D(glm::vec2 n)
{
    return (glm::fract(sin(glm::dot(n, glm::vec2(12.9898, 4.1414))) * 43758.5453));
}

float ChunkLoader::interpNoise2D(float x, float y)
{
    float intX = floor(x);
    float fractX = glm::fract(x);
    float intY = floor(y);
    float fractY = glm::fract(y);

    float v1 = noise2D(glm::vec2(intX, intY));
    float v2 = noise2D(glm::vec2(intX + 1, intY));
    float v3 = noise2D(glm::vec2(intX, intY + 1));
    float v4 = noise2D(glm::vec2(intX + 1, intY + 1));

    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);
    return glm::mix(i1, i2, fractY);
}

float ChunkLoader::fbm(float x, float y) {
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;

    for(int i = 1; i <= octaves; i++) {
        float freq = pow(2.f, i);
        float amp = pow(persistence, i);

        total += interpNoise2D(x * freq, y * freq) * amp;
    }
    return total;
}
