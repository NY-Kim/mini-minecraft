#include <scene/terrain.h>

#include <scene/cube.h>
#include <array>
#include <iostream>

Terrain::Terrain(OpenGLContext* context)
    : context(context), m_chunks(std::map<uint64_t, Chunk>()), dimensions(64, 256, 64)
{
    for(int x = 0; x < this->dimensions[0]; x += 16) {
        for(int z = 0; z < this->dimensions[2]; z += 16) {
            glm::ivec2 origin(x,z);
            Chunk chunk = Chunk(context, origin);

            if (m_chunks.find(serialize(x - 16, z)) != m_chunks.end()) {
                Chunk* negX = &m_chunks[serialize(x - 16, z)];
                chunk.negX_chunk = negX;
                negX->posX_chunk = &chunk;
            }

            if (m_chunks.find(serialize(x + 16, z)) != m_chunks.end()) {
                Chunk* posX = &m_chunks[serialize(x + 16, z)];
                chunk.posX_chunk = posX;
                posX->negX_chunk = &chunk;
            }

            if (m_chunks.find(serialize(x, z - 16)) != m_chunks.end()) {
                Chunk* negZ = &m_chunks[serialize(x, z - 16)];
                chunk.negZ_chunk = negZ;
                negZ->posZ_chunk = &chunk;
            }

            if (m_chunks.find(serialize(x, z + 16)) != m_chunks.end()) {
                Chunk* posZ = &m_chunks[serialize(x, z + 16)];
                chunk.posZ_chunk = posZ;
                posZ->negZ_chunk = &chunk;
            }

            m_chunks[serialize(x, z)] = chunk;
        }
    }
}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    uint64_t key = serialize(x, z);
    glm::ivec2 chunk_xz = getChunkCoordinates(x, z);

    return m_chunks.at(key).getBlockAt(chunk_xz[0], y, chunk_xz[1]);
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    uint64_t key = serialize(x, z);
    glm::ivec2 chunk_xz = getChunkCoordinates(x, z);

    m_chunks[key].getBlockAt(chunk_xz[0], y, chunk_xz[1]) = t;
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

    for (std::map<uint64_t, Chunk>::iterator i = m_chunks.begin(); i != m_chunks.end(); i++) {
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

uint64_t serialize(int x, int z) {
    uint32_t x_coor, z_coor;
    if (x >= 0) {
        x_coor = (uint32_t) x / 16 * 16;
    } else {
        x_coor = (x - 15) / 16 * 16;
    }
    if (z >= 0) {
        z_coor = (uint32_t) z / 16 * 16;
    } else {
        z_coor = (z - 15) / 16 * 16;
    }
    return (uint64_t) (x_coor) << 32 | z_coor;
}

glm::ivec2 deserialize(uint64_t key) {
    glm::ivec2 xz;
    xz[0] = key >> 32;
    xz[1] = key & 0xffffffff;
    std::cout << xz[0] << ", " << xz[1] << std::endl;
    return xz;
}

glm::ivec2 getChunkCoordinates(int x, int z) {
    return glm::ivec2(x % 16, z % 16);
}

glm::ivec2 getWorldCoordinates(glm::ivec2 position, int x, int z) {
    return glm::ivec2(position[0] + x, position[1] + z);
}


