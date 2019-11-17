#include <scene/terrain.h>

#include <scene/cube.h>
#include <array>
#include <iostream>

  Terrain::Terrain(OpenGLContext* context)
      : context(context), m_chunks(std::map<std::pair<int, int>, Chunk>()), dimensions(64, 256, 64)
  {}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
  std::pair<int, int> key = getOrigin(x, z);
  glm::ivec2 chunk_xz = getChunkCoordinates(x, z);

  if (m_chunks.find(key) != m_chunks.end()) {
      return m_chunks.at(key).getBlockAt(chunk_xz[0], y, chunk_xz[1]);
  }
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    std::pair<int, int> key = getOrigin(x, z);
    glm::ivec2 chunk_xz = getChunkCoordinates(x, z);

    if (m_chunks.find(key) != m_chunks.end()) {
        m_chunks[key].getBlockAt(chunk_xz[0], y, chunk_xz[1]) = t;
    } else {
        glm::ivec2 origin = glm::ivec2(key.first, key.second);
        Chunk chunk = Chunk(context, origin);
        chunk.getBlockAt(chunk_xz[0], y, chunk_xz[1]) = t;
        m_chunks[key] = chunk;

        if (m_chunks.find(std::pair<int, int>(key.first - 16, key.second)) != m_chunks.end()) {
            Chunk* negX = &m_chunks[std::pair<int, int>(key.first - 16, key.second)];
            chunk.negX_chunk = negX;
            negX->posX_chunk = &m_chunks[key];
        }

        if (m_chunks.find(std::pair<int, int>(key.first + 16, key.second)) != m_chunks.end()) {
            Chunk* posX = &m_chunks[std::pair<int, int>(key.first + 16, key.second)];
            chunk.posX_chunk = posX;
            posX->negX_chunk = &m_chunks[key];
        }

        if (m_chunks.find(std::pair<int, int>(key.first, key.second - 16)) != m_chunks.end()) {
            Chunk* negZ = &m_chunks[std::pair<int, int>(key.first, key.second - 16)];
            chunk.negZ_chunk = negZ;
            negZ->posZ_chunk = &m_chunks[key];
        }

        if (m_chunks.find(std::pair<int, int>(key.first, key.second + 16)) != m_chunks.end()) {
            Chunk* posZ = &m_chunks[std::pair<int, int>(key.first, key.second + 16)];
            chunk.posZ_chunk = posZ;
            posZ->negZ_chunk = &m_chunks[key];
        }
    }
}


void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            float height = fbm((x / 64.0), (z / 64.0));
            height = pow(height, 3.f) * 52.0 + 128.0;
            for (int y = 127; y < height; y++) {
                if (y <= 128) {
                    setBlockAt(x, y, z, STONE);
                    //m_blocks[x][y][z] = STONE;
                } else {
                    setBlockAt(x, y, z, DIRT);
                    //m_blocks[x][y][z] = DIRT;
                }
            }
            int y = (int)glm::floor(height);
            setBlockAt(x, y, z, GRASS);
        }
    }
}

void Terrain::create() {
    for (std::map<std::pair<int, int>, Chunk>::iterator i = m_chunks.begin(); i != m_chunks.end(); i++) {
        i->second.Chunk::create();
    }
}

//fbm functions=============================================================
//noise basis function
float Terrain::noise2D(glm::vec2 n)
{
    return (glm::fract(sin(glm::dot(n, glm::vec2(12.9898, 4.1414))) * 43758.5453));
}

//interpNoise2D
float Terrain::interpNoise2D(float x, float y)
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

