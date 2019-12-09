#include <scene/terrain.h>
#include <array>
#include <iostream>

  Terrain::Terrain(OpenGLContext* context)
      : context(context), m_chunks(std::map<std::pair<int, int>, uPtr<Chunk>>()), dimensions(64, 256, 64), biomeType()
  {}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
  std::pair<int, int> key = getOrigin(x, z);
  glm::ivec2 chunk_xz = getChunkCoordinates(x, z);

  if (m_chunks.find(key) != m_chunks.end()) {
      return m_chunks.at(key)->getBlockAt(chunk_xz[0], y, chunk_xz[1]);
  } else {
      return EMPTY;
  }

}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    std::pair<int, int> key = getOrigin(x, z);
    glm::ivec2 chunk_xz = getChunkCoordinates(x, z);

    if (m_chunks.find(key) != m_chunks.end() && m_chunks[key] != nullptr) {
        m_chunks[key]->getBlockAt(chunk_xz[0], y, chunk_xz[1]) = t;
    } else {
        glm::ivec2 origin = glm::ivec2(key.first, key.second);
        uPtr<Chunk> chunk = mkU<Chunk>(context, origin);
        chunk->getBlockAt(chunk_xz[0], y, chunk_xz[1]) = t;

        if (m_chunks.find(std::pair<int, int>(key.first - 16, key.second)) != m_chunks.end()) {
            Chunk* negX = m_chunks[std::pair<int, int>(key.first - 16, key.second)].get();
            chunk->negX_chunk = negX;
            negX->posX_chunk = chunk.get();
        }

        if (m_chunks.find(std::pair<int, int>(key.first + 16, key.second)) != m_chunks.end()) {
            Chunk* posX = m_chunks[std::pair<int, int>(key.first + 16, key.second)].get();
            chunk->posX_chunk = posX;
            posX->negX_chunk = chunk.get();
        }

        if (m_chunks.find(std::pair<int, int>(key.first, key.second - 16)) != m_chunks.end()) {
            Chunk* negZ = m_chunks[std::pair<int, int>(key.first, key.second - 16)].get();
            chunk->negZ_chunk = negZ;
            negZ->posZ_chunk = chunk.get();
        }

        if (m_chunks.find(std::pair<int, int>(key.first, key.second + 16)) != m_chunks.end()) {
            Chunk* posZ = m_chunks[std::pair<int, int>(key.first, key.second + 16)].get();
            chunk->posZ_chunk = posZ;
            posZ->negZ_chunk = chunk.get();
        }
        m_chunks[key] = std::move(chunk);
    }
}

void Terrain::CreateRiverScene()
{

    //Create the basic terrain floor
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            float height = fbm((x / 64.0), (z / 64.0));
            height = pow(height, 3.f) * 32.0 + 128.0;

            for (int y = 1; y < height; y++) {
                if (y <= 128) {
                    setBlockAt(x, y, z, STONE);
                } else {
                    setBlockAt(x, y, z, DIRT);
                }
            }
            int y = (int)glm::floor(height);
            setBlockAt(x, y, z, GRASS);
        }
    }

    LSystem linearRiver = LSystem(this, QString("linear"));
    linearRiver.generateRiver();

    LSystem deltaRiver = LSystem(this, QString("delta"));
    deltaRiver.generateRiver();
}

void Terrain::CreateTestScene()
{

    //Create the basic terrain floor
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            glm::vec2 mb = glm::vec2(mb_fbm(x / 700.f, z / 700.f),
                                     mb_fbm((x + 100.34) / 700.f, (z + 678.98234) / 700.f));
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
            float height = overallHeight(x, z, mb);
            if (biomeType == QString("mustafar")) {
                setMustafar(x, z, height);
            } else if (biomeType == QString("canyon")) {
                setCanyon(x, z, height);
            } else if (biomeType == QString("grassland")) {
                setGrassland(x, z, height);
            } else {
                setSnowland(x, z, height);
            }
        }
    }
}

void Terrain::create() {
    for (std::map<std::pair<int, int>, uPtr<Chunk>>::iterator i = m_chunks.begin(); i != m_chunks.end(); i++) {
        i->second->createVBOs();
        i->second->create();
    }
}

