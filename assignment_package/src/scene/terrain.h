#pragma once
#include <QList>
#include <la.h>
#include <drawable.h>
#include <array>

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE
};

class Chunk : public Drawable
{
public:
    std::array<BlockType, 65536> m_blocks;
    glm::ivec2 position; // origin position (x, z coordinates) of the chunk
    Chunk* negX_chunk; // pointer to adjacent chunk in the negative-x direction
    Chunk* posX_chunk; // pointer to adjacent chunk in the positive-x direction
    Chunk* negZ_chunk; // pointer to adjacent chunk in the negative-z direction
    Chunk* posZ_chunk; // pointer to adjacent chunk in the positive-z direction

    Chunk();
    Chunk(OpenGLContext* context, glm::ivec2 origin);
    ~Chunk();

    void create() override;
    GLenum drawMode() override;
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType& getBlockAt(int x, int y, int z);
};

class Terrain
{
public:
    Terrain(OpenGLContext* context);
    OpenGLContext* context;

    std::map<uint64_t, Chunk> m_chunks;

    void CreateTestScene();

    glm::ivec3 dimensions;

    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.

    //fbm functions=============================================================
    //noise basis function
    float noise2D(glm::vec2 n);

    //interpNoise2D
    float interpNoise2D(float x, float y);

    //fbm function
    float fbm(float x, float y);

    //add/delete block function===================================================
    void addBlock(float x, float y, float z);
    void deleteBlock(float x, float y, float z);
};

uint64_t serialize(int x, int z); // given a world coordinate, converts it to a key for m_chunks
glm::ivec2 deserialize(uint64_t key);
glm::ivec2 getChunkCoordinates(int x, int z); // given a world coordinate, converts it to a chunk coordinate
glm::ivec2 getWorldCoordinates(glm::ivec2 position, int x, int z); // given the chunk's position and its coordinates
                                                                    // converts it to a world coordinate
