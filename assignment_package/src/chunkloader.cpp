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
                glm::vec2 currP = glm::vec2(x, z);
                glm::vec2 mb = glm::vec2(fbm((originX + x) / 160.0f, (originZ + z) / 160.0f),
                                         fbm((originX + x + 100.34) / 160.f, (originZ + z + 678.98234) / 160.0f));
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

                for (int y = 1; y < height; y++) {
                    if (y <= 128) {
                        chunk->getBlockAt(x, y, z) = STONE;
                    } else {
                        chunk->getBlockAt(x, y, z) = DIRT;
                    }
                }
                int y = (int)glm::floor(height);
                if (biomeType == QString("mustafar")) {
                    chunk->getBlockAt(x, y, z) = LAVA;
                } else if (biomeType == QString("canyon")) {
                    chunk->getBlockAt(x, y, z) = IRON;
                } else if (biomeType == QString("grassland")) {
                    chunk->getBlockAt(x, y, z) = GRASS;
                } else {
                    chunk->getBlockAt(x, y, z) = SNOW;
                }
            }
        }
        //==============================
//        for(int x = 0; x < 16; ++x) {
//            for(int z = 0; z < 16; ++z) {
//                float height = modFbm2(((originX + x) / (64.0)), ((originZ + z) / (64.0)));

//                for (int y = 1; y < height; y++) {
//                    if (y <= 128) {
//                        chunk->getBlockAt(x, y, z) = STONE;
//                    } else {
//                        chunk->getBlockAt(x, y, z) = DIRT;
//                    }
//                }
//                int y = (int)glm::floor(height);
//                chunk->getBlockAt(x, y, z) = GRASS;
//            }
//        }

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

float ChunkLoader::noise2D(glm::vec2 n)
{
    //return (glm::fract(sin(glm::dot(n, glm::vec2(12.9898, 4.1414))) * 43758.5453));
    return (glm::fract(sin(glm::dot(n, glm::vec2(34.4938, 5.3244))) * 87297.8374));

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

float ChunkLoader::modFbm(float x, float y)
{
    float height = fbm(x, y);
    height = pow(height, 3.f) * 32.0 + 128.0;
    //height = glm::smoothstep(0.5f, 0.7f, height) * 32 + glm::smoothstep(0.3f, 0.46f, height) * 21 + 128.0;  //canyon-style
    return height;
}

//for canyon
float ChunkLoader::modFbm2(float x, float y)
{
    float height = fbm(x, y);
    //height = pow(height, 3.f) * 32.0 + 128.0;  //original
    //height = glm::smoothstep(0.5f, 0.7f, height) * 42 + 128.0;  //canyon-style
    //height = glm::smoothstep(0.5f, 0.7f, height) * 32 + glm::smoothstep(0.3f, 0.46f, height) * 21 + 128.0;  //canyon-style
    height = glm::smoothstep(0.4f, 0.64f, height) * 32 + 128.0 + glm::smoothstep(0.2f, 0.75f, height) * 3;  //canyon-style

    return height;
}

//for canyon
float ChunkLoader::modFbm3(float x, float y)
{
    float height = fbm(x, y);
    //height = pow(height, 3.f) * 32.0 + 128.0;  //original
    //height = glm::smoothstep(0.5f, 0.7f, height) * 42 + 128.0;  //canyon-style
    //height = glm::smoothstep(0.7f, 0.9f, height) * 22 + glm::smoothstep(0.3f, 0.46f, height) * 46 + 128.0;  //canyon-style
    height = pow(height, 2.f) * 52.0 + 128.0;  //original
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

    //return glm::mix(bilerp1, bilerp2, moist);
    return bilerp(bilerp1, bilerp2, moist);
}

float ChunkLoader::overallHeight2(float bilerp1, float bilerp2, float moist)
{
    return glm::mix(bilerp1, bilerp2, moist);
}

// mustafa-canyon, grassland-snowland
float ChunkLoader::bilerp(float biome1, float biome2, float bump)
{
    //return glm::mix(biome1, biome2, bump);
    return glm::mix(biome1, biome2, glm::smoothstep(0.45f, 0.6f, bump));
}

float ChunkLoader::canyonHeight(float x, float z)
{
    return 130;
}

float ChunkLoader::grasslandHeight(float x, float z)
{
    return modFbm(x, z);
}

float ChunkLoader::snowlandHeight(float x, float z)
{
    return modFbm3(x, z);
}

float ChunkLoader::mustafarHeight(float x, float z)
{
    return modFbm2(x, z);
}