void Terrain::destroy() {
    for (std::map<std::pair<int, int>, uPtr<Chunk>>::iterator i = m_chunks.begin(); i != m_chunks.end(); i++) {
        i->second->destroy();
        i->second->idxOpaque.clear();
        i->second->idxTrans.clear();
        i->second->pncOpaque.clear();
        i->second->pncTrans.clear();
    }
}

//fbm functions=============================================================
//mb_noise basis function
float Terrain::mb_noise2D(glm::vec2 n)
{
    return (glm::fract(sin(glm::dot(n, glm::vec2(311.7, 191.999))) * 17434.2371));
}

//mb_interpNoise2D
float Terrain::mb_interpNoise2D(float x, float y)
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
float Terrain::mb_fbm(float x, float y)
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

float Terrain::modGrass(float x, float y)
{
    float height = fbm(x, y);
    height = pow(height, 3.f) * 6.0 + 128.0;
    return height;
}

float Terrain::modMustafar(float x, float y)
{
    float height = fbm(x, y);
    height = glm::smoothstep(0.4f, 0.55f, height) * 1 + glm::smoothstep(0.55f, 0.68f, height) * 12 + 130.0 + glm::smoothstep(0.6f, 0.8f, height) * 10;
    return height;
}

float Terrain::modSnow(float x, float y)
{
    float height = fbm(x, y);
    height = pow(height, 3.f) * 15.0 + pow(height, 3.5f) * 12.0 + 148.0 + glm::smoothstep(0.4f, 0.8f, height) * 15;
    return height;
}

float Terrain::modCanyon(float x, float y)
{
    float height = fbm(x, y);
    height = glm::smoothstep(0.55f, 0.65f, height) * 20 + 128.0 + glm::smoothstep(0.5f, 0.6f, height) * 3 + glm::smoothstep(0.6f, 0.7f, height) * 13;
    return height;
}

//BIOME=======================================================================
//getting the overall height
float Terrain::overallHeight(float x, float z, glm::vec2 moistBump)
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
float Terrain::bilerp(float biome1, float biome2, float bump)
{
    return glm::mix(biome1, biome2, glm::smoothstep(0.45f, 0.55f, bump));
}

float Terrain::canyonHeight(float x, float z)
{
    return modCanyon(x, z);
}

float Terrain::grasslandHeight(float x, float z)
{
    return modGrass(x, z);
}

float Terrain::snowlandHeight(float x, float z)
{
    return  modSnow(x, z);
}

float Terrain::mustafarHeight(float x, float z)
{
    return modMustafar(x, z);
}

void Terrain::setCanyon(float x, float z, float height)
{
    for (int y = 1; y < height; y++) {
        if (y < 127) {
            setBlockAt(x, y, z, STONE);
        } else if (y < 138) {
            setBlockAt(x, y, z, SAND);
        } else if (y < 142) {
            setBlockAt(x, y, z, ORANGE);
        } else if (y < 145) {
            setBlockAt(x, y, z, IVORY);
        } else if (y < 147) {
            setBlockAt(x, y, z, BROWN);
        } else if (y < 152) {
            setBlockAt(x, y, z, IVORY);
        } else {
            setBlockAt(x, y, z, ORANGE);
        }
    }
}

void Terrain::setGrassland(float x, float z, float height)
{
    for (int y = 1; y < height; y++) {
        if (y <= 128) {
            setBlockAt(x, y, z, STONE);
        } else {
            setBlockAt(x, y, z, DIRT);
        }
    }
    int y = (int)glm::floor(height);
    setBlockAt(x, y, z, GRASS);
}

void Terrain::setSnowland(float x, float z, float height)
{
    for (int y = 1; y < height; y++) {
        if (y <= 128) {
            setBlockAt(x, y, z, STONE);
        } else {
            setBlockAt(x, y, z, DIRT);
        }
    }
    int y = (int)glm::floor(height);
    setBlockAt(x, y, z, SNOW);
}

void Terrain::setMustafar(float x, float z, float height)
{
    for (int y = 1; y < height; y++) {
        if (y < 127) {
            setBlockAt(x, y, z, STONE);
        } else if (y < 130){
            setBlockAt(x, y, z, LAVA);
        } else {
            setBlockAt(x, y, z, DARK);
        }
    }
}

