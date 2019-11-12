#include "mygl.h"
#include "la.h"
#include "scene/cube.h"

#include <iostream>
#include <algorithm>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_geomCube(mkU<Cube>(this)), mp_worldAxes(mkU<WorldAxes>(this)),
      mp_progLambert(mkU<ShaderProgram>(this)), mp_progFlat(mkU<ShaderProgram>(this)),
      mp_terrain(mkU<Terrain>()), player(mkU<Player>()), lastUpdate(QDateTime::currentMSecsSinceEpoch())
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
    *player->camera = Camera(w, h, glm::vec3(mp_terrain->dimensions.x, mp_terrain->dimensions.y * 0.75, mp_terrain->dimensions.z),
                            glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    player->position = player->camera->eye - glm::vec3(0.f, 1.5, 0.f);

    glm::mat4 viewproj = player->camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);

    printGLErrorLog();
}


// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{
    // Step 1. Computer time elapsed since last update call
    int64_t prev = lastUpdate;
    lastUpdate = QDateTime::currentMSecsSinceEpoch();
    int64_t deltaT = lastUpdate - prev;

    // Step 2. Iterate over all entities that are capable of receiving input
    // and read their present controller state
    if (std::get<0>(player->wasdPressed)) {
        player->velocity[2] = 2.f;
    } else {
        player->velocity[2] = 0.f;
    }
    if (std::get<1>(player->wasdPressed)) {
        player->velocity[0] = -2.f;
    } else {
        player->velocity[0] = 0.f;
    }
    if (std::get<2>(player->wasdPressed)) {
        player->velocity[2] = (player->velocity[2] == 2.f) ? 0.f : -2.f;
    }
    if (std::get<3>(player->wasdPressed)) {
        player->velocity[0] = (player->velocity[0] == -2.f) ? 0.f : 2.f;
    }

    if (player->spacebarPressed) {
        player->velocity[1] = 2.f;
        player->spacebarPressed = false;
    }

    if (player->cursorXYChange.x() != 0.f) {
        player->camera->RotateAboutUp(player->cursorXYChange.x());
        player->cursorXYChange.setX(0);
    }
    if (player->cursorXYChange.y() != 0.f) {
        player->camera->RotateAboutRight(player->cursorXYChange.y());
        player->cursorXYChange.setY(0);
    }

    // Step 3. Iterate over all entities in scene and perform "physics update"
    // and
    // Step 4. Prevent physics entities from colliding with other physics entities
    // In order to check for collisions, we volume cast using the translation vector
    // We first change position based on the vector then move it back if needed before updating camera
    glm::vec3 trans(player->velocity[0] * deltaT / 1000.f,
                    player->velocity[1] * deltaT / 1000.f,
                    player->velocity[2] * deltaT / 1000.f);
    glm::vec3 updatedPos(player->position + trans);

    for (int xPos = floor(player->position[0]); xPos < ceil(updatedPos[0]); xPos++) {
        if (mp_terrain->getBlockAt(xPos, floor(player->position[1]), floor(player->position[2])) != EMPTY) {
            updatedPos[0] = xPos - 0.01;
            trans[0] = updatedPos[0] - player->position[0];
            break;
        }
    }
    for (int yPos = floor(player->position[1]); yPos < ceil(updatedPos[1]); yPos++) {
        if (mp_terrain->getBlockAt(floor(player->position[0]), yPos, floor(player->position[2])) != EMPTY) {
            updatedPos[1] = yPos - 0.01;
            trans[1] = updatedPos[1] - player->position[1];
            break;
        }
    }
    for (int zPos = floor(player->position[2]); zPos < ceil(updatedPos[2]); zPos++) {
        if (mp_terrain->getBlockAt(floor(player->position[0]), floor(player->position[1]), zPos) != EMPTY) {
            updatedPos[2] = zPos - 0.01;
            trans[2] = updatedPos[2] - player->position[2];
            break;
        }
    }

    player->position = updatedPos;
    player->camera->TranslateAlongRight(trans[0]);
    player->camera->TranslateAlongUp(trans[1]);
    player->camera->TranslateAlongLook(trans[2]);
    player->camera->RecomputeAttributes();

    // Gravity only affects player if not in god mode
    player->velocity[1] = (player->godMode) ? 0 : player->velocity[1] - (9.8 * deltaT / 1000.f);

    // Step 5. Process all renderable entities and draw them
    update();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_progFlat->setViewProjMatrix(player->camera->getViewProj());
    mp_progLambert->setViewProjMatrix(player->camera->getViewProj());

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
    } else player->keyEventUpdate(e);
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    player->keyEventUpdate(e);
}

void MyGL::mouseMoveEvent(QMouseEvent *m) {
    player->mouseEventUpdate(m);
    MoveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *m) {
    player->mouseEventUpdate(m);
}

void MyGL::mouseReleaseEvent(QMouseEvent *m) {
    player->mouseEventUpdate(m);
}
