#include "drawable.h"
#include <la.h>
#include <iostream>

Drawable::Drawable(OpenGLContext* context)
    : countOpaque(0), countTrans(0), bufIdxOpaque(), bufIdxTrans(), bufPNCOpaque(), bufPNCTrans(), bufPos(), bufUV(),
      idxOpaqueBound(false), idxTransBound(false),
      pncOpaqueBound(false), pncTransBound(false),
      posBound(false), uvBound(false),

      context(context)
{}

Drawable::~Drawable()
{}

void Drawable::destroy()
{
    context->glDeleteBuffers(1, &bufIdxOpaque);
    context->glDeleteBuffers(1, &bufPNCOpaque);
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

int Drawable::elemCountOpaque()
{
    return countOpaque;
}

int Drawable::elemCountTrans()
{
    return countTrans;
}

void Drawable::generateIdxOpaque()
{
    idxOpaqueBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdxOpaque);
}

void Drawable::generateIdxTrans()
{
    idxTransBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdxTrans);
}

void Drawable::generatePNCOpaque()
{
    pncOpaqueBound = true;
    // Create a VBO on our GPU and store its handle in bufPNC
    context->glGenBuffers(1, &bufPNCOpaque);
}


void Drawable::generatePNCTrans()
{
    pncTransBound = true;
    // Create a VBO on our GPU and store its handle in bufPNC
    context->glGenBuffers(1, &bufPNCTrans);
}

bool Drawable::bindIdxOpaque()
{
    if(bufIdxOpaque) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxOpaque);
    }
    return idxOpaqueBound;
}

bool Drawable::bindIdxTrans()
{
    if(idxTransBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxTrans);
    }
    return idxTransBound;
}

bool Drawable::bindPNCOpaque()
{
    if(pncOpaqueBound) {
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPNCOpaque);
    }
    return pncOpaqueBound;
}

bool Drawable::bindPNCTrans()
{
    if(pncTransBound) {
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPNCTrans);
    }
    return pncTransBound;
}

void Drawable::generatePos()
{
    posBound = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufPos);
}

void Drawable::generateUV()
{
    uvBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufUV);
}

bool Drawable::bindPos()
{
    if(posBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    }
    return posBound;
}

bool Drawable::bindUV()
{
    if(uvBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    }
    return uvBound;
}