//add/delete block function===================================================
//add
void Terrain::addBlock(glm::vec3 eye, glm::vec3 look, BlockType t)
{
    glm::vec4 rayMarched = rayMarch(eye, look);
    if (rayMarched[3] == 1) {
        glm::vec3 blockCoord = glm::vec3(rayMarched);
        glm::vec3 blockFloor = glm::floor(blockCoord);
        glm::vec3 blockCeil = glm::ceil(blockCoord);
        glm::vec3 blockMid = (blockFloor + blockCeil) / 2.0f;
        glm::vec3 normal = blockCoord - blockMid;
        float maxCoord = fmax(fmax(abs(normal[0]), abs(normal[1])), abs(normal[2]));
        if (maxCoord == abs(normal[0])) {
            if (normal[0] > 0) {
                normal[0] = 1;
            } else {
                normal[0] = -1;
            }
            normal[1] = 0;
            normal[2] = 0;
        } else if (maxCoord == abs(normal[1])) {
            if (normal[1] > 0) {
                normal[1] = 1;
            } else {
                normal[1] = -1;
            }
            normal[0] = 0;
            normal[2] = 0;
        } else {
            if (normal[2] > 0) {
                normal[2] = 1;
            } else {
                normal[2] = -1;
            }
            normal[0] = 0;
            normal[1] = 0;
        }
        blockCoord = glm::floor(blockCoord + normal);
        setBlockAt((int)blockCoord[0], (int)blockCoord[1], (int)blockCoord[2], t);
    }
}

//delete
void Terrain::deleteBlock(glm::vec3 eye, glm::vec3 look)
{
    glm::vec4 rayMarched = rayMarch(eye, look);
    if (rayMarched[3] == 1) {
        glm::vec3 blockCoord = glm::vec3(rayMarched);
        blockCoord = glm::floor(blockCoord);
        if (getBlockAt((int)blockCoord[0] - 1, (int)blockCoord[1], (int)blockCoord[2]) == WATER ||
                getBlockAt((int)blockCoord[0] + 1, (int)blockCoord[1], (int)blockCoord[2]) == WATER ||
                getBlockAt((int)blockCoord[0], (int)blockCoord[1] + 1, (int)blockCoord[2]) == WATER ||
                getBlockAt((int)blockCoord[0], (int)blockCoord[1], (int)blockCoord[2] + 1) == WATER ||
                getBlockAt((int)blockCoord[0], (int)blockCoord[1], (int)blockCoord[2] - 1) == WATER) {
            setBlockAt((int)blockCoord[0], (int)blockCoord[1], (int)blockCoord[2], WATER);
        } else {
            setBlockAt((int)blockCoord[0], (int)blockCoord[1], (int)blockCoord[2], EMPTY);
        }
    }
}

//helper function for ray marching
glm::vec4 Terrain::rayMarch(glm::vec3 eye, glm::vec3 look)
{
    int counter = 0;
    float step = 0.01f;
    bool hit = false;
    glm::vec3 currPos = eye;
    glm::vec3 currBlockCoord;
    while (!hit && (counter * step < 10.0f)) {
        currPos = currPos + (look * step);
        currBlockCoord = glm::floor(currPos);
        if (getBlockAt((int)currBlockCoord[0], (int)currBlockCoord[1], (int)currBlockCoord[2]) == EMPTY ||
                getBlockAt((int)currBlockCoord[0], (int)currBlockCoord[1], (int)currBlockCoord[2]) == WATER) {
            counter++;
            continue;
        } else {
            hit = true;
        }
    }
    glm::vec4 result = glm::vec4(currPos, 0);
    if (hit) {
        result[3] = 1;
    }
    return result;
}

