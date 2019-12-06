#include "mygl.h"
#include "la.h"
#include "chunkloader.h"

#include <iostream>
#include <algorithm>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include <QThreadPool>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QSoundEffect>


#define MINECRAFT_TEXTURE_SLOT 0
#define POSTPROCESS_TEXTURE_SLOT 1


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_texture(mkU<Texture>(this)), m_time(0.0f),
      mp_progLambert(mkU<ShaderProgram>(this)), mp_progFlat(mkU<ShaderProgram>(this)),
      mp_onLand(mkU<PostProcessShader>(this)), mp_inWater(mkU<PostProcessShader>(this)), mp_inLava(mkU<PostProcessShader>(this)), currPostShader(nullptr),
      m_frameBuffer(-1), m_renderedTexture(-1), m_depthRenderBuffer(-1), m_geomQuad(this),
      mp_terrain(mkU<Terrain>(this)), player(mkU<Player>()), lastUpdate(QDateTime::currentMSecsSinceEpoch()),
      chunksToCreate(), mutex(mkU<QMutex>()), init(true),
      splashIn(mkU<QSoundEffect>()), waterSFX(mkU<QSoundEffect>())
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible

    // Set up sound effects
    splashIn->setSource(QUrl::fromLocalFile("../assignment_package/music/splash.wav"));
    splashIn->setVolume(0.2);
    waterSFX->setSource(QUrl::fromLocalFile("../assignment_package/music/water.wav"));
    waterSFX->setVolume(0.5);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomQuad.destroy();
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
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    //Allow for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);



    // Set up the post-processing pipeline
    createRenderBuffers();

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // Create and set up the post-processing shaders
    mp_onLand->create(":/glsl/passthrough.vert.glsl", ":glsl/noOp.frag.glsl");
    mp_inWater->create(":/glsl/passthrough.vert.glsl", ":glsl/water.frag.glsl");
    mp_inLava->create(":/glsl/passthrough.vert.glsl", ":glsl/lava.frag.glsl");
    currPostShader = mp_onLand.get();

    m_geomQuad.create();
    // Set a color with which to draw geometry since you won't have one
    // defined until you implement the Node classes.
    // This makes your geometry render green.
    mp_progLambert->setGeometryColor(glm::vec4(0,1,0,1));

    mp_texture->create(":/minecraft_textures_all.png");
    mp_texture->load(MINECRAFT_TEXTURE_SLOT);

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    //    vao.bind();
    glBindVertexArray(vao);

    mp_terrain->CreateTestScene();
    mp_terrain->create();

    // Now start the background music
    QMediaPlayer *player = new QMediaPlayer();
    QMediaPlaylist *bgm = new QMediaPlaylist();
    bgm->addMedia(QUrl::fromLocalFile("../assignment_package/music/bgm1.mp3"));
    bgm->addMedia(QUrl::fromLocalFile("../assignment_package/music/bgm2.mp3"));
    bgm->setPlaybackMode(QMediaPlaylist::Loop);
    bgm->setCurrentIndex(1);
    player->setVolume(25);
    player->setPlaylist(bgm);
    player->play();
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    *player->camera = Camera(w, h, glm::vec3(19.f, 200.f, 8.f),
                             glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    player->position = player->camera->eye - glm::vec3(0.f, 1.5, 0.f);

    glm::mat4 viewproj = player->camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);
    mp_progLambert->setCameraPosition(player->camera->eye);

    printGLErrorLog();
}

