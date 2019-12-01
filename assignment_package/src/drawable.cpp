#include "drawable.h"
#include <la.h>
#include <iostream>

Drawable::Drawable(OpenGLContext* context)
    : bufIdx(), bufIdxTrans(), bufPos(), bufNor(), bufCol(), bufPNC(), bufPNCTrans(),
      idxBound(false), idxTransBound(false),
      posBound(false), norBound(false), colBound(false), pncBound(false), pncTransPound(false),
      context(context)
{}

Drawable::~Drawable()
{}

void Drawable::destroy()
{
    context->glDeleteBuffers(1, &bufIdx);
    context->glDeleteBuffers(1, &bufPNC);
    context->glDeleteBuffers(1, &bufIdxTrans);
    context->glDeleteBuffers(1, &bufPNCTrans);
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return count;
}

void Drawable::generateIdx()
{
    idxBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdx);
}

void Drawable::generateIdxTrans()
{
    idxTransBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdxTrans);
}

void Drawable::generatePos()
{
    posBound = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufPos);
}

void Drawable::generateNor()
{
    norBound = true;
    // Create a VBO on our GPU and store its handle in bufNor
    context->glGenBuffers(1, &bufNor);
}

void Drawable::generateCol()
{
    colBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufCol);
}

void Drawable::generatePNC()
{
    pncBound = true;
    // Create a VBO on our GPU and store its handle in bufPNC
    context->glGenBuffers(1, &bufPNC);
}

void Drawable::generatePNCTrans()
{
    pncTransPound = true;
    // Create a VBO on our GPU and store its handle in bufPNC
    context->glGenBuffers(1, &bufPNCTrans);
}

bool Drawable::bindIdx()
{
    if(idxBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    }
    return idxBound;
}

bool Drawable::bindIdxTrans()
{
    if(idxTransBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxTrans);
    }
    return idxTransBound;
}

bool Drawable::bindPos()
{
    if(posBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    }
    return posBound;
}

bool Drawable::bindNor()
{
    if(norBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    }
    return norBound;
}

bool Drawable::bindCol()
{
    if(colBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    }
    return colBound;
}

bool Drawable::bindPNC()
{
    if(pncBound) {
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPNC);
    }
    return pncBound;
}

bool Drawable::bindPNCTrans()
{
    if(pncTransPound) {
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPNCTrans);
    }
    return pncTransPound;
}