//helper function to decide regenerate terrain and which direction
//N:1 > NE:2 > E:3 > SE:4 > S:5 > SW:6 > W:7 > NW:8
std::vector<int> Terrain::checkRegenerate(glm::vec3 eye)
{
    std::vector<int> cases;
    if (m_chunks.find(getOrigin(int(eye.x), int(eye.z) + 500)) == m_chunks.end()) {
        cases.push_back(1);
    }
    if (m_chunks.find(getOrigin(int(eye.x) + 500, int(eye.z) + 500)) == m_chunks.end()) {
        cases.push_back(2);
    }
    if (m_chunks.find(getOrigin(int(eye.x) + 500, int(eye.z))) == m_chunks.end()) {
        cases.push_back(3);
    }
    if (m_chunks.find(getOrigin(int(eye.x) + 500, int(eye.z) - 500)) == m_chunks.end()) {
        cases.push_back(4);
    }
    if (m_chunks.find(getOrigin(int(eye.x), int(eye.z) - 500)) == m_chunks.end()) {
        cases.push_back(5);
    }
    if (m_chunks.find(getOrigin(int(eye.x) - 500, int(eye.z) - 500)) == m_chunks.end()) {
        cases.push_back(6);
    }
    if (m_chunks.find(getOrigin(int(eye.x) - 500, int(eye.z))) == m_chunks.end()) {
        cases.push_back(7);
    }
    if (m_chunks.find(getOrigin(int(eye.x) - 500, int(eye.z) + 500)) == m_chunks.end()) {
        cases.push_back(8);
    }
    return cases;
}

//regenerating terrain
void Terrain::regenerateTerrain(std::vector<int> regenCaseList, glm::vec3 eye)
{
    glm::ivec2 origin = terrOrigin(eye);
    int originX = int(origin[0]);
    int originZ = int(origin[1]);
    for (int regenCase : regenCaseList) {
        int xOffset = (regenCase == 2 || regenCase == 3 || regenCase == 4) ? 64 :
                                                                             (regenCase == 6 || regenCase == 7 || regenCase == 8) ? -64 : 0;
        int zOffset = (regenCase == 1 || regenCase == 2 || regenCase == 8) ? 64 :
                                                                             (regenCase == 4 || regenCase == 5 || regenCase == 6) ? -64 : 0;

        for(int x = 0; x < 64; ++x)
        {
            for(int z = 0; z < 64; ++z)
            {
                glm::vec2 mb = glm::vec2(mb_fbm((originX + x + xOffset) / 700.f, (originZ + z + zOffset) / 700.f),
                                         mb_fbm((originX + x + xOffset + 100.34) / 700.f, (originZ + z + zOffset + 678.98234) / 700.f));
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

                float height = overallHeight((originX + x + xOffset), (originZ + z + zOffset), mb);

                if (biomeType == QString("mustafar")) {
                    setMustafar((originX + x + xOffset), (originZ + z + zOffset), height);
                } else if (biomeType == QString("canyon")) {
                    setCanyon((originX + x + xOffset), (originZ + z + zOffset), height);
                } else if (biomeType == QString("grassland")) {
                    setGrassland((originX + x + xOffset), (originZ + z + zOffset), height);
                } else {
                    setSnowland((originX + x + xOffset), (originZ + z + zOffset), height);
                }
//                float height = modGrass(((originX + x + xOffset) / (64.0)), ((originZ + z + zOffset) / (64.0)));
//                for (int y = 1; y < height; y++) {
//                    if (y <= 128) {
//                        setBlockAt(originX + x + xOffset, y, originZ + z + zOffset, STONE);
//                    } else {
//                        setBlockAt(originX + x + xOffset, y, originZ + z + zOffset, DIRT);
//                    }
//                }
//                int y = (int)glm::floor(height);
//                setBlockAt(originX + x, y, originZ + 64 + z, GRASS);
            }
        }
    }
}

//helper function for getting current 4X4 chunk origin
glm::ivec2 Terrain::terrOrigin(glm::vec3 eye)
{
    glm::ivec2 terrOrigin;
    if (eye.x >= 0) {
        terrOrigin[0] = (int)eye.x / 64 * 64;
    } else {
        terrOrigin[0] = (int)(eye.x - 63) / 64 * 64;
    }
    if (eye.z >= 0) {
        terrOrigin[1] = (int)eye.z / 64 * 64;
    } else {
        terrOrigin[1] = (int)(eye.z - 63) / 64 * 64;
    }
    return terrOrigin;
}

