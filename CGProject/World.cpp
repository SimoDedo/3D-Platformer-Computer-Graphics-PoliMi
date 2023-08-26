#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define TERRAIN_INSTANCES 2
#define GRASS_INSTANCES 1
#define BIRCH_INSTANCES 10
#define TREE_INSTANCES 4
#define PINE_INSTANCES 15
#define ROCK_INSTANCES 36
#define WALL_INSTANCES 4
#define GPLANT_INSTANCES 3
#define SPIKE_INSTANCES 10
#define SCROLL_INSTANCES 5
#define FLOAT_INSTANCES 6

Transform terrainTransform[TERRAIN_INSTANCES] = {
	{glm::vec3(0, -0.5, -20),			glm::vec3(0),				glm::vec3(2)},
	{glm::vec3(30, 5, -73),				glm::vec3(0,180.f,0),		glm::vec3(1)},
};

Transform grassTransform[GRASS_INSTANCES] = {
	{glm::vec3(1, 0.2f, 1),				glm::vec3(0.0f),			glm::vec3(0.0005f * 0)},
};

Transform birchTransform[BIRCH_INSTANCES] = {
	{glm::vec3(13.5f, -12.f, 10.5f),	glm::vec3(0, 90.0f, 0),		glm::vec3(4.0f)},
	{glm::vec3(12.0f, -13.f, 20.5f),	glm::vec3(0, 45.0f, 0),		glm::vec3(5.5f)},
	{glm::vec3(4.5f, -11.f, 14.5f),		glm::vec3(0, 180.0f, 0),	glm::vec3(2.8f)},

	{glm::vec3(-7.0f, -0.6f, 0.7f),		glm::vec3(0, 90.0f, 0),		glm::vec3(1.7)},
	{glm::vec3(-8.5f, -0.6f, 7.2f),		glm::vec3(0, 56.0f, 0),		glm::vec3(1.9)},
	{glm::vec3(-12.0f, -0.6f, 11.0f),	glm::vec3(0, 112.0f, 0),	glm::vec3(1.5)},
	{glm::vec3(-15.0f, -0.6f, 1.5f),	glm::vec3(0, 12.0f, 0),		glm::vec3(2)},
	{glm::vec3(-5,-0.6f,11),			glm::vec3(0, 124.0f, 0),	glm::vec3(1.7)},
	{glm::vec3(1,-0.6f,12),				glm::vec3(0, 267.0f, 0),	glm::vec3(2.1)},
	{glm::vec3(6,-0.6f,7),				glm::vec3(0, 94.0f, 0),		glm::vec3(1.8)},
};

Transform treeTransform[TREE_INSTANCES] = {
	{glm::vec3(-21.0f, -0.3f, 3.0f),	glm::vec3(0, 0.0f, 0),		glm::vec3(1.6)},
	{glm::vec3(-23.0f, -0.3f, 9.5f),	glm::vec3(0, 100.0f, 0),	glm::vec3(1.7)},
	{glm::vec3(-26.5f, -0.3f, 6.2f),	glm::vec3(0, 20.0f, 0),		glm::vec3(2)},
	{glm::vec3(-8.0f, -0.3f, -3.5f),	glm::vec3(0, 26.0f, 0),		glm::vec3(1.4)},
};

