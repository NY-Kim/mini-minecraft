# Milestone 3

## Biomes - Ray DongHo Kim

**Terrain class:**

- In order to assign biome type in random but smooth fashion, I used FBM noise function. For each coordinate, I get "moist" and "bumpiness" values, which decides the biome type among 4 types: Canyon, Grassland, Snowland, Mustafar(lava land). After deciding the biome type, I use 4 functions that returns height of that coordinate, if the coordinate was in certain biome. Each function represents one biome. Those height generating functions use FBM and smoothstep to create unique and separate looks from each other. For each point, I get 4 height values, then interpolate(using smoothstep) them to get the overall height value of that coordinate.
- Created more block types: SNOW, COAL, IRON, ORANGE, BROWN, IVORY, SAND, DARK to use in different biomes, so each of them gives unique, different looks.
- Same logic used in chunkLoader, which is used for generating terrain, using multithread.

**Difficulties:** 
- First, I was trying to use Perlin-Noise function to get moist-bumpiness value. But it was keep giving me values, not ranging [-1, 1]. Eventually, I had to use FBM instead, to generate moist-bumpiness value for each coordinate.
- Properly using fbm, smoothstep to give desired look for each biome was hard. I had to calculate and try to get the right formula for the looks of the biomes.

## Things to point out
- I commented-out the river generation part from createTestScene(), because Biome Scene looks better without the rivers and the carvings. If you run the current code, it will show you the terrain with different biomes. TAs told me to just comment that part out, leave it in the code, and write it in ReadMe.
- To test/check the rivers
    1) in mygl.cpp->initializeGL( )->use createRiverScene( ) at line 17 and comment-out createTestScene at line 130
    2) in chunkloader.cpp->run( )->comment-out line 14-46, and use code from line 49-65.

## Post-Process Shader and Sound - Alexander Do

For the post-process shader, I added onto the pipeline I implemented for milestone two. Specifically, I used worley noise to modify the view of the player when in lava, and I used perlin noise to modify the view of the player when in water. Both post-process shaders use the output of the noise multiplied by a random time-dependant factor to displace the vertex coordinates. Time is also used to modfify the overlay color when in lava to simulate heat, whereas in water the LERP is by a constant offset.

For sound, I have two sounds for background music that play on loop. Ambient noises (bird calls and wind) are played at random throughout the playtime. When a bird sound is played, it is chosen at random from one of two possible sounds. When within a radius of 10 blocks, water will make a rushing noise, and lava will make a rushing noise as well as pop at random. Footsteps play when moving, and the sound of the footstep is chosen at random from a bank of three sounds. When a player enters or exits water, a splashing noise plays.

**Difficulties:**
The primary difficult came with figuring out how to use the sound APIs provided in QT. It was difficult finding the right directory to play the sounds from, and I ended up just pushing my sound files to Github. Other than that, I spent time trying to modify my noise functions to look somewhat nice. 


## Things to point out
- I fixed some of the code from my previous milestones (in particular, movement and swimming). I also fixed multithreading to work on merge, as well as to generate instaneously.
- For some reason, QSoundEffect does not work on Mac, but it does on Windows. Please see my branch (Alex_MS3) for working sound effects.
- If the sound files do not load on my personal branch, replace the strings for the directories with the permanent links from https://github.com/acdo/Mini-Minecraft-Sounds (e.g. replace QUrl::fromLocalFile("../assignment_package/music/bgm1.mp3") with QUrl("https://github.com/acdo/Mini-Minecraft-Sounds/raw/ee431e10817a4384afedfeea6422703cc5bc9971/bgm1.mp3"). 


# Milestone 2

## L-System Rivers - Ray DongHo Kim

**LSystem class:**
- For creating rivers(linear river and delta river), I used l-system, writing grammar for how the axiom should expand. In order to create two types of rivers, a member variable "mode" kept which type of river is being created now, and that determined which expansion rules to use.
- Also, random number for orientaion was generated to give randomness to how the rivers expand and branch out.
- When the river gets created, it check if that chunk exists already, and if it does not, 64x64 blocks(terrain) gets created first using fbm function to keep the smooth transition between terrains(4x4(total 16) chunks). After creating the terrain, then it proceeds to create the river, so the river does not get cut off by new terrain later. To avoid that, terrain's existence had to be checked first, then create river on top of it.
- This checking was also done when carving the terrain near the river. In the process of carving out the terrain, if the next block to be carved out is lying on the chunk that is not created yet, then the terrain, including that chunk, gets created, then proceeds to carve out the terrain, so the carved out terrain is smooth in terms of the transition between the terrains.
**Turtle class:**
- Turtle class kept information of orientation, recursion depth, position, and river width. This information was used to create each stream of the rivers.

