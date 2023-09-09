# 3D-Platformer-Computer-Graphics-PoliMi
A 3D platformer featuring a mushroom on quest to recover five scrolls. Made as the final project for the Computer Graphics course at PoliMi.

All assets used can be found online for free. 

![CGProject_WI4nExO02k](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/2bc48cbe-f2dc-4174-8813-4e2723fee340)

## Overview
The project was developed in C++ using the Vulkan API to write low level operations that handle the creation, transformation and deletion of meshes in a 3D environment.

In order for meshes to be visualized, pipelines are created with various vertex and fragment shaders. 

Pipelines are supplied, via a command buffer, the vertex and index buffers and the Data Sets containing the buffers and textures of the meshes.  

Game logic is implemented in the application to handle jumps, collisions, damage and scroll retirieval.

Shaders implement light models and BRDF models to simulate various lights.

## Features
Beyond what is strictly needed for a Vulkan application (matrix operation, pipeline creation, descriptor layouts, descripror sets, ecc..) here are some of the feature of the developed application.
+ **Game logic**
  + Movement with basic physics
  + Jump with incremental height and lateral movement akin to a 3D platformer
  + Collision detection though ray vs OBB (Oriented Bounding Box), and sphere vs OBB
  + Damage detection though collisions, and health bar
+ **Rendering**
  + Use of storage buffer to implement instanced rendering, paired with a vertex shader
  + Alghoritmic creation of meshes for the bounding boxes
+ **Shaders**
  + MRAO maps usage
  + Normal maps used via the TBN matrix
  + Sway of the vegetation in the vertex shaders
  + Use of different UVs in the same mesh to mix different textures on the ground, based on a Noise image
  + Height map in vertex shader
  + Dynamic skybox, mixing different textures to get the correct color of the sky depending on time
## Media
### Start screen

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/9a1e4a28-001d-4928-b48d-84c4fc678688

### Grass

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/fd65dfeb-bbde-4c30-8f26-6202ef4680de

![CGProject_tgBSorJHIs](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/fecba27f-f933-4dd7-9c46-8f8ea74795e6)
![CGProject_3cafoo6ZSj](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/194ab551-d953-42ab-aee2-713abb39d5b2)

### Sway

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/bc761453-786e-4f1f-be33-9921e1e14d14

### Gameplay

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/47f6bac5-3aa3-4967-8eb8-0f7fa65cfb7b

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/27c2b329-dc7d-4ab5-b9d4-bd3e7d5e9bcf

### Light

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/4489f2d4-0733-494c-988b-9b979fb8d15b

![CGProject_x0ubQgyYbd](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/09e6ae59-5426-4d0a-afd2-4e09d190e40a)
![CGProject_hJMRBW44Q0](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/8bec0a9e-4c98-4b7e-9937-e489a2454c95)
![CGProject_aFj5zgYrHy](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/0398c66a-19c2-45e6-adfb-2c5f00c92445)
![CGProject_7DeJOA8tnd](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/d0bffbd6-f7f3-4c67-85f2-846d90e06a31)
![CGProject_tgBSorJHIs](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/6707505e-ecf4-4794-8271-05f3346bb606)