Transform pineTransform[PINE_INSTANCES] = {
	{glm::vec3(26.0f, -0.2f, -21.f),	glm::vec3(0, 0.0f, 0),		glm::vec3(1,0.7f,1.0f)},
	{glm::vec3(15.0f, -0.2f, -11.f),	glm::vec3(0, 23.0f, 0),		glm::vec3(0.75f)},
	{glm::vec3(14.0f, -0.2f, -27.f),	glm::vec3(0, 14.0f, 0),		glm::vec3(0.9f)},
	{glm::vec3(3.0f, -0.2f, -24.f),		glm::vec3(0, 36.0f, 0),		glm::vec3(1.35f)},
	{glm::vec3(3.5f, -0.2f, -18.5f),	glm::vec3(0, 93.0f, 0),		glm::vec3(0.7f)},
	{glm::vec3(0.0f, -0.2f, -15.5f),	glm::vec3(0, 56.0f, 0),		glm::vec3(0.8f)},
	{glm::vec3(4.0f, -0.2f, -9.5f),		glm::vec3(0, 57.0f, 0),		glm::vec3(0.9f)},
	{glm::vec3(10.0f, -0.2f, -14.5f),	glm::vec3(0, 47.0f, 0),		glm::vec3(0.85f)},
	{glm::vec3(26.0f, -0.2f, 4.f),		glm::vec3(0, 86.0f, 0),		glm::vec3(1,1.3f,1)},
	{glm::vec3(21.0f, -0.2f, -38.f),	glm::vec3(0, 46.0f, 0),		glm::vec3(1,0.8f,1)},
	{glm::vec3(20.0f, -0.2f, -29.f),	glm::vec3(0, 56.0f, 0),		glm::vec3(1,1.25f,1)},

	{glm::vec3(40.0f, 4.9f, -62.f),		glm::vec3(0, 198.0f, 0),	glm::vec3(1.8,1.f,1.8)},
	{glm::vec3(42.0f, 4.9f, -73.f),		glm::vec3(0.0f),			glm::vec3(1.8,0.9f,1.8)},
	{glm::vec3(24.0f, 4.9f, -76.5f),	glm::vec3(0.0f),			glm::vec3(1.8,1.2f,1.8)},
	{glm::vec3(15.5f, 4.9f, -64.f),		glm::vec3(0.0f),			glm::vec3(1.8,1.1f,1.8)},
};

