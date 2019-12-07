#pragma once
#include <QList>
#include <la.h>
#include <drawable.h>
#include <array>

#include "lsystem.h"
#include <memory>

#define uPtr std::unique_ptr
#define mkU std::make_unique

class LSystem;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, LAVA, WATER, SNOW, COAL, IRON
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
    std::vector<GLuint> idxOpaque;
    std::vector<GLuint> idxTrans;
    std::vector<glm::vec4> pncOpaque; // vector that stores position, normal, and color
    std::vector<glm::vec4> pncTrans;


    Chunk();
    Chunk(OpenGLContext* context, glm::ivec2 origin);
    ~Chunk();

    void createVBOs();
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

    std::map<std::pair<int, int>, uPtr<Chunk>> m_chunks;

    void CreateTestScene();
    void create();
    void destroy();

    glm::ivec3 dimensions;

    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.
    //fbm functions
    float noise2D(glm::vec2 n);
    float interpNoise2D(float x, float y);
    float fbm(float x, float y);
    float modFbm(float x, float y);

    //perlin noise for moist/bump values
    float random2(glm::vec2 n);
    float surflet(glm::vec2 p, glm::vec2 gridPoint);
    float perlinNoise(glm::vec2 uv);

    //getting the overall height
    float overallHeight(float bilerp1, float bilerp2, float moist);
    float bilerp(float desert, float mountain, float bump);
    float desertHeight(float x, float z);
    float mountainHeight(float x, float z);
    float islandHeight(float x, float z);
    float grasslandHeight(float x, float z);


    //add block
    void addBlock(glm::vec3 eye, glm::vec3 look);
    //delete block
    void deleteBlock(glm::vec3 eye, glm::vec3 look);
    //ray marching helper function
    glm::vec4 rayMarch(glm::vec3 eye, glm::vec3 look);

    //helper function to decide regenerate terrain and which direction
    std::vector<int> checkRegenerate(glm::vec3 eye);
    //regenerating terrain
    void regenerateTerrain(std::vector<int> regenCaseList, glm::vec3 eye);
    //helper function for getting current 4X4 chunk origin
    glm::ivec2 terrOrigin(glm::vec3 eye);

    void setNeighbors(Chunk* chunk);
};

std::pair<int, int> getOrigin(int x, int z);
glm::ivec2 getChunkCoordinates(int x, int z); // given a world coordinate, converts it to a chunk coordinate
glm::ivec2 getWorldCoordinates(glm::ivec2 position, int x, int z); // given the chunk's position and its coordinates
                                                                    // converts it to a world coordinate