float MyGL::rayMarch(glm::vec3 ray, glm::vec3 currPos) {
    float minT;
    float currT = 0.f;
    float length = glm::length(ray);
    ray = glm::normalize(ray);
    glm::ivec3 currCell(glm::floor(currPos));

    // March to find blocks that the vertex (position: currPos) will intersect with
    // Do this for each cardinal axis respectively and use corresponding minT to scale velocity
    while (currT < length) {
        minT = std::sqrt(3);
        int axisOfIntersection = -1;
        for (int i = 0; i < 3; i++) {
            if (ray[i] != 0) {
                int signOffset = std::max(0.f, glm::sign(ray[i]));
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == ray[i] && signOffset == 0) {
                    signOffset = -1;
                }

                float axisT = (currCell[i] + signOffset - currPos[i]) / ray[i];
                if(axisT < minT && axisT > 0) {
                    minT = axisT;
                    axisOfIntersection = i;
                }
            }
        }

        // Move position to next block
        currT += minT;
        currPos += ray * minT;


        // Offset block coord by -1 along the axis we hit IF AND ONLY IF the ray's direction
        // along that axis is negative.
        float signOfDirAlongHitAxis = glm::sign(ray[axisOfIntersection]);
        glm::vec3 offset(0.f);
        offset[axisOfIntersection] = glm::min(0.f, signOfDirAlongHitAxis); // -1 if negative, 0 if positive

        // Check if there will be a collision
        glm::vec3 blockCoords = glm::floor(currPos) + offset;
        if (mp_terrain->getBlockAt(blockCoords[0], blockCoords[1], blockCoords[2]) != EMPTY) {
            if (mp_terrain->getBlockAt(blockCoords[0], blockCoords[1], blockCoords[2]) == LAVA ||
                    mp_terrain->getBlockAt(blockCoords[0], blockCoords[1], blockCoords[2]) == WATER) {
                player->inLiquid = true;
            }
            else return std::max(currT, 0.f);
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
    int64_t deltaT = QDateTime::currentMSecsSinceEpoch() - prev;

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

    if (player->spacebarPressed && (player->onGround || player->inLiquid)) {
        player->velocity[1] = 2.f;
        if (!player->inLiquid) {
            player->spacebarPressed = false;
        }
    }

    if (player->qPressed) {
        player->velocity[1] = -2.f;
    } else if (player->godMode) {
        player->velocity[1] = 0.f;
    }

    if (player->ePressed) {
        player->velocity[1] = (player->qPressed) ? 0.f : 2.f;
    }

    if (player->fPressed) {
        player->godMode = !(player->godMode);
        if (player->godMode) {
            player->velocity[1] = 0;
        }
        player->fPressed = false;
    }

    // Step 3. Iterate over all entities in scene and perform "physics update"
    // and
    // Step 4. Prevent physics entities from colliding with other physics entities
    // In order to check for collisions, we volume cast using the translation vector
    // We find the furthest possible point , then volume cast and reupdate camera if necessary
    bool inLiquidBefore = player->inLiquid;
    if (glm::length(player->velocity) > 0.f) {
        glm::vec3 trans(player->velocity[0] * deltaT / 100.f,
                player->velocity[1] * deltaT / 100.f,
                player->velocity[2] * deltaT / 100.f);

        glm::vec3 updatedPos(player->position);

        if (player->godMode) {
            updatedPos += player->camera->look * trans[0];
            updatedPos += player->camera->up * trans[1];
        } else {
            glm::vec3 grounded = glm::normalize(glm::vec3(player->camera->look[0], 0.f, player->camera->look[2]));
            updatedPos += grounded * trans[0];
            updatedPos += player->camera->world_up * trans[1];
        }
        updatedPos += player->camera->right * trans[2];

        glm::vec3 ray = updatedPos - player->position;

        // If it ain't broke, don't fix it :)
        if (glm::length(ray) == 0) {
            update();
            return;
        }

        // If in god mode, just move along the ray
        if (player->godMode) {
            player->camera->eye += ray;
            player->camera->ref += ray;
            player->position += ray;
        }
        else {
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
            minT = std::max(minT - 0.02f, 0.f);
            if (player->inLiquid) {
                minT = minT * 2.f / 3.f;

            }
            ray = glm::normalize(ray) * minT;
            player->camera->eye += ray;
            player->camera->ref += ray;
            player->position += ray;
        }
    }

    // Gravity only affects player if not in god mode or not on ground
    glm::ivec3 currPos(glm::floor(player->position));

    bool bottomInLiquid = (mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2]) == LAVA ||
            mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2]) == WATER) ||
            (mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2]) == LAVA ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2]) == WATER) ||
            (mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2] - 1) == LAVA ||
            mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2] - 1) == WATER) ||
            (mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2] - 1) == LAVA ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2] - 1) == WATER);
    bool topInLiquid = (mp_terrain->getBlockAt(currPos[0], currPos[1], currPos[2]) == LAVA ||
            mp_terrain->getBlockAt(currPos[0], currPos[1], currPos[2]) == WATER) ||
            (mp_terrain->getBlockAt(currPos[0] + 1, currPos[1], currPos[2]) == LAVA ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1], currPos[2]) == WATER) ||
            (mp_terrain->getBlockAt(currPos[0], currPos[1], currPos[2] - 1) == LAVA ||
            mp_terrain->getBlockAt(currPos[0], currPos[1], currPos[2] - 1) == WATER) ||
            (mp_terrain->getBlockAt(currPos[0] + 1, currPos[1], currPos[2] - 1) == LAVA ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1], currPos[2] - 1) == WATER);

    player->inLiquid = bottomInLiquid || topInLiquid;

    if (inLiquidBefore != player->inLiquid) {
        splashIn->play();
    }
    player->onGround =
            (mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2]) != EMPTY ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2]) != EMPTY ||
            mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2] - 1) != EMPTY ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2] - 1) != EMPTY) &&
            (mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2]) != WATER ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2]) != WATER ||
            mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2] - 1) != WATER ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2] - 1) != WATER) &&
            (mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2]) != LAVA ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2]) != LAVA ||
            mp_terrain->getBlockAt(currPos[0], currPos[1] - 1.f, currPos[2] - 1) != LAVA ||
            mp_terrain->getBlockAt(currPos[0] + 1, currPos[1] - 1.f, currPos[2] - 1) != LAVA);

    player->velocity[1] = (player->godMode || player->onGround) ? 0.f : std::max(player->velocity[1] - (9.8 * deltaT / 1000.f), -4.0);

    // Step 5. Process all renderable entities and draw them

    std::vector<int> regenCase = mp_terrain->checkRegenerate(player->position);
    if (regenCase.size() != 0) {
        // Initial load-in
        if (init) {
            mp_terrain->destroy();
            mp_terrain->create();
            init = false;
        }

        else {
            for (int dir : regenCase) {
                // a) Create 4x4 chunks and set its neighbors
                glm::ivec2 currOrigin = mp_terrain->terrOrigin(player->position);
                glm::ivec2 chunkOrigin = getNewOrigin(currOrigin, dir);
                if (mp_terrain->m_chunks.find(std::pair<int, int>(chunkOrigin[0], chunkOrigin[1])) != mp_terrain->m_chunks.end()) {
                    continue;
                }

                std::vector<Chunk*> threadChunks;
                for (int x = 0; x < 4; ++x) {
                    for (int z = 0; z < 4; ++z) {
                        uPtr<Chunk> chunk = mkU<Chunk>(this, chunkOrigin + glm::ivec2(x * 16, z * 16));
                        Chunk* currChunk = chunk.get();
                        mp_terrain->m_chunks[std::pair<int, int>(currChunk->position[0], currChunk->position[1])] = std::move(chunk);
                        mp_terrain->setNeighbors(currChunk);
                        threadChunks.push_back(currChunk);
                    }
                }

                // b) Make worker to handle the chunks and start it
                QString name("Worker ");
                name.append(QString::number(dir));
                ChunkLoader* worker = new ChunkLoader(dir, threadChunks, &chunksToCreate, name, mutex.get());
                QThreadPool::globalInstance()->start(worker);

                // c) Lock mutex, create all the chunks that have been processed so far, then clear the vector and unlock

                mutex->lock();
                if (chunksToCreate.size() > 0) {
                    for (Chunk* c : chunksToCreate) {
                        c->create();
                    }
                    chunksToCreate.clear();
                }
                mutex->unlock();


            }
            // If there are any remaining non-created chunks, lock mutex, create all the chunks, then clear the vector and unlock
            mutex->lock();
            if (chunksToCreate.size() > 0) {
                for (Chunk* c : chunksToCreate) {
                    c->create();
                }
                chunksToCreate.clear();
            }
            mutex->unlock();
        }
    }

    // Check which post-process buffer to use
    glm::ivec3 camPos(glm::floor(player->camera->eye));
    if (mp_terrain->getBlockAt(camPos[0], camPos[1], camPos[2]) == LAVA) {
        currPostShader = mp_inLava.get();
    } else if (mp_terrain->getBlockAt(camPos[0], camPos[1], camPos[2]) == WATER) {
        currPostShader = mp_inWater.get();
    } else currPostShader = mp_onLand.get();
    update();

    // Check if water is nearby, if so play

    for (int x = -10; x <= 10; ++x) {
        if (waterSFX->isPlaying()) {
            break;
        }
        for (int y = -10; y <= 10; y++) {
            if (waterSFX->isPlaying()) {
                break;
            }
            for (int z = -10; z <= 10; z++) {
                if (mp_terrain->getBlockAt(camPos[0], camPos[1], camPos[2]) == WATER) {
                    if (!waterSFX->isPlaying()) {
                        waterSFX->play();
                    }
                    break;
                }
            }
        }
    }


    // Potential fix for deltaT, don't update lastUpdate counter until finished
    lastUpdate = QDateTime::currentMSecsSinceEpoch();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Render the 3D scene to our frame buffer

    // Render to our framebuffer rather than the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_progFlat->setViewProjMatrix(player->camera->getViewProj());
    mp_progLambert->setViewProjMatrix(player->camera->getViewProj());
    mp_progLambert->setTime(m_time);
    currPostShader->setTime(m_time);
    m_time++;

    currPostShader->setDimensions(glm::ivec2(this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio()));

    GLDrawScene();


    glDisable(GL_DEPTH_TEST);
    performPostprocessRenderPass();
    glEnable(GL_DEPTH_TEST);
}

