#include <scene/terrain.h>

#include <scene/cube.h>

Terrain::Terrain() : dimensions(64, 256, 64)
{}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    // TODO: Make this work with your new block storage!
    return m_blocks[x][y][z];
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    // TODO: Make this work with your new block storage!
    m_blocks[x][y][z] = t;
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
                    m_blocks[x][y][z] = STONE;
                } else {
                    m_blocks[x][y][z] = DIRT;
                }
            }
            m_blocks[x][(int)glm::floor(height)][z] = GRASS;

//            for(int y = 127; y < 256; ++y)
//            {
//                if(y <= 128)
//                {
//                    if((x + z) % 2 == 0)
//                    {
//                        m_blocks[x][y][z] = STONE;
//                    }
//                    else
//                    {
//                        m_blocks[x][y][z] = DIRT;
//                    }
//                }
//                else
//                {
//                    m_blocks[x][y][z] = EMPTY;
//                }
//            }
        }
    }
    // Add "walls" for collision testing
//    for(int x = 0; x < 64; ++x)
//    {
//        m_blocks[x][129][0] = GRASS;
//        m_blocks[x][130][0] = GRASS;
//        m_blocks[x][129][63] = GRASS;
//        m_blocks[0][130][x] = GRASS;
//    }
//    for(int y = 129; y < 140; ++y)
//    {
//        m_blocks[32][y][32] = GRASS;
//    }
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
void Terrain::addBlock(float x, float y, float z)
{

}

//delete
void Terrain::deleteBlock(float x, float y, float z)
{

}
