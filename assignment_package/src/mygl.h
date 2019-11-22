#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include "postprocessshader.h"
#include <scene/cube.h>
#include <scene/worldaxes.h>
#include "camera.h"
#include <scene/terrain.h>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <smartpointerhelp.h>

#include "player.h"
#include "scene/quad.h"

#include <QMutex>

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    uPtr<Cube> mp_geomCube;// The instance of a unit cube we can use to render any cube. Should NOT be used in final version of your project.
    uPtr<WorldAxes> mp_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    uPtr<ShaderProgram> mp_progLambert;// A shader program that uses lambertian reflection
    uPtr<ShaderProgram> mp_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    uPtr<PostProcessShader> mp_onLand;
    uPtr<PostProcessShader> mp_inWater;
    uPtr<PostProcessShader> mp_inLava;
    PostProcessShader* currPostShader;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    /// Required for post-processing
    // A collection of handles to the five frame buffers we've given
    // ourselves to perform render passes. The 0th frame buffer is always
    // written to by the render pass that uses the currently bound surface shader.
    GLuint m_frameBuffer;
    // A collection of handles to the textures used by the frame buffers.
    // m_frameBuffers[i] writes to m_renderedTextures[i].
    GLuint m_renderedTexture;
    // A collection of handles to the depth buffers used by our frame buffers.
    // m_frameBuffers[i] writes to m_depthRenderBuffers[i].
    GLuint m_depthRenderBuffer;
    // Screen space quadrangle for post-processing (i.e. swimming effect)
    Quad m_geomQuad;


    uPtr<Terrain> mp_terrain;

    /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer timer;

    // Additional variables for project
    uPtr<Player> player;
    int64_t lastUpdate;
    std::vector<uPtr<Chunk>> chunksToCreate;
    uPtr<QMutex> mutex;
    bool init;

    void MoveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.


public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void GLDrawScene();

    float rayMarch(glm::vec3 ray, glm::vec3 currPos);
    glm::ivec2 getNewOrigin(glm::ivec2 curr, int regenCase);
    void setNeighbors(Chunk* chunk, glm::ivec2 key);

protected:
    void keyPressEvent(QKeyEvent *e);

    void keyReleaseEvent(QKeyEvent *e);

    void mouseMoveEvent(QMouseEvent *m);
    void mousePressEvent(QMouseEvent *m);
    void mouseReleaseEvent(QMouseEvent *m);

private:
    // Sets up the arrays of frame buffers
    // used to store render passes. Invoked
    // once in initializeGL().
    void createRenderBuffers();

    // A helper function that iterates through
    // each of the render passes required by the
    // currently bound post-process shader and
    // invokes them.
    void performPostprocessRenderPass();

private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();
};


#endif // MYGL_H