void Terrain::setNeighbors(Chunk *chunk) {
    std::pair<int, int> key(chunk->position[0], chunk->position[1]);
    if (m_chunks.find(std::pair<int, int>(key.first - 16, key.second)) != m_chunks.end()) {
        Chunk* negX = m_chunks[std::pair<int, int>(key.first - 16, key.second)].get();
        chunk->negX_chunk = negX;
        negX->posX_chunk = m_chunks[key].get();
    }

    if (m_chunks.find(std::pair<int, int>(key.first + 16, key.second)) != m_chunks.end()) {
        Chunk* posX = m_chunks[std::pair<int, int>(key.first + 16, key.second)].get();
        chunk->posX_chunk = posX;
        posX->negX_chunk = m_chunks[key].get();
    }

    if (m_chunks.find(std::pair<int, int>(key.first, key.second - 16)) != m_chunks.end()) {
        Chunk* negZ = m_chunks[std::pair<int, int>(key.first, key.second - 16)].get();
        chunk->negZ_chunk = negZ;
        negZ->posZ_chunk = m_chunks[key].get();
    }

    if (m_chunks.find(std::pair<int, int>(key.first, key.second + 16)) != m_chunks.end()) {
        Chunk* posZ = m_chunks[std::pair<int, int>(key.first, key.second + 16)].get();
        chunk->posZ_chunk = posZ;
        posZ->negZ_chunk = m_chunks[key].get();
    }
}

Chunk::Chunk() : Drawable(nullptr) {}

Chunk::Chunk(OpenGLContext *context, glm::ivec2 origin)
    : Drawable(context), position(glm::ivec2(origin[0], origin[1])),
      negX_chunk(nullptr), posX_chunk(nullptr), negZ_chunk(nullptr), posZ_chunk(nullptr),
      idxOpaque(), idxTrans(), pncOpaque(), pncTrans()
{
    m_blocks.fill(EMPTY);
}

Chunk::~Chunk() {}

