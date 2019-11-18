#Procedural Terrain - Ray DongHo Kim

Additionally, in your readme, briefly describe how you implemented your chosen features. Discuss any difficulties you encountered when coding the project, and explain the approach you took to implementing each feature (e.g. "I chose to cast rays from the corners of my player's bounding box for collision detection because I thought it might be an easier approach than overlap checking.")

**Terrain class:**
- For procedurally generated terrain, I used FBM function to generate seemingly-random terrain with smooth curves along the surface.
- For generating additional terrain when walking towards the boundary, timerUpdate checks if additional terrain should be generated or not. If my certain amount of distance away from my current position(camera position) in certain direction does not have a terrain(chunk actually), the function that generates terrain(4x4 chunks) is invoked. Based on the current position and information about which way the player is heading to, it generates 4x4 chunks in that direction, which the height is determined by the same fbm function. This way there are no drastic changes(spikes/cliffs) between chunks, but create smooth procedural terrain. I divided the cases in to 8 cases(North, NE, East, SE, South, SW, West, NW), and for each case, starting corner block's x-z value is different which tells the generating function to create chunks in given direction.
- To add and delete blocks, I used ray-marching method to determine, rather a block is hit along the look vector with certain length. If ray hits a block that is not empty, then it sets that block to empty for deleteBlock. For add block, it computes which face to add block to, by using the intersection point and the block's midpoint, geting the "normal" and determining which face is being intersected by the ray(look vector).

**Difficulties:** 
- Keeping track of correct coordinate value was confusing in the beginning, which is critical to give smooth terrain throughout the chunks, without any obvious boundaries between chunks.
- Adding block initially caused diagonal addition sometimes, which is not allowed. So I had to use extra logic of getting the midpoint of the cube and determining the face that the ray is hitting to properly add a new block by the face, not by edge/corner of a cube.

#Terrain Rendering and Chunking - Nayeong Kim

**Terrain class:**
- Instead of m_blocks, I have m_chunk which is a map from the chunk's origin coordinate (x, z values) to the Chunk
- The keys for m_blocks (chunk origin values) are stored as std::pair<int, int> where the first int is the x origin value and the second int is the z origin value 

**Chunk class:**
- A chunk inherits Drawable and thus has a create function
- A chunk has m_blocks which stores 16*256*16 blocks as an array and an position variable (ivec2) that stores origin x, z values
- A chunk provides getBlockAt for specific block modification within the chunk
- When creating chunks, we iterate through all blocks and check if each the 6 adjacent blocks (left, right, front, back, top, bottom) are empty. When each of them are empty, we compute the position, normal, and color the corresponding face and store them in a single vector pnc.
- pnc is divided into position, normal, color data in shaderprogram.cpp, and each populates vs_pos, vs_nor, vs_col of the shader file


#Game Engine Update Function and Physics - Alexander Do

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

#Things to point out
- When the player is too close to the cube(right in front of it), the player cannot jump. Has to be at least a step back to jump.
- When key is pressed, without releasing, the terrain does not generate. When the key is released and the player is near the boundary by fixed amount, then the additional terrain gets generated.