**Difficulties:** 
- First, I was unsure how to make rivers that is continuous, even though it goes over from already-created terrain to coordinates where nothing exists. Initially river was being created, without the terrain around that area, and when the camera gets closer, terrain was regenerated on top of it, causing the river to be cut off.
- After solving that issue, same thing happened in the process of carving out. Sometimes carving was stopped only until the already-generated terrain. And when new neighboring terrain was generated afterwards, there were obvious spikes, because part of the mountain in already-generated side was carved, while the other part was just generated now, without the influence of carving out.

## Things to point out
- After our final merge, our terrain loses texture and gets toned to a blue color. However, if we comment out either post-processing part or the lambert shading part, it works fine. Branch named "lsystemWithTexture" is a version with lsystem and texturing merged, which works fine, just without the multi-thread. To check river generation and terrain generation, please check that version(lsystemWithTexture), which is easier visually.

## Texturing and Texture Animation

For texturing, I modified my interleaving VBO to include uv, cosine power, and animation flag, along with position, normal, and color. So in Terrain.cpp, I compute the UV, and set the cosine power and animation flag depending on the type of block.

To generate animation, I created a new variable in MyGL called u_Time, which increments whenever paintGL is called. Depending on the u_Time variable, I change the UV by either shifting it to the right or up. The variation was made to make all the animation in the side of the blocks look like its falling down. 

In the Lambert shader, I modified it to a Blinn-Phong shader by referencing my code from HW 4, 5. Then I added the animation part at the end.

**Difficulties:**
At first, it took a while to come up with how to do the animation since 5 blocks were given accross 2 rows in the texture map. I ended up only using 4 blocks (2 column blocks when I shift down, and 2 row blocks when I shift to the right).


## Things to point out
- Once we merge, our terrain loses texture and gets toned to a blue color. However, if we comment out either post-processing part or the lambert shading part, it works fine.


## Swimming and Multithreaded Terrain Generation - Alexander Do

For swimming, I created a inLiquid boolean variable that keeps track of if the play is inside of a WATER or LAVA block. If the boolean is true, then the physics are updated accordingly. Overlays are done by using the post-processing shader method detailed in HW04/05. We first render the terrain and map it to a texture which is then passed into the post-processing pipeline. The texture information is passed directly through the vertex shader to either a NoOP fragment shader if the player's "eyes" are not in liquid, or a lava or water fragment shader which creates the overlay effect depending on if the player's "eyes" are in lava or water respectively. For the overlay, we simply LERP the texture color with either red or blue.

For multithreading, I created a member variable for a mutex in MyGL. Whenever the player reaches a certain distance (500 units) from a border, threads are created to generate the FBM and VBO data for each necessary chunk. For these threads, I created a new class called "ChunkLoader" which inherits QRunnable and performs the necessary operations. Inside MyGL, there is a vector for chunks to be created whose memory address is given to the thread. These threads are started using QThreadPool::globalInstance()->start(). The mutex is locked either when a thread is adding the preprocessed chunks into that vector or the chunks in the vector are being created in MyGL.

**Difficulties:**
The primary difficulty came with figuring out how to use the threads to preprocess the chunk information. This required me to refactor the code from MS1 into various helper functions in order to properly do this. For example, each chunk's data is processed individually, and 4x4 chunk groups are done per thread, rather than the 4x4 chunk groups being processed in one pass. I also had to make sure that chunks that were added to the terrain but were not yet fully created were not being drawn yet.


## Things to point out
- Once we merge, the multithreading no longer works. This must have to do with how the code was refactored to distinguish between opaque and transparent blocks, but we were not able to figure out what was causing the issue. Please refer to the multithread branch to confirm that my implementation does work.

# Milestone 1
## Procedural Terrain - Ray DongHo Kim