void MyGL::GLDrawScene()
{
    mp_texture->bind(MINECRAFT_TEXTURE_SLOT);
    for (const auto& map : mp_terrain->m_chunks) {
        Chunk* cPtr = map.second.get();
        if (std::find(chunksToCreate.begin(), chunksToCreate.end(), cPtr) == chunksToCreate.end()) {
            mp_progLambert->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0)));
            mp_progLambert->drawOpaque(*cPtr);
        }
    }

    for (const auto& map : mp_terrain->m_chunks) {
        Chunk* cPtr = map.second.get();
        if (std::find(chunksToCreate.begin(), chunksToCreate.end(), cPtr) == chunksToCreate.end()) {
            mp_progLambert->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0)));
            mp_progLambert->drawTrans(*cPtr);
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
    if (m->button() == Qt::LeftButton) {
        mp_terrain->deleteBlock(player->camera->eye, player->camera->look);
        mp_terrain->destroy();
        mp_terrain->create();
        update();
    } else if (m->button() == Qt::RightButton) {
        mp_terrain->addBlock(player->camera->eye, player->camera->look, LAVA);
        mp_terrain->destroy();
        mp_terrain->create();
        update();
    } else if (m->button() == Qt::MiddleButton) {
        mp_terrain->addBlock(player->camera->eye, player->camera->look, WATER);
        mp_terrain->destroy();
        mp_terrain->create();
        update();
    }
}

