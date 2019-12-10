#include <scene/inventory.h>
#include <array>

Inventory::Inventory(OpenGLContext *context)
    : Drawable(context), drawn(false), block_map(std::map<BlockType, int>()), selected_type(GRASS)
{
    block_map[GRASS] = 0;
    block_map[DIRT] = 1;
    block_map[STONE] = 2;
    block_map[SAND] = 3;
    block_map[DARK] = 4;
    block_map[IRON] = 5;
    block_map[COAL] = 6;
    block_map[ORANGE] = 7;
}

void Inventory::printInventory() {
    std::cout << "selected: " << block_map[selected_type] << std::endl;
}

void Inventory::create() {
    int i = block_map[selected_type];
    std::vector<GLuint> idx = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
    std::vector<glm::vec4> pos = {glm::vec4(-0.6f, -0.9f, 0.98, 1.f),
                                  glm::vec4(0.6f, -0.9f, 0.98, 1.f),
                                  glm::vec4(0.6f, -0.7f, 0.98, 1.f),
                                  glm::vec4(-0.6f, -0.7f, 0.98, 1.f),
                                  // selection indicator
                                  glm::vec4(-0.6f + 0.15f * i, -0.9f, 0.99, 1.f),
                                  glm::vec4(-0.45f + 0.15f * i, -0.9f, 0.99, 1.f),
                                  glm::vec4(-0.45f + 0.15f * i, -0.7f, 0.99, 1.f),
                                  glm::vec4(-0.6f + 0.15f * i, -0.7f, 0.99, 1.f)};
    std::vector<glm::vec2> uv = {glm::vec2(5.f / 16.f, 2.f / 16.f),
                                 glm::vec2(13.f / 16.f, 2.f / 16.f),
                                 glm::vec2(13.f / 16.f, 3.f / 16.f),
                                 glm::vec2(5.f / 16.f, 3.f / 16.f),
                                 // selection indicator
                                 glm::vec2(0.f / 16.f, 0.f / 16.f),
                                 glm::vec2(1.f / 16.f, 0.f / 16.f),
                                 glm::vec2(1.f / 16.f, 1.f / 16.f),
                                 glm::vec2(0.f / 16.f, 1.f / 16.f)};

    countOpaque = idx.size();

    generateIdxOpaque();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxOpaque);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateUV();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    context->glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), uv.data(), GL_STATIC_DRAW);
}

void Inventory::selectRight() {
    int i = block_map[selected_type];
    if (i < 7) {
        for (auto const& x: block_map) {
            if (x.second == i + 1) {
                selected_type = x.first;
                return;
            }
        }
    }
}

void Inventory::selectLeft() {
    int i = block_map[selected_type];
    if (i > 0) {
        for (auto const& x: block_map) {
            if (x.second == i - 1) {
                selected_type = x.first;
                return;
            }
        }
    }
}

GLenum Inventory::drawMode() {
    return GL_TRIANGLES;
}
