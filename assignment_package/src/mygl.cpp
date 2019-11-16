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
    *player->camera = Camera(w, h, glm::vec3(19.f, 137.f, 8.f),
                            glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    player->position = player->camera->eye - glm::vec3(0.f, 1.5, 0.f);

    glm::mat4 viewproj = player->camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);

    printGLErrorLog();
}

float MyGL::rayMarch(glm::vec3 ray, glm::vec3 currPos) {
    float minT;
    float currT = 0.f;
    float length = glm::length(ray);
    glm::ivec3 currCell(currPos);

    // March to find blocks that the vertex (position: currPos) will intersect with
    while (currT < length) {
        minT = std::sqrt(3);
        int axisOfIntersection = -1;
        for (int i = 0; i < 3; i++) {
            if (ray[i] != 0) {
                int signOffset = std::max(0.f, glm::sign(ray[i]));
                float axisT = (currCell[i] + signOffset - currPos[i]) / ray[i];
                if(axisT < minT && axisT > 0) {
                    minT = axisT;
                    axisOfIntersection = i;
                }
            }
        }

        if (axisOfIntersection == -1) {
            return 0.f;
        }

        // Move position to next block
        currPos += ray * minT;
        currT += minT;

        // Offset block coord by -1 along the axis we hit IF AND ONLY IF the ray's direction
        // along that axis is negative.
        float signOfDirAlongHitAxis = glm::sign(ray[axisOfIntersection]);
        glm::vec3 offset(0.f);
        offset[axisOfIntersection] = glm::min(0.f, signOfDirAlongHitAxis); // -1 if negative, 0 if positive

        // Check if there will be a collision
        glm::vec3 blockCoords = glm::floor(currPos) + offset;
        if (mp_terrain->getBlockAt(blockCoords[0], blockCoords[1], blockCoords[2]) != EMPTY) {
            return std::max(currT - 0.01f, 0.f);
        }
        currCell = blockCoords;
    }
    return length;
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
        player->velocity[0] = 2.f;
    } else {
        player->velocity[0] = 0.f;
    }
    if (std::get<1>(player->wasdPressed)) {
        player->velocity[2] = -2.f;
    } else {
        player->velocity[2] = 0.f;
    }
    if (std::get<2>(player->wasdPressed)) {
        player->velocity[0] = (player->velocity[2] == 2.f) ? 0.f : -2.f;
    }
    if (std::get<3>(player->wasdPressed)) {
        player->velocity[2] = (player->velocity[0] == -2.f) ? 0.f : 2.f;
    }

    if (player->mouseMoved) {
        player->camera->RotatePolar();
        player->mouseMoved = false;
    }

    if (player->spacebarPressed) {
        player->velocity[1] = 2.f;
        player->spacebarPressed = false;
    }

    if (player->qPressed) {
        player->velocity[1] = -2.f;
    }
    if (player->ePressed) {
        player->velocity[1] = (player->qPressed) ? 0.f : 2.f;
    }

    // Step 3. Iterate over all entities in scene and perform "physics update"
    // and
    // Step 4. Prevent physics entities from colliding with other physics entities
    // In order to check for collisions, we volume cast using the translation vector
    // We find the furthest possible point , then volume cast and reupdate camera if necessary
    if (player->velocity[0] != 0.f || player->velocity[1] != 0.f || player->velocity[2] != 0.f) {
        glm::vec3 trans(player->velocity[0] * deltaT / 100.f,
                        player->velocity[1] * deltaT / 100.f,
                        player->velocity[2] * deltaT / 100.f);

        glm::vec3 updatedPos(player->position);

        if (player->godMode) {
            updatedPos += player->camera->look * trans[0];
        } else {
            glm::vec3 grounded = glm::normalize(glm::vec3(player->camera->look[0], 0.f, player->camera->look[2]));
            updatedPos += grounded * trans[0];
        }

        updatedPos += (player->godMode) ? player->camera->up * trans[1] : player->camera->world_up * trans[1];
        updatedPos += player->camera->right * trans[2];

        glm::vec3 ray = updatedPos - player->position;
        glm::vec3 bottomLeftVertex = player->position - glm::vec3(0.5, 0.f, 0.f);

        float minT = glm::length(ray);
        for (int x = 0; x <= 1; ++x) {
            for (int y = 0; y <= 2; ++y) {
                for (int z = 0; z >= -1; --z) {
                    glm::vec3 currVertPos = bottomLeftVertex + glm::vec3(x, y, z);
                    minT = std::min(minT, rayMarch(ray, currVertPos));
                }
            }
        }
        ray *= (minT / glm::length(ray));
        if (std::isnan(ray[0])) {
            ray = glm::vec3(0.f);
        }
        player->camera->eye += ray;
        player->camera->ref += ray;
        player->position = player->camera->eye - glm::vec3(0.f, 1.5, 0.f);
        std::cout << glm::to_string(player->position) << std::endl;
    }

    // Gravity only affects player if not in god mode
    player->velocity[1] = (player->godMode) ? 0 : std::max(player->velocity[1] - (9.8 * deltaT / 10000.f), -2.0);

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
