#ifndef INVENTORY_H
#define INVENTORY_H

#include <drawable.h>
#include <la.h>
#include <scene/terrain.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Inventory : public Drawable
{
public:
    Inventory(OpenGLContext* context);
    virtual void create();
    virtual GLenum drawMode();

    bool drawn;
    std::map<BlockType, int> block_map;
    BlockType selected_type;
    void printInventory();
    void selectRight();
    void selectLeft();
};

#endif // INVENTORY_H