void MyGL::mouseReleaseEvent(QMouseEvent *m) {
    player->mouseEventUpdate(m);
}

void MyGL::createRenderBuffers()
{
    // Initialize the frame buffers and render textures
    glGenFramebuffers(1, &m_frameBuffer);
    glGenTextures(1, &m_renderedTexture);
    glGenRenderbuffers(1, &m_depthRenderBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);

    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Clamp the colors at the edge of our texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Initialize our depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

    // Set m_renderedTexture as the color output of our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_renderedTexture, 0);

    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0, which we previously set to m_renderedTextures[i]
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        printGLErrorLog();
    }
}

void MyGL::performPostprocessRenderPass()
{
    // Render the frame buffer as a texture on a screen-size quad

    // Tell OpenGL to render to the viewport's frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0 + POSTPROCESS_TEXTURE_SLOT);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);

    currPostShader->draw(m_geomQuad, POSTPROCESS_TEXTURE_SLOT);
}

glm::ivec2 MyGL::getNewOrigin(glm::ivec2 curr, int regenCase) {
    int xOffset = (regenCase == 2 || regenCase == 3 || regenCase == 4) ? 64 :
                                                                         (regenCase == 6 || regenCase == 7 || regenCase == 8) ? -64 : 0;
    int zOffset = (regenCase == 1 || regenCase == 2 || regenCase == 8) ? 64 :
                                                                         (regenCase == 4 || regenCase == 5 || regenCase == 6) ? -64 : 0;

    return curr + glm::ivec2(xOffset, zOffset);
}