Transform rockTransform[ROCK_INSTANCES] = {
	{glm::vec3(-2.3f, -1.7f, -2),		glm::vec3(0, 90.0f, 0),		glm::vec3(2.f)},
	//0
	{glm::vec3(23.0f, 5.0f, 23.0f),		glm::vec3(0, 45.0f,0),		glm::vec3(10.0f)}, //back
	{glm::vec3(10.0f, 5.0f, 23.0f),		glm::vec3(0, 230.0f, 0),	glm::vec3(9.0f)},
	{glm::vec3(0.0f, 5.0f, 23.0f),		glm::vec3(0, 35.0f, 0),		glm::vec3(16.0f)},
	{glm::vec3(-11.0f, 5.0f, 23.0f),	glm::vec3(0, 245.0f, 0),	glm::vec3(11.0f)},
	{glm::vec3(-23.0f, 5.0f, 23.0f),	glm::vec3(0, 0.0f, 0),		glm::vec3(13.0f)},

	{glm::vec3(39.0f, 5.0f, 14.0f),		glm::vec3(0, 315.0f, 0),	glm::vec3(15.0f)}, //right
	{glm::vec3(41.0f, 5.0f, -0.0f),		glm::vec3(0, 115.0f, 0),	glm::vec3(12.0f)},
	{glm::vec3(41.0f, 5.0f, -11.0f),	glm::vec3(0, 330.0f, 0),	glm::vec3(11.0f)},
	{glm::vec3(41.0f, 5.0f, -23.0f),	glm::vec3(0, 110.0f, 0),	glm::vec3(17.0f)},
	{glm::vec3(41.0f, 5.0f, -27.0f),	glm::vec3(0, 300.0f, 0),	glm::vec3(9.0f)},
	{glm::vec3(41.0f, 5.0f, -32.0f),	glm::vec3(0, 270.0f, 0),	glm::vec3(13.0f)},

	{glm::vec3(-35.0f, 5.0f, 9.0f),		glm::vec3(0, 0.0f, 0),		glm::vec3(14.0f)}, //left
	{glm::vec3(-35.0f, 5.0f, -5.0f),	glm::vec3(0, 60.0f, 0),		glm::vec3(12.0f)},
	//1
	{glm::vec3(-10.0f, 0.0f, 3.0f),		glm::vec3(0, 0.0f, 0),		glm::vec3(2.0f)},
	{glm::vec3(-12.0f, .6f, 2.0f),		glm::vec3(0, 115.0f, 0),	glm::vec3(3.0f)},
	{glm::vec3(-15.0f, .8f, 6.0f),		glm::vec3(0, 45.0f, 0),		glm::vec3(5.0f)},

	{glm::vec3(26.0f, 6.f, -13.0f),		glm::vec3(0, 0.0f, 0),		glm::vec3(5.5, 1.2f, 5)},
	{glm::vec3(17.0f, 5.f, -21.0f),		glm::vec3(0, 90.0f, 0),		glm::vec3(5.0f, 1.5f, 5.0f)},
	{glm::vec3(26.0f, 6.5f, -27.0f),	glm::vec3(0, 270.0f, 0),	glm::vec3(5.5f, 2.f, 5.5f)},
	{glm::vec3(23.0f, 8.3f, -36.0f),	glm::vec3(0, 0.0f, 0),		glm::vec3(6.2f, 1.7f, 6.2f)},
	//2
	{glm::vec3(15.0f, 1, 8.0f),			glm::vec3(0, 0.0f, 0),		glm::vec3(2, 1.5, 4)},
	{glm::vec3(17.0f, 1.5f, 0.0f),		glm::vec3(0, 180.0f, 0),	glm::vec3(3, 3, 5)},
	{glm::vec3(21.0f, 2.5, 7.0f),		glm::vec3(0, 90.0f, 0),		glm::vec3(4, 4.5, 6)},
	{glm::vec3(25.0f, 3.5, -2.0f),		glm::vec3(0, 0.0f, 0),		glm::vec3(5, 6, 6)},
	//3
	{glm::vec3(23.0f, 8.3f, -36.0f),	glm::vec3(0, 0.0f, 0),		glm::vec3(0)},
	//4
	{glm::vec3(-10, 5.f, -35.f),		glm::vec3(0, 30.0f, 0),		glm::vec3(6.5f)},
	{glm::vec3(-30, 5.f, -20.f),		glm::vec3(0, -45.0f, 0),	glm::vec3(6.5f)},
	{glm::vec3(-50, 5.f, -45.f),		glm::vec3(0, 180.0f, 0),	glm::vec3(15.0f)},
	{glm::vec3(-24, 5.f, -50.f),		glm::vec3(0, 0.0f, 0),		glm::vec3(5.0f)},
	{glm::vec3(-3, 5.f, -44.f),			glm::vec3(0, 45.0f, 0),		glm::vec3(4.0f)},
	
	{glm::vec3(30, 6.5f, -67.f),		glm::vec3(0, 0.0f, 0),		glm::vec3(3,2.2f,3)},
	//5
	{glm::vec3(+30, -13.f, -45.f),		glm::vec3(0, 90.0f, 0),		glm::vec3(5.0f, 2.5f, 5.0)},
	{glm::vec3(+10, -13.f, -65.f),		glm::vec3(0, 180.0f, 0),	glm::vec3(5.0f, 3, 5.0)},
	{glm::vec3(+50, -13.f, -65.f),		glm::vec3(0, 0.0f, 0),		glm::vec3(5.0f, 3, 5.0)},
	{glm::vec3(+30, -13.f, -93.f),		glm::vec3(0, 270.0f, 0),	glm::vec3(5.0f, 3, 5.5)},
};
int rockType[ROCK_INSTANCES] = {
	0,

	0,
	0,
	0,
	0,
	0,

	0,
	0,
	0,
	0,
	0,
	0,

	0,
	0,

	1,
	1,
	1,

	1,
	1,
	1,
	1,


	2,
	2,
	2,
	2,

	3,

	4,
	4,
	4,
	4,
	4,

	4,

	5,
	5,
	5,
	5,
};


Transform wallTransform[WALL_INSTANCES] = {
	{glm::vec3(25.0f, 4.0f, -5.0f),		glm::vec3(0, 0.0f, 0),		glm::vec3(2,4,2)},
	{glm::vec3(23.0f, 4.0f, -7.0f),		glm::vec3(0, 90.0f, 0),		glm::vec3(2,4,2)},
	{glm::vec3(25.0f, 4.0f, -9.0f),		glm::vec3(0, 180.0f, 0),	glm::vec3(2,4,2)},
	{glm::vec3(27.0f, 4.0f, -7.0f),		glm::vec3(0, 270.0f, 0),	glm::vec3(2,4,2)},
};

Transform gplantTransform[GPLANT_INSTANCES] = {
	{glm::vec3(-15.0f, -0.4f, -15.0f),	glm::vec3(0, 0.0f, 0),		glm::vec3(0.08f),},
	{glm::vec3(12.5f, 0.3f, 5.0f),		glm::vec3(0, 0.0f, 0),		glm::vec3(0.08f),},
	{glm::vec3(9.0f, -0.05f, -28.5f),	glm::vec3(0, 0.0f, 0),		glm::vec3(0.08f),},
};

