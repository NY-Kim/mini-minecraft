#include "chunkloader.h"
#include <iostream>

ChunkLoader::ChunkLoader(int regenCase, std::vector<Chunk*> toModify, std::vector<Chunk*> *chunks, QString name, QMutex *mutex)
    : regenCase(regenCase), chunks(toModify), toWriteTo(chunks), name(name), mutex(mutex)
{}

void ChunkLoader::run() {
    for (Chunk* chunk : chunks) {
    // Step 1. Build FBM blocks
        int originX = chunk->position[0];
        int originZ = chunk->position[1];
        //========================
        for(int x = 0; x < 16; ++x) {
            for(int z = 0; z < 16; ++z) {
                glm::vec2 mb = glm::vec2(mb_fbm((originX + x) / 500.f, (originZ + z) / 500.f),
                                         mb_fbm((originX + x + 100.34) / 500.f, (originZ + z + 678.98234) / 500.f));
                //assigning biomeType
                if (mb[0] < 0.5) {
                    if (mb[1] < 0.5) {
                        biomeType = QString("mustafar");
                    } else {
                        biomeType = QString("canyon");
                    }
                } else {
                    if (mb[1] < 0.5) {
                        biomeType = QString("grassland");
                    } else {
                        biomeType = QString("snowland");
                    }
                }

                float height = overallHeight((originX + x), (originZ + z), mb);

                if (biomeType == QString("mustafar")) {
                    setMustafar(x, z, height, chunk);
                } else if (biomeType == QString("canyon")) {
                    setCanyon(x, z, height, chunk);
                } else if (biomeType == QString("grassland")) {
                    setGrassland(x, z, height, chunk);
                } else {
                    setSnowland(x, z, height, chunk);
                }
            }
        }

        // Step 2. Build VBO vectors without passing to GPU
        chunk->createVBOs();
    }

    // Step 3. Lock mutex, push chunk onto MyGL vector, then unlock
    mutex->lock();
    for (Chunk* chunk : chunks) {
        toWriteTo->push_back(chunk);
    }
    mutex->unlock();
}

//mb_noise basis function
float ChunkLoader::mb_noise2D(glm::vec2 n)
{
    return (glm::fract(sin(glm::dot(n, glm::vec2(311.7, 191.999))) * 17434.2371));
}

//mb_interpNoise2D
float ChunkLoader::mb_interpNoise2D(float x, float y)
{
    float intX = floor(x);
    float fractX = glm::fract(x);
    float intY = floor(y);
    float fractY = glm::fract(y);

    float v1 = mb_noise2D(glm::vec2(intX, intY));
    float v2 = mb_noise2D(glm::vec2(intX + 1, intY));
    float v3 = mb_noise2D(glm::vec2(intX, intY + 1));
    float v4 = mb_noise2D(glm::vec2(intX + 1, intY + 1));

    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);
    return glm::mix(i1, i2, fractY);
}

//mb_fbm function
float ChunkLoader::mb_fbm(float x, float y)
{
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;

    for(int i = 1; i <= octaves; i++) {
        float freq = pow(2.f, i);
        float amp = pow(persistence, i);

        total += mb_interpNoise2D(x * freq, y * freq) * amp;
    }
    return total;
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

float ChunkLoader::modGrass(float x, float y)
{
    float height = fbm(x, y);
    height = pow(height, 3.f) * 16.0 + 128.0;
    return height;
}

float ChunkLoader::modMustafar(float x, float y)
{
    float height = fbm(x, y);
    height = glm::smoothstep(0.55f, 0.75f, height) * 22 + 128.0 + glm::smoothstep(0.62f, 0.8f, height) * 20;
    return height;
}

float ChunkLoader::modSnow(float x, float y)
{
    float height = fbm(x, y);
    height = pow(height, 2.f) * 72.0 + 148.0 + glm::smoothstep(0.45f, 0.6f, height) * 15;
    return height;
}

float ChunkLoader::modCanyon(float x, float y)
{
    float height = fbm(x, y);
    height = glm::smoothstep(0.55f, 0.7f, height) * 20 + 128.0 + glm::smoothstep(0.2f, 0.75f, height) * 3 + glm::smoothstep(0.55f, 0.65f, height) * 13;
    return height;
}

//BIOME=======================================================================
//getting the overall height
float ChunkLoader::overallHeight(float x, float z, glm::vec2 moistBump)
{
    float moist = moistBump[0];
    float bump = moistBump[1];
    float canyonY = canyonHeight(x / 64.0f, z / 64.0f);
    float grassY = grasslandHeight(x / 64.0f, z / 64.0f);
    float snowY = snowlandHeight(x / 64.0f, z / 64.0f);
    float mustafarY = mustafarHeight(x / 64.0f, z / 64.0f);

    float bilerp1 = bilerp(mustafarY, canyonY, bump);
    float bilerp2 = bilerp(grassY, snowY, bump);

    return bilerp(bilerp1, bilerp2, moist);
}

// mustafa-canyon, grassland-snowland
float ChunkLoader::bilerp(float biome1, float biome2, float bump)
{
    //return glm::mix(biome1, biome2, bump);
    return glm::mix(biome1, biome2, glm::smoothstep(0.45f, 0.6f, bump));
}

float ChunkLoader::canyonHeight(float x, float z)
{
    return modCanyon(x, z);
}

float ChunkLoader::grasslandHeight(float x, float z)
{
    return modGrass(x, z);
}

float ChunkLoader::snowlandHeight(float x, float z)
{
    return modSnow(x, z);
}

float ChunkLoader::mustafarHeight(float x, float z)
{
    return modMustafar(x, z);
}

void ChunkLoader::setCanyon(float x, float z, float height, Chunk *chunk)
{
    for (int y = 1; y < height; y++) {
        if (y < 127) {
            chunk->getBlockAt(x, y, z) = STONE;
        } else if (y < 138) {
            chunk->getBlockAt(x, y, z) = SAND;
        } else if (y < 142) {
            chunk->getBlockAt(x, y, z) = ORANGE;
        } else if (y < 145) {
            chunk->getBlockAt(x, y, z) = IVORY;
        } else if (y < 147) {
            chunk->getBlockAt(x, y, z) = BROWN;
        } else if (y < 152) {
            chunk->getBlockAt(x, y, z) = IVORY;
        } else {
            chunk->getBlockAt(x, y, z) = ORANGE;
        }
    }
}

void ChunkLoader::setGrassland(float x, float z, float height, Chunk* chunk)
{
    for (int y = 1; y < height; y++) {
        if (y <= 128) {
            chunk->getBlockAt(x, y, z) = STONE;
        } else {
            chunk->getBlockAt(x, y, z) = DIRT;
        }
    }
    int y = (int)glm::floor(height);
    chunk->getBlockAt(x, y, z) = GRASS;
}

void ChunkLoader::setSnowland(float x, float z, float height, Chunk* chunk)
{
    for (int y = 1; y < height; y++) {
        if (y <= 128) {
            chunk->getBlockAt(x, y, z) = STONE;
        } else {
            chunk->getBlockAt(x, y, z) = DIRT;
        }
    }
    int y = (int)glm::floor(height);
    chunk->getBlockAt(x, y, z) = SNOW;
}

void ChunkLoader::setMustafar(float x, float z, float height, Chunk* chunk)
{
    for (int y = 1; y < height; y++) {
        if (y < 127) {
            chunk->getBlockAt(x, y, z) = STONE;
        } else if (y < 128){
            chunk->getBlockAt(x, y, z) = LAVA;
        } else {
            chunk->getBlockAt(x, y, z) = DARK;
        }
    }
}
