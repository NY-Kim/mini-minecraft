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

        if (m_chunks.find(std::pair<int, int>(key.first - 16, key.second)) != m_chunks.end()) {
            Chunk* negX = &m_chunks[std::pair<int, int>(key.first - 16, key.second)];
            chunk.negX_chunk = negX;
            negX->posX_chunk = &chunk;
        }

        if (m_chunks.find(std::pair<int, int>(key.first + 16, key.second)) != m_chunks.end()) {
            Chunk* posX = &m_chunks[std::pair<int, int>(key.first + 16, key.second)];
            chunk.posX_chunk = posX;
            posX->negX_chunk = &chunk;
        }

        if (m_chunks.find(std::pair<int, int>(key.first, key.second - 16)) != m_chunks.end()) {
            Chunk* negZ = &m_chunks[std::pair<int, int>(key.first, key.second - 16)];
            chunk.negZ_chunk = negZ;
            negZ->posZ_chunk = &chunk;
        }

        if (m_chunks.find(std::pair<int, int>(key.first, key.second + 16)) != m_chunks.end()) {
            Chunk* posZ = &m_chunks[std::pair<int, int>(key.first, key.second + 16)];
            chunk.posZ_chunk = posZ;
            posZ->negZ_chunk = &chunk;
        }

        m_chunks[key] = chunk;
    }
}

void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            for(int y = 127; y < 256; ++y)
            {
                if(y <= 128)
                {
                    if((x + z) % 2 == 0)
                    {
                        setBlockAt(x, y, z, STONE);
                    }
                    else
                    {
                        setBlockAt(x, y, z, DIRT);
                    }
                }
                else
                {
                    setBlockAt(x, y, z, EMPTY);
                }
            }
        }
    }

    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x)
    {
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    for(int y = 129; y < 140; ++y)
    {
       setBlockAt(32, y, 32, GRASS);
    }

    for (std::map<std::pair<int, int>, Chunk>::iterator i = m_chunks.begin(); i != m_chunks.end(); i++) {
        i->second.Chunk::create();
    }
}

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
    glm::ivec2 xz;
    if (x >= 0) {
        xz[0] = x % 16;
    } else {
        xz[0] = (x % 16 + 16) % 16;
    }
    if (z >= 0) {
        xz[1] = z % 16;
    } else {
        xz[1] = (z % 16 + 16) % 16;
    }

    return xz;
}

glm::ivec2 getWorldCoordinates(glm::ivec2 position, int x, int z) {
    return glm::ivec2(position[0] + x, position[1] + z);
}


