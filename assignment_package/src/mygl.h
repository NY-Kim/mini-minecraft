#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include "camera.h"
#include <scene/terrain.h>
#include "texture.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <smartpointerhelp.h>

#include "player.h"

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    uPtr<ShaderProgram> mp_progLambert;// A shader program that uses lambertian reflection
    uPtr<ShaderProgram> mp_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    uPtr<Terrain> mp_terrain;
    uPtr<Texture> mp_texture;

    /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer timer;
    float m_time;

    // Additional variables for project
    uPtr<Player> player;
    int64_t lastUpdate;

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

protected:
    void keyPressEvent(QKeyEvent *e);

    void keyReleaseEvent(QKeyEvent *e);

    void mouseMoveEvent(QMouseEvent *m);
    void mousePressEvent(QMouseEvent *m);
    void mouseReleaseEvent(QMouseEvent *m);

private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();
};


#endif // MYGL_H
