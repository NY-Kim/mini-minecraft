#pragma once

#include <openglcontext.h>
#include <la.h>

//This defines a class which can be rendered by our shader program.
//Make any geometry a subclass of ShaderProgram::Drawable in order to render it with the ShaderProgram class.
class Drawable
{
protected:
    int countOpaque;     // The number of indices stored in bufIdx.
    int countTrans;

    GLuint bufIdxOpaque; // A Vertex Buffer Object that we will use to store triangle indices (GLuints)
    GLuint bufIdxTrans;
    GLuint bufPNCOpaque;
    GLuint bufPNCTrans;

    bool idxOpaqueBound; // Set to TRUE by generateIdx(), returned by bindIdx().
    bool idxTransBound;
    bool pncOpaqueBound;
    bool pncTransBound;



public:
    OpenGLContext* context; // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                          // we need to pass our OpenGL context to the Drawable in order to call GL functions
                          // from within this class.

    Drawable(OpenGLContext* context);
    virtual ~Drawable();

    virtual void create() = 0; // To be implemented by subclasses. Populates the VBOs of the Drawable.
    virtual void destroy(); // Frees the VBOs of the Drawable.

    // Getter functions for various GL data
    virtual GLenum drawMode();
    int elemCountOpaque();
    int elemCountTrans();

    // Call these functions when you want to call glGenBuffers on the buffers stored in the Drawable
    // These will properly set the values of idxBound etc. which need to be checked in ShaderProgram::draw()
    void generateIdxOpaque();
    void generateIdxTrans();
    void generatePNCOpaque();
    void generatePNCTrans();

    bool bindIdxOpaque();
    bool bindIdxTrans();
    bool bindPNCOpaque();
    bool bindPNCTrans();

};
