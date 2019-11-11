#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_geomCube(mkU<Cube>(this)), mp_worldAxes(mkU<WorldAxes>(this)),
      mp_progLambert(mkU<ShaderProgram>(this)), mp_progFlat(mkU<ShaderProgram>(this)),
      mp_terrain(mkU<Terrain>()), player()
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    mp_geomCube->destroy();
}


void MyGL::MoveMouseToCenter()
{
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of Cube
    mp_geomCube->create();
    mp_worldAxes->create();

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // Set a color with which to draw geometry since you won't have one
    // defined until you implement the Node classes.
    // This makes your geometry render green.
    mp_progLambert->setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
//    vao.bind();
    glBindVertexArray(vao);

    mp_terrain->CreateTestScene();
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    *player.camera = Camera(w, h, glm::vec3(mp_terrain->dimensions.x, mp_terrain->dimensions.y * 0.75, mp_terrain->dimensions.z),
                       glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    glm::mat4 viewproj = player.camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);

    printGLErrorLog();
}


// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{
    update();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_progFlat->setViewProjMatrix(player.camera->getViewProj());
    mp_progLambert->setViewProjMatrix(player.camera->getViewProj());

    GLDrawScene();

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setModelMatrix(glm::mat4());
    mp_progFlat->draw(*mp_worldAxes);
    glEnable(GL_DEPTH_TEST);
}

void MyGL::GLDrawScene()
{
    for(int x = 0; x < mp_terrain->dimensions.x; ++x)
    {
        for(int y = 0; y < mp_terrain->dimensions.y; ++y)
        {
            for(int z = 0; z < mp_terrain->dimensions.z; ++z)
            {
                BlockType t;
                if((t = mp_terrain->m_blocks[x][y][z]) != EMPTY)
                {
                    switch(t)
                    {
                    case DIRT:
                        mp_progLambert->setGeometryColor(glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f);
                        break;
                    case GRASS:
                        mp_progLambert->setGeometryColor(glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f);
                        break;
                    case STONE:
                        mp_progLambert->setGeometryColor(glm::vec4(0.5f));
                        break;
                    default:
                        // Other types are as of yet not defined
                        break;
                    }
                    mp_progLambert->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, y, z)));
                    mp_progLambert->draw(*mp_geomCube);
                }
            }
        }
    }
}


void MyGL::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else player.keyEventUpdate(e, this->width(), this->height());
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    player.keyEventUpdate(e, 0, 0);
}

void MyGL::mouseMoveEvent(QMouseEvent *m) {
    player.mouseEventUpdate(m);
    MoveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *m) {
    player.mouseEventUpdate(m);
}

void MyGL::mouseReleaseEvent(QMouseEvent *m) {
    player.mouseEventUpdate(m);
}