void Chunk::createVBOs() {
    std::vector<GLuint>* idx;
    std::vector<glm::vec4>* pnc;
    int indexOpaque = 0;
    int indexTrans = 0;
    int* index;

    std::map<BlockType, glm::vec4> color_map;
    color_map[DIRT] = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
    color_map[GRASS] = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
    color_map[STONE] = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    color_map[LAVA] = glm::vec4(1.f, 0.5f, 0.5f, 1.0f);
    color_map[WATER] = glm::vec4(0.5f, 0.5f, 1.5f, 0.7f);
    color_map[SNOW] = glm::vec4(1.f, 1.f, 1.f, 1.0f);
    color_map[COAL] = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    color_map[IRON] = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    color_map[ORANGE] = glm::vec4(255.f, 153.f, 51.f, 255.f) / 255.f;
    color_map[BROWN] = glm::vec4(102.f, 51.f, 0.f, 255.f) / 255.f;
    color_map[IVORY] = glm::vec4(255.f, 255.f, 204.f, 255.f) / 255.f;
    color_map[SAND] = glm::vec4(208.f, 189.f, 115.f, 255.f) / 255.f;
    color_map[DARK] = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    std::map<std::pair<BlockType, int>, glm::vec2> uv_map; // 0 - top, 1 - side, 2 - bottom
    uv_map[std::pair<BlockType, int>(DIRT, 0)] = glm::vec2(2, 15) / 16.f;
    uv_map[std::pair<BlockType, int>(DIRT, 1)] = glm::vec2(2, 15) / 16.f;
    uv_map[std::pair<BlockType, int>(DIRT, 2)] = glm::vec2(2, 15) / 16.f;
    uv_map[std::pair<BlockType, int>(GRASS, 0)] = glm::vec2(8, 13) / 16.f;
    uv_map[std::pair<BlockType, int>(GRASS, 1)] = glm::vec2(3, 15) / 16.f;
    uv_map[std::pair<BlockType, int>(GRASS, 2)] = glm::vec2(2, 15) / 16.f;
    uv_map[std::pair<BlockType, int>(STONE, 0)] = glm::vec2(1, 15) / 16.f;
    uv_map[std::pair<BlockType, int>(STONE, 1)] = glm::vec2(1, 15) / 16.f;
    uv_map[std::pair<BlockType, int>(STONE, 2)] = glm::vec2(1, 15) / 16.f;
    uv_map[std::pair<BlockType, int>(LAVA, 0)] = glm::vec2(13, 1) / 16.f;
    uv_map[std::pair<BlockType, int>(LAVA, 1)] = glm::vec2(14, 0) / 16.f;
    uv_map[std::pair<BlockType, int>(LAVA, 2)] = glm::vec2(13, 1) / 16.f;
    uv_map[std::pair<BlockType, int>(WATER, 0)] = glm::vec2(13, 3) / 16.f;
    uv_map[std::pair<BlockType, int>(WATER, 1)] = glm::vec2(14, 2) / 16.f;
    uv_map[std::pair<BlockType, int>(WATER, 2)] = glm::vec2(13, 3) / 16.f;
    uv_map[std::pair<BlockType, int>(SNOW, 0)] = glm::vec2(2, 11) / 16.f;
    uv_map[std::pair<BlockType, int>(SNOW, 1)] = glm::vec2(4, 11) / 16.f;
    uv_map[std::pair<BlockType, int>(SNOW, 2)] = glm::vec2(2, 15) / 16.f;
    uv_map[std::pair<BlockType, int>(COAL, 0)] = glm::vec2(2, 13) / 16.f;
    uv_map[std::pair<BlockType, int>(COAL, 1)] = glm::vec2(2, 13) / 16.f;
    uv_map[std::pair<BlockType, int>(COAL, 2)] = glm::vec2(2, 13) / 16.f;
    uv_map[std::pair<BlockType, int>(IRON, 0)] = glm::vec2(1, 13) / 16.f;
    uv_map[std::pair<BlockType, int>(IRON, 1)] = glm::vec2(1, 13) / 16.f;
    uv_map[std::pair<BlockType, int>(IRON, 2)] = glm::vec2(1, 13) / 16.f;
    uv_map[std::pair<BlockType, int>(ORANGE, 0)] = glm::vec2(2, 2) / 16.f;
    uv_map[std::pair<BlockType, int>(ORANGE, 1)] = glm::vec2(2, 2) / 16.f;
    uv_map[std::pair<BlockType, int>(ORANGE, 2)] = glm::vec2(2, 2) / 16.f;
    uv_map[std::pair<BlockType, int>(BROWN, 0)] = glm::vec2(1, 5) / 16.f;
    uv_map[std::pair<BlockType, int>(BROWN, 1)] = glm::vec2(1, 5) / 16.f;
    uv_map[std::pair<BlockType, int>(BROWN, 2)] = glm::vec2(1, 5) / 16.f;
    uv_map[std::pair<BlockType, int>(IVORY, 0)] = glm::vec2(0, 4) / 16.f;
    uv_map[std::pair<BlockType, int>(IVORY, 1)] = glm::vec2(0, 4) / 16.f;
    uv_map[std::pair<BlockType, int>(IVORY, 2)] = glm::vec2(0, 4) / 16.f;
    uv_map[std::pair<BlockType, int>(SAND, 0)] = glm::vec2(14, 7) / 16.f;
    uv_map[std::pair<BlockType, int>(SAND, 1)] = glm::vec2(14, 7) / 16.f;
    uv_map[std::pair<BlockType, int>(SAND, 2)] = glm::vec2(14, 7) / 16.f;
    uv_map[std::pair<BlockType, int>(DARK, 0)] = glm::vec2(1, 8) / 16.f;
    uv_map[std::pair<BlockType, int>(DARK, 1)] = glm::vec2(1, 8) / 16.f;
    uv_map[std::pair<BlockType, int>(DARK, 2)] = glm::vec2(1, 8) / 16.f;

    std::map<BlockType, float> cos_pow_map;
    cos_pow_map[DIRT] = 50;
    cos_pow_map[GRASS] = 50;
    cos_pow_map[STONE] = 60;
    cos_pow_map[LAVA] = 50;
    cos_pow_map[WATER] = 70;
    cos_pow_map[SNOW] = 50;
    cos_pow_map[COAL] = 60;
    cos_pow_map[IRON] = 60;
    cos_pow_map[ORANGE] = 60;
    cos_pow_map[BROWN] = 60;
    cos_pow_map[IVORY] = 60;
    cos_pow_map[SAND] = 60;
    cos_pow_map[DARK] = 60;

    std::map<BlockType, float> ani_flag_map;
    ani_flag_map[DIRT] = 0;
    ani_flag_map[GRASS] = 0;
    ani_flag_map[STONE] = 0;
    ani_flag_map[LAVA] = 1;
    ani_flag_map[WATER] = 1;
    ani_flag_map[SNOW] = 0;
    ani_flag_map[COAL] = 0;
    ani_flag_map[IRON] = 0;
    ani_flag_map[ORANGE] = 0;
    ani_flag_map[BROWN] = 0;
    ani_flag_map[IVORY] = 0;
    ani_flag_map[SAND] = 0;
    ani_flag_map[DARK] = 0;

    std::vector<std::pair<int, int>> offsets = {std::pair<int, int>(0, 0),
                                               std::pair<int, int>(1, 0),
                                               std::pair<int, int>(1, 1),
                                               std::pair<int, int>(0, 1)};
    std::vector<glm::vec2> offset2 = {glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0)};

    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 256; y++)
        {
            for (int z = 0; z < 16; z++)
            {
                BlockType block = getBlockAt(x, y, z);
                if (block == EMPTY) {
                    continue;
                } else if (block == WATER) { // TODO: Add GLASS & ICE
                    pnc = &pncTrans;
                    idx = &idxTrans;
                    index = &indexTrans;
                } else {
                    pnc = &pncOpaque;
                    idx = &idxOpaque;
                    index = &indexOpaque;
                }

                glm::ivec2 world_xz = getWorldCoordinates(position, x, z);
                BlockType adj_block;
                int i = 0;

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

                if (adj_block == EMPTY || (block != WATER && adj_block == WATER)) {
                    // draw face in negative-x direction
                    for (std::pair<int, int> offset : offsets) {
                        pnc->push_back(glm::vec4(world_xz[0], y + offset.first, world_xz[1] + offset.second, 1));
                        pnc->push_back(glm::vec4(-1, 0, 0, 0));
                        pnc->push_back(color_map[block]);
                        glm::vec2 uv = uv_map[std::pair<BlockType, int>(block, 1)] + offset2[i++] / 16.f;
                        pnc->push_back(glm::vec4(uv[0], uv[1], cos_pow_map[block], ani_flag_map[block]));
                    }

                    for (int i = 0; i < 2; i++) {
                        idx->push_back(*index);
                        idx->push_back(*index + i + 1);
                        idx->push_back(*index + i + 2);
                    }


                    *index += 4;
                }

                // check face to the positive-x direction
                i = 0;
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

                if (adj_block == EMPTY || (block != WATER && adj_block == WATER)) {
                    // draw face in positive-x direction
                    for (std::pair<int, int> offset : offsets) {
                        pnc->push_back(glm::vec4(world_xz[0] + 1, y + offset.first, world_xz[1] + offset.second, 1));
                        pnc->push_back(glm::vec4(1, 0, 0, 0));
                        pnc->push_back(color_map[block]);
                        glm::vec2 uv = uv_map[std::pair<BlockType, int>(block, 1)] + offset2[i++] / 16.f;
                        pnc->push_back(glm::vec4(uv[0], uv[1], cos_pow_map[block], ani_flag_map[block]));
                    }

                    for (int i = 0; i < 2; i++) {
                        idx->push_back(*index);
                        idx->push_back(*index + i + 1);
                        idx->push_back(*index + i + 2);
                    }

                    *index += 4;
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

                if (adj_block == EMPTY || (block != WATER && adj_block == WATER)) {
                    // draw face in negative-z direction
                    for (std::pair<int, int> offset : offsets) {
                        pnc->push_back(glm::vec4(world_xz[0] + offset.first, y + offset.second, world_xz[1], 1));
                        pnc->push_back(glm::vec4(0, 0, -1, 0));
                        pnc->push_back(color_map[block]);
                        glm::vec2 uv = uv_map[std::pair<BlockType, int>(block, 1)] + glm::vec2(offset.first, offset.second) / 16.f;
                        pnc->push_back(glm::vec4(uv[0], uv[1], cos_pow_map[block], ani_flag_map[block]));
                    }

                    for (int i = 0; i < 2; i++) {
                        idx->push_back(*index);
                        idx->push_back(*index + i + 1);
                        idx->push_back(*index + i + 2);
                    }

                    *index += 4;
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

                if (adj_block == EMPTY || (block != WATER && adj_block == WATER)) {
                    // draw face in positive-z direction
                    for (std::pair<int, int> offset : offsets) {
                        pnc->push_back(glm::vec4(world_xz[0] + offset.first, y + offset.second, world_xz[1] + 1, 1));
                        pnc->push_back(glm::vec4(0, 0, 1, 0));
                        pnc->push_back(color_map[block]);
                        glm::vec2 uv = uv_map[std::pair<BlockType, int>(block, 1)] + glm::vec2(offset.first, offset.second) / 16.f;
                        pnc->push_back(glm::vec4(uv[0], uv[1], cos_pow_map[block], ani_flag_map[block]));
                    }

                    for (int i = 0; i < 2; i++) {
                        idx->push_back(*index);
                        idx->push_back(*index + i + 1);
                        idx->push_back(*index + i + 2);
                    }

                    *index += 4;
                }

                // check face to the negative-y direction
                adj_block = getBlockAt(x, y - 1, z);
                if (y == 0 || adj_block == EMPTY || (block != WATER && adj_block == WATER)) {
                    // draw face to the negative-y face
                    for (std::pair<int, int> offset : offsets) {
                        pnc->push_back(glm::vec4(world_xz[0] + offset.first, y, world_xz[1] + offset.second, 1));
                        pnc->push_back(glm::vec4(0, -1, 0, 1));
                        pnc->push_back(color_map[block]);
                        glm::vec2 uv = uv_map[std::pair<BlockType, int>(block, 2)] + glm::vec2(offset.first, offset.second) / 16.f;
                        pnc->push_back(glm::vec4(uv[0], uv[1], cos_pow_map[block], ani_flag_map[block]));
                    }

                    for (int i = 0; i < 2; i++) {
                        idx->push_back(*index);
                        idx->push_back(*index + i + 1);
                        idx->push_back(*index + i + 2);
                    }

                    *index += 4;
                }

                // check face to the positive-y direction
                adj_block = getBlockAt(x, y + 1, z);
                if (y == 255 || adj_block == EMPTY || (block != WATER && adj_block == WATER)) {
                    // draw face to the positive-y face
                    for (std::pair<int, int> offset : offsets) {
                        pnc->push_back(glm::vec4(world_xz[0] + offset.first, y + 1, world_xz[1] + offset.second, 1));
                        pnc->push_back(glm::vec4(0, 1, 0, 1));
                        pnc->push_back(color_map[block]);
                        glm::vec2 uv = uv_map[std::pair<BlockType, int>(block, 0)] + glm::vec2(offset.first, offset.second) / 16.f;
                        pnc->push_back(glm::vec4(uv[0], uv[1], cos_pow_map[block], ani_flag_map[block]));
                    }

                    for (int i = 0; i < 2; i++) {
                        idx->push_back(*index);
                        idx->push_back(*index + i + 1);
                        idx->push_back(*index + i + 2);
                    }

                    *index += 4;
                }
            }
        }
    }
}

void Chunk::create() {

    countOpaque = idxOpaque.size();

    if (countOpaque > 0) {
        generateIdxOpaque();
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxOpaque);
        context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxOpaque.size() * sizeof(GLuint), idxOpaque.data(), GL_STATIC_DRAW);

        generatePNCOpaque();
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPNCOpaque);
        context->glBufferData(GL_ARRAY_BUFFER, pncOpaque.size() * sizeof(glm::vec4), pncOpaque.data(), GL_STATIC_DRAW);
    }

    countTrans = idxTrans.size();

    if (countTrans > 0) {
        generateIdxTrans();
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxTrans);
        context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxTrans.size() * sizeof(GLuint), idxTrans.data(), GL_STATIC_DRAW);

        generatePNCTrans();
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPNCTrans);
        context->glBufferData(GL_ARRAY_BUFFER, pncTrans.size() * sizeof(glm::vec4), pncTrans.data(), GL_STATIC_DRAW);
    }
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

