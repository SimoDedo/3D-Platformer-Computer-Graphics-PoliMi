# 3D-Platformer-Computer-Graphics-PoliMi
A 3D platformer featuring a mushroom on quest to recover five scrolls. Made as the final project for the Computer Graphics course at PoliMi.

All assets used can be found online for free. 

![CGProject_WI4nExO02k](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/ccaa0147-81e1-4c84-b84f-f70b215df896)

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
  + Alghoritmic creation of meshes for the bounding boxes that may be displayed as wireframes
+ **Shaders**
  + MRAO maps usage
  + Normal maps used via the TBN matrix
  + Sway of the vegetation in the vertex shaders
  + Use of different UVs in the same mesh to mix different textures on the ground, based on a Noise image
  + Height map in vertex shader
  + Dynamic skybox, mixing different textures to get the correct color of the sky depending on time
## Media
### Start screen

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/93f8944e-7564-42ac-b59f-be0353c516e4

### Gameplay

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/3856a23d-6d1c-4e6d-aa88-e5e8f7270ce0

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/5aee323a-4760-42e2-8301-b7f128e2513b

### Grass

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/915bbf52-b4c6-4302-9190-7288a2fdaccd

![CGProject_3cafoo6ZSj](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/5488cf82-c752-44fc-a9ea-fc2feb68e88e)
![CGProject_tgBSorJHIs](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/96a845ec-f68c-4fc9-9fbf-e6a165b84056)


### Sway

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/ba9dcc08-783e-43c3-9e0b-d0425853ad1a

### Light

https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/fad0e49e-974a-4144-a30f-bc2c7c75d7ee

![CGProject_x0ubQgyYbd](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/e14f221b-669d-463e-b56f-94355b77bef1)
![CGProject_hJMRBW44Q0](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/37cf2102-072d-4494-afab-0133dd46e43e)
![CGProject_aFj5zgYrHy](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/59b532da-a3b2-44c7-b3f9-c22c7762d083)
![CGProject_7DeJOA8tnd](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/e27eb796-4608-4fec-b6cb-9ae9a0c77e86)

### Debug

![immagine](https://github.com/SimoDedo/3D-Platformer-Computer-Graphics-PoliMi/assets/71946484/7d4550f6-806f-4c1f-9591-5f59b6ecd8d2)