**Terrain class:**
- For procedurally generated terrain, I used FBM function to generate seemingly-random terrain with smooth curves along the surface.
- For generating additional terrain when walking towards the boundary, timerUpdate checks if additional terrain should be generated or not. If my certain amount of distance away from my current position(camera position) in certain direction does not have a terrain(chunk actually), the function that generates terrain(4x4 chunks) is invoked. Based on the current position and information about which way the player is heading to, it generates 4x4 chunks in that direction, which the height is determined by the same fbm function. This way there are no drastic changes(spikes/cliffs) between chunks, but create smooth procedural terrain. I divided the cases in to 8 cases(North, NE, East, SE, South, SW, West, NW), and for each case, starting corner block's x-z value is different which tells the generating function to create chunks in given direction.
- To add and delete blocks, I used ray-marching method to determine, rather a block is hit along the look vector with certain length. If ray hits a block that is not empty, then it sets that block to empty for deleteBlock. For add block, it computes which face to add block to, by using the intersection point and the block's midpoint, geting the "normal" and determining which face is being intersected by the ray(look vector).

**Difficulties:** 
- Keeping track of correct coordinate value was confusing in the beginning, which is critical to give smooth terrain throughout the chunks, without any obvious boundaries between chunks.
- Adding block initially caused diagonal addition sometimes, which is not allowed. So I had to use extra logic of getting the midpoint of the cube and determining the face that the ray is hitting to properly add a new block by the face, not by edge/corner of a cube.

## Terrain Rendering and Chunking - Nayeong Kim

**Terrain class:**
- Instead of m_blocks, I have m_chunk which is a map from the chunk's origin coordinate (x, z values) to the Chunk
- The keys for m_blocks (chunk origin values) are stored as std::pair<int, int> where the first int is the x origin value and the second int is the z origin value 

**Chunk class:**
- A chunk inherits Drawable and thus has a create function
- A chunk has m_blocks which stores 16*256*16 blocks as an array and an position variable (ivec2) that stores origin x, z values
- A chunk provides getBlockAt for specific block modification within the chunk
- When creating chunks, we iterate through all blocks and check if each the 6 adjacent blocks (left, right, front, back, top, bottom) are empty. When each of them are empty, we compute the position, normal, and color the corresponding face and store them in a single vector pnc.
- pnc is divided into position, normal, color data in shaderprogram.cpp, and each populates vs_pos, vs_nor, vs_col of the shader file

## Game Engine Update Function and Physics - Alexander Do

**Player class:**

- Used glm::vec3s to represent position and velocity
- Used a unique pointer to a camera for camera pointer
- Used tuple of booleans to keep track of wasd presses (used a tuple because I tend to group wasd presses together when I play video games)
- Use separate boolean variables for "god mode" (aka fly mode), keeping track of if the player is on the ground, and if various buttons (e.g. spacebar, lmb, etc.) were pressed or mouse was moved

**Physics** (e.g. inclusion of gravity and collision detection) are determined based on godMode boolean variable

**MyGL::timerUpdate:**

- Step 1 is done by saving a variable "lastUpdate" for the previous value of QDateTime::currentMSecsSinceEpoch, then finding the difference between that and a new call of the function, then updating lastUpdate accordingly.
- Step 2 is done by checking all boolean values for key presses in the player class and acting accordingly (e.g. if mouse was moved, rotate the camera according to pivot model described in slides, if w was pressed, change velocity[0] to 2, etc.)
- Steps 3 and 4 are done by multiplying velocity by a scaled change in time to determine translation in world space, then using the translation in world space to find the final position of the camera if there were no collisions (updatedPos).

  Translation along the look vector is dependent on the godMode boolean. If the player is not in godMode, we must zero the y-component of the look vector and renormalize so that the player only moves along the x-z plane when pressing WASD.

  We then find a ray from the origin to updatedPos. If the player is in godMode, we just update the player's position and camera using this ray and update. Otherwise, we grid march from the 12 vertices of the player using that ray to determine if a collision occurs when not in god mode. We then update the player's position and camera accordingly. Finally, we determine if the player is now standing on the ground by checking the blocks beneath the four "feet" vertices, and if not, update the y-velocity to either be 0 or to simulate gravity based on if the player is in god mode or not.

**Difficulties:** Collision detection was by far the most difficult portion of this section, especially before grid marching was introduced in lecture. Even afterwards, there were some sneaky bugs that arose (e.g. not checking all four vertices of the "feet" when updating the onGround boolean variable). Many hours and trips to office hours were spent trying to get the player to properly collide with the world and debugging problems such as the player transporting to <NAN, NAN, NAN> or not walking off edges properly.  

## Things to point out
- When the player is too close to the cube(right in front of it), the player cannot jump. Has to be at least a step back to jump.
- When key is pressed, without releasing, the terrain does not generate. When the key is released and the player is near the boundary by fixed amount, then the additional terrain gets generated.