//fbm function
float Terrain::fbm(float x, float y)
{
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

//add/delete block function===================================================
//add
void Terrain::addBlock(glm::vec3 eye, glm::vec3 look)
{
    glm::vec3 blockCoord = rayMarch(eye, look);
    blockCoord = glm::floor(blockCoord - (look * 0.75f));
    setBlockAt((int)blockCoord[0], (int)blockCoord[1], (int)blockCoord[2], LAVA);
}

//delete
void Terrain::deleteBlock(glm::vec3 eye, glm::vec3 look)
{
    glm::vec3 blockCoord = rayMarch(eye, look);
    blockCoord = glm::floor(blockCoord);
    setBlockAt((int)blockCoord[0], (int)blockCoord[1], (int)blockCoord[2], EMPTY);
}

//helper function for ray marching
glm::vec3 Terrain::rayMarch(glm::vec3 eye, glm::vec3 look)
{
    float step = 0.75;
    bool hit = false;
    glm::vec3 currPos = eye;
    glm::vec3 currBlockCoord;
    while (!hit) {
        currPos = currPos + (look * step);
        currBlockCoord = glm::floor(currPos);
        if (getBlockAt((int)currBlockCoord[0],
                       (int)currBlockCoord[1],
                       (int)currBlockCoord[2]) == EMPTY) {
            continue;
        } else {
            hit = true;
        }
    }
    return currPos;
}

////helper function to decide regenerate terrain and which direction
////N:1 > NE:2 > E:3 > SE:4 > S:5 > SW:6 > W:7 > NW:8
//int Terrain::checkRegenerate(glm::vec3 eye)
//{
//    //std::pair<int, int> key = getOrigin(int(eye.x) + 32, int(eye.z));
//    if (m_chunks.find(getOrigin(int(eye.x), int(eye.z) + 32)) == m_chunks.end()) {
//        return 1;
//    } else if (m_chunks.find(getOrigin(int(eye.x) + 32, int(eye.z) + 32)) == m_chunks.end()) {
//        return 2;
//    } else if (m_chunks.find(getOrigin(int(eye.x) + 32, int(eye.z))) == m_chunks.end()) {
//        return 3;
//    } else if (m_chunks.find(getOrigin(int(eye.x) + 32, int(eye.z) - 32)) == m_chunks.end()) {
//        return 4;
//    } else if (m_chunks.find(getOrigin(int(eye.x), int(eye.z) - 32)) == m_chunks.end()) {
//        return 5;
//    } else if (m_chunks.find(getOrigin(int(eye.x) - 32, int(eye.z) - 32)) == m_chunks.end()) {
//        return 6;
//    } else if (m_chunks.find(getOrigin(int(eye.x) - 32, int(eye.z))) == m_chunks.end()) {
//        return 7;
//    } else if (m_chunks.find(getOrigin(int(eye.x) - 32, int(eye.z) + 32)) == m_chunks.end()) {
//        return 8;
//    } else {
//        return 0;
//    }
//}

////regenerating terrain
//void Terrain::regenerateTerrain(int regenCase)
//{
//    if (regenCase == 1) {

//    } else if (regenCase == 2) {

//    }
//}

Chunk::Chunk() : Drawable(nullptr) {}

Chunk::Chunk(OpenGLContext *context, glm::ivec2 origin)
    : Drawable(context), position(glm::ivec2(origin[0], origin[1])),
      negX_chunk(nullptr), posX_chunk(nullptr), negZ_chunk(nullptr), posZ_chunk(nullptr)
{
    m_blocks.fill(EMPTY);
}

Chunk::~Chunk() {}

void Chunk::create() {
    std::vector<GLuint> idx;
    std::vector<glm::vec4> pnc; // vector that stores position, normal, and color

    std::map<BlockType, glm::vec4> color_map;
    color_map[DIRT] = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
    color_map[GRASS] = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
    color_map[STONE] = glm::vec4(0.5f);
    color_map[LAVA] = glm::vec4(1.f, 0.f, 0.f, 1.f);

    int index = 0;
    std::vector<std::pair<int, int>> offsets = {std::pair<int, int>(0, 0),
                                               std::pair<int, int>(1, 0),
                                               std::pair<int, int>(1, 1),
                                               std::pair<int, int>(0, 1)};
    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 256; y++)
        {
            for (int z = 0; z < 16; z++)
            {
                BlockType block = getBlockAt(x, y, z);
                if (block == EMPTY) {
                    continue;
                }

                glm::ivec2 world_xz = getWorldCoordinates(position, x, z);
                BlockType adj_block;

                // check face to the negative-x directoin
                if (x == 0) {
                    Chunk* adj_chunk = negX_chunk;
                    if (adj_chunk != nullptr) {
                        adj_block = adj_chunk->getBlockAt(15, y, z);
                    } else {
                        adj_block = EMPTY;
                    }
                } else {
                    adj_block = getBlockAt(x - 1, y, z);
                }

                if (adj_block == EMPTY) {
                    // draw face in negative-x direction
                    for (std::pair<int, int> offset : offsets) {
                        pnc.push_back(glm::vec4(world_xz[0], y + offset.first, world_xz[1] + offset.second, 1));
                        pnc.push_back(glm::vec4(-1, 0, 0, 0));
                        pnc.push_back(color_map[block]);
                    }

                    for (int i = 0; i < 2; i++) {
                        idx.push_back(index);
                        idx.push_back(index + i + 1);
                        idx.push_back(index + i + 2);
                    }

                    index += 4;
                }

                // check face to the positive-x direction
                if (x == 15) {
                    Chunk* adj_chunk = posX_chunk;
                    if (adj_chunk != nullptr) {
                        adj_block = adj_chunk->getBlockAt(0, y, z);
                    } else {
                        adj_block = EMPTY;
                    }
                } else {
                    adj_block = getBlockAt(x + 1, y, z);
                }

                if (adj_block == EMPTY) {
                    // draw face in positive-x direction
                    for (std::pair<int, int> offset : offsets) {
                        pnc.push_back(glm::vec4(world_xz[0] + 1, y + offset.first, world_xz[1] + offset.second, 1));
                        pnc.push_back(glm::vec4(1, 0, 0, 0));
                        pnc.push_back(color_map[block]);
                    }

                    for (int i = 0; i < 2; i++) {
                        idx.push_back(index);
                        idx.push_back(index + i + 1);
                        idx.push_back(index + i + 2);
                    }

                    index += 4;
                }

                // check face to the negative-z directoin
                if (z == 0) {
                    Chunk* adj_chunk = negZ_chunk;
                    if (adj_chunk != nullptr) {
                        adj_block = adj_chunk->getBlockAt(x, y, 15);
                    } else {
                        adj_block = EMPTY;
                    }
                } else {
                    adj_block = getBlockAt(x, y, z - 1);
                }

                if (adj_block == EMPTY) {
                    // draw face in negative-z direction
                    for (std::pair<int, int> offset : offsets) {
                        pnc.push_back(glm::vec4(world_xz[0] + offset.first, y + offset.second, world_xz[1], 1));
                        pnc.push_back(glm::vec4(0, 0, -1, 0));
                        pnc.push_back(color_map[block]);
                    }

                    for (int i = 0; i < 2; i++) {
                        idx.push_back(index);
                        idx.push_back(index + i + 1);
                        idx.push_back(index + i + 2);
                    }

                    index += 4;
                }

                // check face to the positive-z direction
                if (z == 15) {
                    Chunk* adj_chunk = posZ_chunk;
                    if (adj_chunk != nullptr) {
                        adj_block = adj_chunk->getBlockAt(x, y, 0);
                    } else {
                        adj_block = EMPTY;
                    }
                } else {
                    adj_block = getBlockAt(x, y, z + 1);
                }

                if (adj_block == EMPTY) {
                    // draw face in positive-z direction
                    for (std::pair<int, int> offset : offsets) {
                        pnc.push_back(glm::vec4(world_xz[0] + offset.first, y + offset.second, world_xz[1] + 1, 1));
                        pnc.push_back(glm::vec4(0, 0, 1, 0));
                        pnc.push_back(color_map[block]);
                    }

                    for (int i = 0; i < 2; i++) {
                        idx.push_back(index);
                        idx.push_back(index + i + 1);
                        idx.push_back(index + i + 2);
                    }

                    index += 4;
                }

                // check face to the negative-y direction
                if (y == 0 || getBlockAt(x, y - 1, z) == EMPTY) {
                    // draw face to the negative-y face
                    for (std::pair<int, int> offset : offsets) {
                        pnc.push_back(glm::vec4(world_xz[0] + offset.first, y, world_xz[1] + offset.second, 1));
                        pnc.push_back(glm::vec4(0, -1, 0, 0));
                        pnc.push_back(color_map[block]);
                    }

                    for (int i = 0; i < 2; i++) {
                        idx.push_back(index);
                        idx.push_back(index + i + 1);
                        idx.push_back(index + i + 2);
                    }

                    index += 4;
                }

               // check face to the positive-y direction
                if (y == 255 || getBlockAt(x, y + 1, z) == EMPTY) {
                    // draw face to the positive-y face
                    for (std::pair<int, int> offset : offsets) {
                        pnc.push_back(glm::vec4(world_xz[0] + offset.first, y + 1, world_xz[1] + offset.second, 1));
                        pnc.push_back(glm::vec4(0, 1, 0, 0));
                        pnc.push_back(color_map[block]);
                    }

                    for (int i = 0; i < 2; i++) {
                        idx.push_back(index);
                        idx.push_back(index + i + 1);
                        idx.push_back(index + i + 2);
                    }

                    index += 4;
                }
            }
        }
    }

    count = idx.size();

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePNC();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPNC);
    context->glBufferData(GL_ARRAY_BUFFER, pnc.size() * sizeof(glm::vec4), pnc.data(), GL_STATIC_DRAW);
}

GLenum Chunk::drawMode() {
    return GL_TRIANGLES;
}

BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return m_blocks[x + 16 * z + 256 * y];
}

BlockType& Chunk::getBlockAt(int x, int y, int z) {
    return m_blocks[x + 16 * z + 256 * y];
}

std::pair<int, int> getOrigin(int x, int z) {
    std::pair<int, int> xz;
    if (x >= 0) {
        xz.first = x / 16 * 16;
    } else {
        xz.first = (x - 15) / 16 * 16;
    }
    if (z >= 0) {
        xz.second = z / 16 * 16;
    } else {
        xz.second = (z - 15) / 16 * 16;
    }

    return xz;
}

glm::ivec2 getChunkCoordinates(int x, int z) {
    return glm::ivec2(x % 16, z % 16);
}

glm::ivec2 getWorldCoordinates(glm::ivec2 position, int x, int z) {
    return glm::ivec2(position[0] + x, position[1] + z);
}