Transform spikeTransform[SPIKE_INSTANCES] = {
	{glm::vec3(-7.0f, .6f, 2.5f),		glm::vec3(0, 0.0f, 0),		glm::vec3(1.3f)},
	{glm::vec3(-30, 1.f, -34.0f),		glm::vec3(0, 17.0f, 0),		glm::vec3(3.f)},
	{glm::vec3(25.0f, 1.f, -7.0f),		glm::vec3(0, 37.0f, 0),		glm::vec3(2.f)},
	{glm::vec3(29, 10.4f, -68.0f),		glm::vec3(0, 28.0f, 0),		glm::vec3(1.0f)},
	{glm::vec3(32, 10.4f, -65.5f),		glm::vec3(0, 45.0f, 0),		glm::vec3(1.0f)},
	{glm::vec3(33, 9.f, -70),			glm::vec3(0, 13.0f, 0),		glm::vec3(1.0f)},
	{glm::vec3(-15.5f, 1.5f, -22.5),	glm::vec3(0, 15.0f, 0),		glm::vec3(8.0f)},
	{glm::vec3(13.f, 4.2f, 2.2),		glm::vec3(0, 17.0f, 0),		glm::vec3(2.0f)},
	{glm::vec3(22.5f, 5.f, 2.2),		glm::vec3(0, 47.0f, 0),		glm::vec3(1.5f)},
	{glm::vec3(24.f, 8.f, -29.f),		glm::vec3(0, 28.0f, 0),		glm::vec3(1.f)},
};
glm::vec3 spikeDir[SPIKE_INSTANCES] = {
	glm::vec3(-1,0,2),
	glm::vec3(1,0,0),
	glm::vec3(0,1,0),
	glm::vec3(1,0,0),
	glm::vec3(-1,0,0),
	glm::vec3(-1,0,0),
	glm::vec3(0,1,0),
	glm::vec3(1,0,0),
	glm::vec3(0,1,0),
	glm::vec3(1,0,0),
};
float spikeDistance[SPIKE_INSTANCES] = {
	2.0f,
	10.0f,
	0.0f,
	3.0f,
	3.0f,
	4.0f,
	3.0f,
	5.0f,
	3.0f,
	3.0f,
};
float spikeSpeed[SPIKE_INSTANCES] = {
	0.2f,
	0.7f,
	0.8f,
	0.6f,
	0.6f,
	0.7f,
	0.3f,
	0.2f,
	0.5f,
	0.5f,
};

Transform scrollTransform[SCROLL_INSTANCES] = {
	{glm::vec3(-14.0f, 4.5f, 6.0f),		glm::vec3(0.0f, 0.0f, 45.0f),	glm::vec3(1.f)},
	{glm::vec3(-25.0f, 1.f, -40.0f),	glm::vec3(0.0f, 0.0f, 45.0f),	glm::vec3(1.f)},
	{glm::vec3(25.0f, 10.f, -7.0f),		glm::vec3(0.0f, 0.0f, 45.0f),	glm::vec3(1.f)},
	{glm::vec3(30.0f, 12.f, -67.0f),	glm::vec3(0.0f, 0.0f, 45.0f),	glm::vec3(1.f)},
	{glm::vec3(-6.0f, 9.f, -11.0f),		glm::vec3(0.0f, 0.0f, 45.0f),	glm::vec3(1.f)},
};

Transform floatTransform[FLOAT_INSTANCES] = {
	{glm::vec3(10.0f, 4.0f, -29.0f),	glm::vec3(0.0f, 0.0f, 0.0f),	glm::vec3(1.f)},
	{glm::vec3(7.0f, 6.0f, -40.0f),		glm::vec3(0.0f, 45.0f, 0.0f),	glm::vec3(1.8f)},
	{glm::vec3(31.0f, 5.0f, -36.0f),	glm::vec3(0.0f, 90.0f, 0.0f),	glm::vec3(1.3f)},
	{glm::vec3(24.0f, 5.0f, -44.0f),	glm::vec3(0.0f, 12.0f, 0.0f),	glm::vec3(1.4f)},
	{glm::vec3(17.0f, 5.0f, -47.0f),	glm::vec3(0.0f, 112.0f, 0.0f),	glm::vec3(1.1f)},
	{glm::vec3(35.0f, 5.0f, -49.0f),	glm::vec3(0.0f, 78.0f, 0.0f),	glm::vec3(2.1f)},
};