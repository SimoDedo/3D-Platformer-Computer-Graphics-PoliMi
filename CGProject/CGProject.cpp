// This has been adapted from the Vulkan tutorial
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Starter.hpp"
#include "Utils.hpp"
#include "Enemy.cpp"
#include "Controller.cpp"
#include "World.cpp"

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)
#define GRASS_MODELS 8
#define ROCK_MODELS 6

// MAIN ! 
class CGProject : public BaseProject {
	protected:
	// Other application parameters
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point lastPressTime;
	float minimumPressDelay = 0.2f;

	int gameState = GAME_STATE_START_SCREEN;
	int DEBUG = 0;

	Controller controller;
	Enemy spikeEnemy[SPIKE_INSTANCES];
	Scroll scroll[SCROLL_INSTANCES];
	float freqDay = 0.025f; //Period (time for a day) = 2*Pi / freq
		//Values that define three parabolas for day/sunrise/night skybox cycle
	float
		dayStart = -0.4f,
		sunriseStart = -0.2f, sunriseEnd = 0.4f,
		nightStart = 0.2f;
	glm::vec2
		knownPointDay = glm::vec2(dayStart, 0),
		knownPointSunrise = glm::vec2(sunriseStart, 0),
		knownPointNight = glm::vec2(nightStart, 0);
	glm::vec2
		vertexDay = glm::vec2(1, 1.2f),
		vertexSunrise = glm::vec2(sunriseStart + (sunriseEnd - sunriseStart) / 2, 1),
		vertexNight = glm::vec2(-1, 1);
	float
		aCoeffDay = aCoefficientParabola(knownPointDay, vertexDay),
		aCoeffSunrise = aCoefficientParabola(knownPointSunrise, vertexSunrise),
		aCoeffNight = aCoefficientParabola(knownPointNight, vertexNight);

	float grassSwayRand[GRASS_INSTANCES], treeSwayRand[TREE_INSTANCES], birchSwayRand[BIRCH_INSTANCES], pineSwayRand[PINE_INSTANCES], gplantSwayRand[GPLANT_INSTANCES];

	// Current aspect ratio (used by the callback that resized the window
	float winW, winH, Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLGubo, DSLMesh, DSLMeshNorm, DSLTerrain, DSLSkyBox, DSLOverlay;

	// Vertex formats
	VertexDescriptor VMesh, VTerrain, VSkyBox, VOverlay;

	// Pipelines [Shader couples]
	Pipeline PMesh, PMeshSway, PMeshNorm, PGround, PSkyBox, POverlay;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	Model<VertexSkyBox> MSkyBox;
	Model<VertexMesh> MShroom, MGrass[GRASS_MODELS], MBirch, MTree, MPine, MRock[ROCK_MODELS], MWall, MGplant, MFloat, MSpike, MScroll;
	Model<VertexTerrain> MTerrain;
	Model<VertexOverlay> MText, MHUD[4];

	DescriptorSet DSGubo, DSTerrain[TERRAIN_INSTANCES], DSShroom, DSGrass[GRASS_INSTANCES], DSBirch[BIRCH_INSTANCES], DSTree[TREE_INSTANCES], DSPine[PINE_INSTANCES], DSRock[ROCK_INSTANCES], DSWall[WALL_INSTANCES],
		DSScroll[SCROLL_INSTANCES], DSGplant[GPLANT_INSTANCES], DSFloat[FLOAT_INSTANCES], DSSpike[SPIKE_INSTANCES], DSText, DSHUD[4], DSSkyBox;

	Texture TTerrain, TTerrain2, TTerrainNoise, TTerrain_n, TTerrain2_n, TTerrainHeight, TTerrain_MRAO, TTerrain2_MRAO;
	Texture TShroom, TShroom_MRAO;
	Texture TGrass, TGrass_MRAO;
	Texture TBirch, TBirch_MRAO;
	Texture TTree, TTree_MRAO;
	Texture TPine, TPine_MRAO;
	Texture TRock[ROCK_MODELS], TRock_n[ROCK_MODELS], TRock_MRAO[ROCK_MODELS];
	Texture TWall, TWall_n, TWall_MRAO;
	Texture TScroll, TScroll_n, TScroll_MRAO;
	Texture TGplant, TGplant_MRAO, TGplant_e;
	Texture TFloat, TFloat_MRAO;
	Texture TSpike, TSpike_n, TSpike_MRAO;
	Texture TSkyBoxDay, TSkyBoxSunrise, TSkyBoxNight;
	Texture TText, THUD[4];
	Texture TDummy_b;
	
	// C++ storage for uniform variables
	MeshUniformBlock uboTerrain[TERRAIN_INSTANCES], uboShroom, uboRock[ROCK_INSTANCES], uboWall[WALL_INSTANCES], uboScroll[SCROLL_INSTANCES], uboFloat[FLOAT_INSTANCES];
	MeshSwayUniformBlock uboGrass[GRASS_INSTANCES], uboBirch[BIRCH_INSTANCES], uboTree[TREE_INSTANCES], uboPine[PINE_INSTANCES], uboGplant[GPLANT_INSTANCES], uboSpike[SPIKE_INSTANCES];
	SkyBoxUniformBlock uboSkyBox;
	OverlayUniformBlock uboText, uboHUD[4];
	GlobalUniformBlock gubo;

	BoundingSphere bsShroom;
	BoundingBox bbShroom, bbTerrain[TERRAIN_INSTANCES], bbBirch[BIRCH_INSTANCES*2], bbTree[TREE_INSTANCES*2], bbPine[PINE_INSTANCES], bbWall[WALL_INSTANCES], bbScroll[SCROLL_INSTANCES],  bbSpike[SPIKE_INSTANCES],
		bbFloat[FLOAT_INSTANCES];
	std::vector<BoundingBox *> bbRock; //Need collection for rock to know at runtime how many rock of each type and adjust bb accordingly (some rocks have more than one)
	std::vector<BoundingBox *> bbCollection;

	Pipeline PDebug;
	DescriptorSetLayout DSLBoundingBox;
	VertexDescriptor VBoundingBox;
	std::vector<Model<VertexBoundingBox>> MBoundingBox;
	std::vector<DescriptorSet> DSBoundingBox;
	std::vector<BoundingBoxUniformBlock> uboBoundingBox;


	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 1920;
		windowHeight = 1024;
		windowTitle = "CGProject";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.0f, 0.005f, 0.01f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 400;
		texturesInPool = 400;
		setsInPool = 400;
		
		Ar = (float)windowWidth / (float)windowHeight;
	}
	
	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		winW = (float)w;
		winH = (float)h;
		Ar = (float)w / (float)h;
	}
	
	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		// Init local variables
		//Starting time to calculate elapsed seconds when needed
		startTime = std::chrono::high_resolution_clock::now();
		//Randomize variable that need randomizing
		for (int i = 0; i < GRASS_INSTANCES; i++)
		{
			grassSwayRand[i] = RandFloat(0, 3.14f);
		}
		for (int i = 0; i < TREE_INSTANCES; i++)
		{
			treeSwayRand[i] = RandFloat(0, 3.14f);
		}
		for (int i = 0; i < BIRCH_INSTANCES; i++)
		{
			birchSwayRand[i] = RandFloat(0, 3.14f);
		}
		for (int i = 0; i < PINE_INSTANCES; i++)
		{
			pineSwayRand[i] = RandFloat(0, 3.14f);
		}
		for (int i = 0; i < GPLANT_INSTANCES; i++)
		{
			gplantSwayRand[i] = RandFloat(0, 3.14f);
		}
		//Algorithmically place objects in the scene
		
		//Define bounding boxes for collision and their ubo for debugging
		bsShroom.pos = glm::vec3(0, controller.getCharOriginOffset_y(), 0);
		bsShroom.radius = 0.6f;
		bsShroom.mMat = &(uboShroom.mMat);

		bbShroom.pos = glm::vec3(0, controller.getCharOriginOffset_y(), 0);
		bbShroom.halfExtents = glm::vec3(0.27f);
		bbShroom.canCollide = false;
		bbShroom.isEnemy = false;
		bbShroom.mMat = &(uboShroom.mMat);
		bbShroom.nMat = &(uboShroom.nMat);
		bbCollection.push_back(&bbShroom);

		for (int i = 0; i < TERRAIN_INSTANCES; i++)
		{
			bbTerrain[i].pos = glm::vec3(0,-1.7,0);
			bbTerrain[i].halfExtents = glm::vec3(20, 2.0f, 20);
			bbTerrain[i].canCollide = true;
			bbTerrain[i].isEnemy = false;
			bbTerrain[i].mMat = &(uboTerrain[i].mMat);
			bbTerrain[i].nMat = &(uboTerrain[i].nMat);
			bbCollection.push_back(&bbTerrain[i]);
		}
		
		for (int i = 0; i < BIRCH_INSTANCES; i++)
		{
			bbBirch[i * 2].pos = glm::vec3(0, 1.4, 0); //Trunk
			bbBirch[i * 2].halfExtents = glm::vec3(0.1f, 1.4, 0.1f);
			bbBirch[i * 2].canCollide = true;
			bbBirch[i * 2].isEnemy= false;
			bbBirch[i * 2].mMat = &(uboBirch[i].mMat);
			bbBirch[i * 2].nMat = &(uboBirch[i].nMat);
			bbCollection.push_back(&bbBirch[i * 2]);
			bbBirch[i * 2 + 1].pos = glm::vec3(0, 3, 0); //Leaves
			bbBirch[i * 2 + 1].halfExtents = glm::vec3(0.7f, 0.5f, 0.7f);
			bbBirch[i * 2 + 1].canCollide = true;
			bbBirch[i * 2 + 1].isEnemy = false;
			bbBirch[i * 2 + 1].mMat = &(uboBirch[i].mMat);
			bbBirch[i * 2 + 1].nMat = &(uboBirch[i].nMat);
			bbCollection.push_back(&bbBirch[i * 2 + 1]);
		}
		for (int i = 0; i < TREE_INSTANCES; i++)
		{
			bbTree[i * 2].pos = glm::vec3(0, 1.4, 0); //Trunk
			bbTree[i * 2].halfExtents = glm::vec3(0.2f, 1.2, 0.2f);
			bbTree[i * 2].canCollide = true;
			bbTree[i * 2].isEnemy = false;
			bbTree[i * 2].mMat = &(uboTree[i].mMat);
			bbTree[i * 2].nMat = &(uboTree[i].nMat);
			bbCollection.push_back(&bbTree[i * 2]);
			bbTree[i * 2 + 1].pos = glm::vec3(-0.2, 4.5, -0.1); //Leaves
			bbTree[i * 2 + 1].halfExtents = glm::vec3(0.9, 0.8, 0.8f);
			bbTree[i * 2 + 1].canCollide = true;
			bbTree[i * 2 + 1].isEnemy = false;
			bbTree[i * 2 + 1].mMat = &(uboTree[i].mMat);
			bbTree[i * 2 + 1].nMat = &(uboTree[i].nMat);
			bbCollection.push_back(&bbTree[i * 2 + 1]);
		}
		for (int i = 0; i < PINE_INSTANCES; i++)
		{
			bbPine[i].pos = glm::vec3(0, 3, 0); //Trunk
			bbPine[i].halfExtents = glm::vec3(0.4f, 6, 0.4f);
			bbPine[i].canCollide = true;
			bbPine[i].isEnemy = false;
			bbPine[i].mMat = &(uboPine[i].mMat);
			bbPine[i].nMat = &(uboPine[i].nMat);
			bbCollection.push_back(&bbPine[i]);
		}
		for (int i = 0; i < ROCK_INSTANCES; i++)
		{
			if (rockType[i] == 0){
				BoundingBox* bb1 = new BoundingBox();
				bb1->pos = glm::vec3(-0.3, 0, 0); //Big rock
				bb1->halfExtents = glm::vec3(0.5f, 0.55f, 0.5f);
				bb1->canCollide = true;
				bb1->isEnemy = false;
				bb1->mMat = &(uboRock[i].mMat);
				bb1->nMat = &(uboRock[i].nMat);
				bbRock.push_back(bb1);
				bbCollection.push_back(bb1);
				BoundingBox* bb2 = new BoundingBox();
				bbRock.push_back(bb2);
				bb2->pos = glm::vec3(0.2, -0.25, 0.6); //Small rock
				bb2->halfExtents = glm::vec3(0.3f, 0.35f, 0.3f);
				bb2->canCollide = true;
				bb2->isEnemy = false;
				bb2->mMat = &(uboRock[i].mMat);
				bb2->nMat = &(uboRock[i].nMat);
				bbCollection.push_back(bb2);
			}
			else if (rockType[i] == 1) {
				BoundingBox* bb1 = new BoundingBox();
				bbRock.push_back(bb1);
				bb1->pos = glm::vec3(0, 0, 0);
				bb1->halfExtents = glm::vec3(0.6f, 0.52f, 0.6f);
				bb1->canCollide = true;
				bb1->isEnemy = false;
				bb1->mMat = &(uboRock[i].mMat);
				bb1->nMat = &(uboRock[i].nMat);
				bbCollection.push_back(bb1);
			}
			else if (rockType[i] == 2) {
				BoundingBox* bb1 = new BoundingBox();
				bbRock.push_back(bb1);
				bb1->pos = glm::vec3(0, 0, 0);
				bb1->halfExtents = glm::vec3(0.53f, 0.6f, 0.4f);
				bb1->canCollide = true;
				bb1->isEnemy = false;
				bb1->mMat = &(uboRock[i].mMat);
				bb1->nMat = &(uboRock[i].nMat);
				bbCollection.push_back(bb1);
			}
			else if (rockType[i] == 3) {
				BoundingBox* bb1 = new BoundingBox();
				bbRock.push_back(bb1);
				bb1->pos = glm::vec3(0, 0, 0);
				bb1->halfExtents = glm::vec3(1.3f, 0.55f, 0.5f);
				bb1->canCollide = true;
				bb1->isEnemy = false;
				bb1->mMat = &(uboRock[i].mMat);
				bb1->nMat = &(uboRock[i].nMat);
				bbCollection.push_back(bb1);
			}
			else if (rockType[i] == 4) {
				BoundingBox* bb1 = new BoundingBox();
				bbRock.push_back(bb1);
				bb1->pos = glm::vec3(0, -0.5, 0); //Lower rock
				bb1->halfExtents = glm::vec3(1.2f, 0.7, 1.2f);
				bb1->canCollide = true;
				bb1->isEnemy = false;
				bb1->mMat = &(uboRock[i].mMat);
				bb1->nMat = &(uboRock[i].nMat);
				bbCollection.push_back(bb1);
				BoundingBox* bb2 = new BoundingBox();
				bbRock.push_back(bb2);
				bb2->pos = glm::vec3(0.1, 0.5, 0); //Upper rock
				bb2->halfExtents = glm::vec3(0.7, 1, 0.7);
				bb2->canCollide = true;
				bb2->isEnemy = false;
				bb2->mMat = &(uboRock[i].mMat);
				bb2->nMat = &(uboRock[i].nMat);
				bbCollection.push_back(bb2);
				BoundingBox* bb3 = new BoundingBox();
				bbRock.push_back(bb3);
				bb3->pos = glm::vec3(0, 0, -0.7); //Upper rock
				bb3->halfExtents = glm::vec3(1.2, 0.9, 0.6);
				bb3->canCollide = true;
				bb3->isEnemy = false;
				bb3->mMat = &(uboRock[i].mMat);
				bb3->nMat = &(uboRock[i].nMat);
				bbCollection.push_back(bb3);
			}
			else if (rockType[i] == 5) {
				BoundingBox* bb1 = new BoundingBox();
				bbRock.push_back(bb1);
				bb1->pos = glm::vec3(1, 4.f, 0);
				bb1->halfExtents = glm::vec3(1.f, 3.8f, 4.f);
				bb1->canCollide = true;
				bb1->isEnemy = false;
				bb1->mMat = &(uboRock[i].mMat);
				bb1->nMat = &(uboRock[i].nMat);
				bbCollection.push_back(bb1);
			}
		}
		for (int i = 0; i < WALL_INSTANCES; i++)
		{
			bbWall[i].pos = glm::vec3(0, -0.1f, 0);
			bbWall[i].halfExtents = glm::vec3(1.1f, 1.f, 0.1f);
			bbWall[i].canCollide = true;
			bbWall[i].isEnemy = false;
			bbWall[i].mMat = &(uboWall[i].mMat);
			bbWall[i].nMat = &(uboWall[i].nMat);
			bbCollection.push_back(&bbWall[i]);
		}
		for (int i = 0; i < SCROLL_INSTANCES; i++)
		{
			bbScroll[i].pos = glm::vec3(0, 0, 0);
			bbScroll[i].halfExtents = glm::vec3(0.7f, 0.25f, 0.25f);
			bbScroll[i].canCollide = false;
			bbScroll[i].isEnemy = false;
			bbScroll[i].mMat = &(uboScroll[i].mMat);
			bbScroll[i].nMat = &(uboScroll[i].nMat);
			bbCollection.push_back(&bbScroll[i]);
		}
		for (int i = 0; i < SPIKE_INSTANCES; i++)
		{
			bbSpike[i].pos = glm::vec3(0, 0, 0);
			bbSpike[i].halfExtents = glm::vec3(.3f);
			bbSpike[i].canCollide = true;
			bbSpike[i].isEnemy = true;
			bbSpike[i].mMat = &(uboSpike[i].mMat);
			bbSpike[i].nMat = &(uboSpike[i].nMat);
			bbCollection.push_back(&bbSpike[i]);
		}
		for (int i = 0; i < FLOAT_INSTANCES; i++)
		{
			bbFloat[i].pos = glm::vec3(0, 6, 0);
			bbFloat[i].halfExtents = glm::vec3(1,2.4f,1);
			bbFloat[i].canCollide = true;
			bbFloat[i].isEnemy = true;
			bbFloat[i].mMat = &(uboFloat[i].mMat);
			bbFloat[i].nMat = &(uboFloat[i].nMat);
			bbCollection.push_back(&bbFloat[i]);
		}

		//Create the exact number of ubo needed for the bounding boxes for debug display
		for (int i = 0; i < bbCollection.size(); i++)
		{
			uboBoundingBox.push_back(BoundingBoxUniformBlock());
		}
		
		//Initialize character controller and enemies
		controller.init(bbShroom);
		for (int i = 0; i < SPIKE_INSTANCES; i++) {
			spikeEnemy[i].init(bbSpike[i], spikePosition[i], spikeRot[i], spikeScale[i], spikeDir[i], spikeDistance[i], spikeSpeed[i]);
		}
		for (int i = 0; i < SCROLL_INSTANCES; i++) {
			scroll[i].init(bbScroll[i], scrollPosition[i], scrollRot[i], scrollScale[i]);
		}

		// Descriptor Layouts [what will be passed to the shaders]
		DSLMesh.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//Diffuse
					{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//MRAO
					{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}	//Emission (Use TDummy_b if no emission needed)
				});
		DSLMeshNorm.init(this, {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
				{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//Diffuse
				{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//Normal
				{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}	//MRAO
			});
		DSLTerrain.init(this, {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
				{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT},		//Height
				{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//Diffuse1
				{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//Normal1
				{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//MRAO1
				{5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//Diffuse2
				{6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//Normal2
				{7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//MRAO2
				{8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}	//Noise
			});
		DSLSkyBox.init(this, {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
				{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//Cubic day
				{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	//Cubic twilight
				{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}	//Cubic night
			});
		DSLOverlay.init(this, {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
				{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});
		DSLGubo.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
				});
		DSLBoundingBox.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
				});

		// Vertex descriptors
		VMesh.init(this, {
				  {0, sizeof(VertexMesh), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, pos),
				         sizeof(glm::vec3), POSITION},
				  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, norm),
				         sizeof(glm::vec3), NORMAL},
				  {0, 2, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexMesh, tangent),
					   sizeof(glm::vec4), TANGENT},
				  {0, 3, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMesh, UV),
				         sizeof(glm::vec2), UV_0}
				});
		VTerrain.init(this, {
			{0, sizeof(VertexTerrain), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexTerrain, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexTerrain, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexTerrain, tangent),
					   sizeof(glm::vec4), TANGENT},
				{0, 3, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexTerrain, UV),
					   sizeof(glm::vec2), UV_0},
				{0, 4, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexTerrain, UVNoise),
					   sizeof(glm::vec2), UV_1}
			});
		VSkyBox.init(this, {
			{0, sizeof(VertexSkyBox), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexSkyBox, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexSkyBox, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexSkyBox, UV),
					   sizeof(glm::vec2), UV_0}
			});
		VOverlay.init(this, {
			{0, sizeof(VertexOverlay), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				  {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, pos),
						 sizeof(glm::vec2), OTHER},
				  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, UV),
						 sizeof(glm::vec2), UV_0}
			});
		VBoundingBox.init(this, {
			{0, sizeof(VertexBoundingBox), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexBoundingBox, pos),
					   sizeof(glm::vec3), POSITION}
			});

		// Pipelines [Shader couples]
		PMesh.init(this, &VMesh, "shaders/MeshVert.spv", "shaders/MeshFrag.spv", {&DSLGubo, &DSLMesh});
		PMesh.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT, false);
		PMeshSway.init(this, &VMesh, "shaders/MeshSwayVert.spv", "shaders/MeshFrag.spv", { &DSLGubo, &DSLMesh });
		PMeshSway.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT, true);
		PMeshNorm.init(this, &VMesh, "shaders/MeshNormVert.spv", "shaders/MeshNormFrag.spv", { &DSLGubo, &DSLMeshNorm });
		PMeshNorm.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT, false);
		PGround.init(this, &VTerrain, "shaders/GroundVert.spv", "shaders/GroundFrag.spv", { &DSLGubo, &DSLTerrain });
		PGround.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT, false);
		PSkyBox.init(this, &VSkyBox,"shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", { &DSLGubo, &DSLSkyBox });
		PSkyBox.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, false);
		POverlay.init(this, &VOverlay, "shaders/OverlayVert.spv", "shaders/OverlayFrag.spv", { &DSLOverlay });
		POverlay.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, true);
		PDebug.init(this, &VBoundingBox, "shaders/BoundingBoxVert.spv", "shaders/BoundingBoxFrag.spv", { &DSLBoundingBox });
		PDebug.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_LINE,
			VK_CULL_MODE_NONE, false);

		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		MTerrain.init(this, &VTerrain, "Models/Ground/ground.gltf", GLTF);
		MSkyBox.init(this, &VSkyBox, "models/SkyBox/SkyBoxCube.gltf", GLTF);
		MShroom.init(this, &VMesh, "Models/ShroomFriend/shroom.gltf", GLTF);
		for (int i = 0; i < GRASS_MODELS; i++) {
			MGrass[i].init(this,   &VMesh, string_format("Models/Grass/grass%d.gltf", i+1), GLTF);
		}
		MBirch.init(this, &VMesh, "Models/Birch/birch.gltf", GLTF);
		MPine.init(this, &VMesh, "Models/Pine/Pine.gltf", GLTF);
		MTree.init(this, &VMesh, "Models/Tree/tree.gltf", GLTF);
		for (int i = 0; i < ROCK_MODELS; i++)
		{
			MRock[i].init(this, &VMesh, string_format("Models/Rock/rock%d.gltf", i), GLTF);
		}
		MWall.init(this, &VMesh, "Models/Wall/wall.gltf", GLTF);
		MScroll.init(this, &VMesh, "Models/Scroll/scroll.gltf", GLTF);
		MGplant.init(this, &VMesh, "Models/GPlant/gplant.gltf", GLTF);
		MFloat.init(this, &VMesh, "Models/Float/float.gltf", GLTF);
		MSpike.init(this, &VMesh, "Models/Spike/spike.gltf", GLTF);
		for (int i = 0; i < bbCollection.size(); i++)
		{
			MBoundingBox.push_back(Model<VertexBoundingBox>());
			CreateBoundingBoxModel(MBoundingBox[i].vertices, MBoundingBox[i].indices, bbCollection[i]);
			MBoundingBox[i].initMesh(this, &VBoundingBox);
		}
		
		glm::vec2 anchor = glm::vec2(-0.7f);
		float h = 0.45;
		float w = 0.55;
		MText.vertices = { {{anchor.x, anchor.y}, {0.0f,0.0f}}, {{anchor.x, anchor.y + h}, {0.0f,1.0f}},
						 {{anchor.x + w, anchor.y}, {1.0f,0.0f}}, {{ anchor.x + w, anchor.y + h}, {1.0f,1.0f}} };
		MText.indices = { 0, 1, 2,    1, 2, 3 };
		MText.initMesh(this, &VOverlay);
		anchor = glm::vec2(-0.95, -0.95);
		h = 0.25;
		w = 0.1;
		for (int i = 0; i < 4; i++)
		{
			MHUD[i].vertices = { {{anchor.x, anchor.y}, {0.0f,0.0f}}, {{anchor.x, anchor.y + h}, {0.0f,1.0f}},
							 {{anchor.x + w, anchor.y}, {1.0f,0.0f}}, {{ anchor.x + w, anchor.y + h}, {1.0f,1.0f}} };
			MHUD[i].indices = { 0, 1, 2,    1, 2, 3 };
			MHUD[i].initMesh(this, &VOverlay);
		}

		// Create the textures
		TTerrain.init(this, "textures/Ground/Grass_bluetint_up2.png");
		TTerrain_n.init(this, "textures/Ground/Grass_bluetint_up_n.png", VK_FORMAT_R8G8B8A8_UNORM); //Normal map need VK_FORMAT_R8G8B8A8_UNORM
		TTerrain_MRAO.init(this, "textures/Ground/Grass_bluetint_up_mrao.png");
		TTerrain2.init(this, "textures/Ground/Stylized_02_Stone_Ground_basecolor.jpg");
		TTerrain2_n.init(this, "textures/Ground/Stylized_02_Stone_Ground_normal.jpg", VK_FORMAT_R8G8B8A8_UNORM);
		TTerrain2_MRAO.init(this, "textures/Ground/Stylized_02_Stone_Ground_MRAO.jpg");
		TTerrainNoise.init(this, "textures/Ground/Noise5.png");
		TTerrainHeight.init(this, "textures/Ground/HeightMap.png");

		TShroom.init(this, "textures/ShroomFriend/mushroom_friend_baseColor.png");
		TShroom_MRAO.init(this, "textures/ShroomFriend/mushroom_friend_baseColor_MRAO.png");

		TGrass.init(this, "textures/Grass/GrassGrass_MAT_baseColor.png");
		TGrass_MRAO.init(this,   "textures/Grass/GrassGrass_MAT_baseColor_MRAO.png");

		TBirch.init(this, "textures/Birch/birch_red_mat_baseColor.png");
		TBirch_MRAO.init(this, "textures/Birch/birch_red_mat_baseColor_MRAO.png");

		TTree.init(this, "textures/Tree/MergedBake_Bake1.png");
		TTree_MRAO.init(this, "textures/Tree/MergedBake_Bake1_MRAO.png");

		TPine.init(this, "textures/Pine/pine_tree_red_mat_baseColor.png");
		TPine_MRAO.init(this, "textures/Pine/pine_tree_red_mat_MRAO.png");

		for (int i = 0; i < ROCK_MODELS; i++)
		{
			TRock[i].init(this, string_format("textures/Rock/rock%d_baseColor.png", i).c_str());
			TRock_n[i].init(this, string_format("textures/Rock/rock%d_normal.png", i).c_str(), VK_FORMAT_R8G8B8A8_UNORM);
			TRock_MRAO[i].init(this, string_format("textures/Rock/rock%d_MRAO.png", i).c_str());
		}

		TWall.init(this, "textures/Wall/stone_wall_low_diffuse.jpeg");
		TWall_n.init(this, "textures/Wall/stone_wall_low_normal.png", VK_FORMAT_R8G8B8A8_UNORM);
		TWall_MRAO.init(this, "textures/Wall/stone_wall_low_diffuse_MRAO.png");

		TScroll.init(this, "textures/Scroll/Scroll_baseColor.jpg");
		TScroll_n.init(this, "textures/Scroll/Scroll_normal.png", VK_FORMAT_R8G8B8A8_UNORM);
		TScroll_MRAO.init(this, "textures/Scroll/Scroll_MRAO.png");

		TGplant.init(this, "textures/GPlant/glowing_plants_baseColor.png");
		TGplant_MRAO.init(this, "textures/GPlant/glowing_plants_baseColor_MRAO.png");
		TGplant_e.init(this, "textures/GPlant/glowing_plants_emissive.png");

		TFloat.init(this, "textures/FloatingRock/floatingrock_baseColor.png");
		TFloat_MRAO.init(this, "textures/FloatingRock/floatingrock_MRAO.png");

		TSpike.init(this, "textures/Spike/map1_baseColor.png");
		TSpike_n.init(this, "textures/Spike/map1_normal.png", VK_FORMAT_R8G8B8A8_UNORM);
		TSpike_MRAO.init(this, "textures/Spike/map1_MRAO.png");

		const char* T2fnday[] = {	"textures/SkyBox/d_right.png", "textures/SkyBox/d_left.png",
								"textures/SkyBox/d_top.png",   "textures/SkyBox/d_bottom.png",
								"textures/SkyBox/d_front.png", "textures/SkyBox/d_back.png" };
		TSkyBoxDay.initCubic(this, T2fnday);
		const char* T2fnsunrise[] = {	"textures/SkyBox/s_right.png", "textures/SkyBox/s_left.png",
								"textures/SkyBox/s_top.png",   "textures/SkyBox/s_bottom.png",
								"textures/SkyBox/s_front.png", "textures/SkyBox/s_back.png" };
		TSkyBoxSunrise.initCubic(this, T2fnsunrise);
		const char* T2fnnight[] = {	"textures/SkyBox/n_right.png", "textures/SkyBox/n_left.png",
								"textures/SkyBox/n_top.png",   "textures/SkyBox/n_bottom.png",
								"textures/SkyBox/n_front.png", "textures/SkyBox/n_back.png" };
		TSkyBoxNight.initCubic(this, T2fnnight);
		TText.init(this, "textures/Overlay/text.png");
		for (int i = 0; i < 4; i++)
		{
			THUD[i].init(this, string_format("textures/Overlay/life%d.png", i ).c_str());
		}
		TDummy_b.init(this, "textures/dummy_black.png");
	}
	
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		PMesh.create();
		PMeshSway.create();
		PMeshNorm.create();
		PGround.create();
		PSkyBox.create();
		POverlay.create();
		PDebug.create();

		
		// Here you define the data set
		for (int i = 0; i < TERRAIN_INSTANCES; i++)
		{
			DSTerrain[i].init(this, &DSLTerrain, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TTerrainHeight},
					{2, TEXTURE, 0, &TTerrain},
					{3, TEXTURE, 0, &TTerrain2},
					{4, TEXTURE, 0, &TTerrain_n},
					{5, TEXTURE, 0, &TTerrain2_n},
					{6, TEXTURE, 0, &TTerrain_MRAO},
					{7, TEXTURE, 0, &TTerrain2_MRAO},
					{8, TEXTURE, 0, &TTerrainNoise}
				});
		}
		DSShroom.init(this, &DSLMesh, {
				{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
				{1, TEXTURE, 0, &TShroom},
				{2, TEXTURE, 0, &TShroom_MRAO},
				{3, TEXTURE, 0, &TDummy_b}
			});
		for (int i = 0; i < GRASS_INSTANCES; i++)
		{
			DSGrass[i].init(this, &DSLMesh, {
						{0, UNIFORM, sizeof(MeshSwayUniformBlock), nullptr},
						{1, TEXTURE, 0, &TGrass},
						{2, TEXTURE, 0, &TGrass_MRAO},
						{3, TEXTURE, 0, &TDummy_b}
					});
		}
		for (int i = 0; i < BIRCH_INSTANCES; i++)
		{
			DSBirch[i].init(this, &DSLMesh, {
						{0, UNIFORM, sizeof(MeshSwayUniformBlock), nullptr},
						{1, TEXTURE, 0, &TBirch},
						{2, TEXTURE, 0, &TBirch_MRAO},
						{3, TEXTURE, 0, &TDummy_b}
				});
		}
		for (int i = 0; i < TREE_INSTANCES; i++)
		{
			DSTree[i].init(this, &DSLMesh, {
						{0, UNIFORM, sizeof(MeshSwayUniformBlock), nullptr},
						{1, TEXTURE, 0, &TTree},
						{2, TEXTURE, 0, &TTree_MRAO},
						{3, TEXTURE, 0, &TDummy_b}
				});
		}
		for (int i = 0; i < PINE_INSTANCES; i++)
		{
			DSPine[i].init(this, &DSLMesh, {
						{0, UNIFORM, sizeof(MeshSwayUniformBlock), nullptr},
						{1, TEXTURE, 0, &TPine},
						{2, TEXTURE, 0, &TPine_MRAO},
						{3, TEXTURE, 0, &TDummy_b}
				});
		}
		for (int i = 0; i < ROCK_INSTANCES; i++)
		{
			DSRock[i].init(this, &DSLMeshNorm, {
						{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
						{1, TEXTURE, 0, &TRock[rockType[i]]},
						{2, TEXTURE, 0, &TRock_n[rockType[i]]},
						{3, TEXTURE, 0, &TRock_MRAO[rockType[i]]}
				});
		}
		for (int i = 0; i < WALL_INSTANCES; i++)
		{
			DSWall[i].init(this, &DSLMeshNorm, {
						{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
						{1, TEXTURE, 0, &TWall},
						{2, TEXTURE, 0, &TWall_n},
						{3, TEXTURE, 0, &TWall_MRAO}
				});
		}
		for (int i = 0; i < SCROLL_INSTANCES; i++)
		{
			DSScroll[i].init(this, &DSLMeshNorm, {
						{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
						{1, TEXTURE, 0, &TScroll},
						{2, TEXTURE, 0, &TScroll_n},
						{3, TEXTURE, 0, &TScroll_MRAO}
				});
		}
		for (int i = 0; i < GPLANT_INSTANCES; i++)
		{
			DSGplant[i].init(this, &DSLMesh, {
						{0, UNIFORM, sizeof(MeshSwayUniformBlock), nullptr},
						{1, TEXTURE, 0, &TGplant},
						{2, TEXTURE, 0, &TGplant_MRAO},
						{3, TEXTURE, 0, &TGplant_e}
				});
		}
		for (int i = 0; i < FLOAT_INSTANCES; i++)
		{
			DSFloat[i].init(this, &DSLMesh, {
						{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
						{1, TEXTURE, 0, &TFloat},
						{2, TEXTURE, 0, &TFloat_MRAO},
						{3, TEXTURE, 0, &TDummy_b}
				});
		}
		for (int i = 0; i < SPIKE_INSTANCES; i++)
		{
			DSSpike[i].init(this, &DSLMeshNorm, {
						{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
						{1, TEXTURE, 0, &TSpike},
						{2, TEXTURE, 0, &TSpike_n},
						{3, TEXTURE, 0, &TSpike_MRAO}
				});
		}
		for (int i = 0; i < bbCollection.size(); i++)
		{
			DSBoundingBox.push_back(DescriptorSet());
			DSBoundingBox[i].init(this, &DSLBoundingBox, {
						{0, UNIFORM, sizeof(BoundingBoxUniformBlock), nullptr},
				});
		}
		DSSkyBox.init(this, &DSLSkyBox, {
				{0, UNIFORM, sizeof(SkyBoxUniformBlock), nullptr},
				{1, TEXTURE, 0, &TSkyBoxDay},
				{2, TEXTURE, 0, &TSkyBoxSunrise},
				{3, TEXTURE, 0, &TSkyBoxNight}
			});
		DSText.init(this, &DSLOverlay, {
				{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
				{1, TEXTURE, 0, &TText}
			});
		for (int i = 0; i < 4; i++)
		{
			DSHUD[i].init(this, &DSLOverlay, {
					{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
					{1, TEXTURE, 0, &THUD[i]}
				});
		}
		DSGubo.init(this, &DSLGubo, {
					{0, UNIFORM, sizeof(GlobalUniformBlock), nullptr}
				});
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PMesh.cleanup();
		PMeshSway.cleanup();
		PMeshNorm.cleanup();
		PGround.cleanup();
		PSkyBox.cleanup();
		POverlay.cleanup();
		PDebug.cleanup();

		// Cleanup datasets
		for (int i = 0; i < GRASS_INSTANCES; i++)
		{
			DSGrass[i].cleanup();
		}
		for (int i = 0; i < BIRCH_INSTANCES; i++)
		{
			DSBirch[i].cleanup();
		}
		for (int i = 0; i < PINE_INSTANCES; i++)
		{
			DSPine[i].cleanup();
		}
		for (int i = 0; i < TREE_INSTANCES; i++)
		{
			DSTree[i].cleanup();
		}
		for (int i = 0; i < ROCK_INSTANCES; i++)
		{
			DSRock[i].cleanup();
		}
		for (int i = 0; i < WALL_INSTANCES; i++)
		{
			DSWall[i].cleanup();
		}
		for (int i = 0; i < SCROLL_INSTANCES; i++)
		{
			DSScroll[i].cleanup();
		}
		for (int i = 0; i < GPLANT_INSTANCES; i++)
		{
			DSGplant[i].cleanup();
		}
		for (int i = 0; i < FLOAT_INSTANCES; i++)
		{
			DSFloat[i].cleanup();
		}
		for (int i = 0; i < SPIKE_INSTANCES; i++)
		{
			DSSpike[i].cleanup();
		}
		for (int i = 0; i < TERRAIN_INSTANCES; i++)
		{
			DSTerrain[i].cleanup();
		}
		for (int i = 0; i < bbCollection.size(); i++)
		{
			DSBoundingBox[i].cleanup();
		}
		DSShroom.cleanup();
		DSSkyBox.cleanup();
		DSText.cleanup();
		for (int i = 0; i < 4; i++)
		{
			DSHUD[i].cleanup();
		}
		DSGubo.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		TGrass.cleanup();
		TGrass_MRAO.cleanup();
		TTerrain.cleanup();
		TTerrain2.cleanup();
		TTerrain_n.cleanup();
		TTerrain2_n.cleanup();
		TTerrain_MRAO.cleanup();
		TTerrain2_MRAO.cleanup();
		TTerrainNoise.cleanup();
		TTerrainHeight.cleanup();
		TShroom.cleanup();
		TShroom_MRAO.cleanup();
		TBirch.cleanup();
		TBirch_MRAO.cleanup();
		TPine.cleanup();
		TPine_MRAO.cleanup();
		TTree.cleanup();
		TTree_MRAO.cleanup();
		for (int i = 0; i < ROCK_MODELS; i++)
		{
			TRock[i].cleanup();
			TRock_n[i].cleanup();
			TRock_MRAO[i].cleanup();
		}
		TWall.cleanup();
		TWall_n.cleanup();
		TWall_MRAO.cleanup();
		TScroll.cleanup();
		TScroll_n.cleanup();
		TScroll_MRAO.cleanup();
		TGplant.cleanup();
		TGplant_MRAO.cleanup();
		TGplant_e.cleanup();
		TFloat.cleanup();
		TFloat_MRAO.cleanup();
		TSpike.cleanup();
		TSpike_n.cleanup();
		TSpike_MRAO.cleanup();
		TSkyBoxDay.cleanup();
		TSkyBoxSunrise.cleanup();
		TSkyBoxNight.cleanup();
		TText.cleanup();
		for (int i = 0; i < 4; i++)
		{
			THUD[i].cleanup();
		}
		TDummy_b.cleanup();

		
		// Cleanup models
		MTerrain.cleanup();
		for (int i = 0; i < GRASS_MODELS; i++) {
			MGrass[i].cleanup();
		}
		MShroom.cleanup();
		MBirch.cleanup();
		MPine.cleanup();
		MTree.cleanup();
		for (int i = 0; i < ROCK_MODELS; i++)
		{
			MRock[i].cleanup();
		}
		MWall.cleanup();
		MScroll.cleanup();
		MGplant.cleanup();
		MFloat.cleanup();
		MSpike.cleanup();
		for (int i = 0; i < bbCollection.size(); i++)
		{
			MBoundingBox[i].cleanup();
		}
		MText.cleanup();
		for (int i = 0; i < 4; i++)
		{
			MHUD[i].cleanup();
		}
		MSkyBox.cleanup();

		// Cleanup descriptor set layouts
		DSLMesh.cleanup();
		DSLMeshNorm.cleanup();
		DSLTerrain.cleanup();
		DSLOverlay.cleanup();
		DSLSkyBox.cleanup();
		DSLBoundingBox.cleanup();
		DSLGubo.cleanup();
		
		// Destroies the pipelines
		PMesh.destroy();
		PMeshSway.destroy();
		PMeshNorm.destroy();
		PGround.destroy();
		PSkyBox.destroy();
		POverlay.destroy();
		PDebug.destroy();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		DSGubo.bind(commandBuffer, PMesh, 0, currentImage);

		PGround.bind(commandBuffer);

		for (int i = 0; i < TERRAIN_INSTANCES; i++)
		{
			MTerrain.bind(commandBuffer);
			DSTerrain[i].bind(commandBuffer, PGround, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MTerrain.indices.size()), 1, 0, 0, 0);
		}

		PMesh.bind(commandBuffer);

		MShroom.bind(commandBuffer);
		DSShroom.bind(commandBuffer, PMesh, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MShroom.indices.size()), 1, 0, 0, 0);
		for (int i = 0; i < FLOAT_INSTANCES; i++) {
			MFloat.bind(commandBuffer);
			DSFloat[i].bind(commandBuffer, PMeshSway, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MFloat.indices.size()), 1, 0, 0, 0);
		}

		PMeshSway.bind(commandBuffer);
		for (int i = 0; i < GRASS_INSTANCES; i++){
			for (int j = 0; j < GRASS_MODELS; j++) {
				MGrass[j].bind(commandBuffer);
				DSGrass[i].bind(commandBuffer, PMeshSway, 1, currentImage);
				vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(MGrass[j].indices.size()), 1, 0, 0, 0);
			}
		}
		for (int i = 0; i < BIRCH_INSTANCES; i++) {
			MBirch.bind(commandBuffer);
			DSBirch[i].bind(commandBuffer, PMeshSway, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MBirch.indices.size()), 1, 0, 0, 0);
		}
		for (int i = 0; i < TREE_INSTANCES; i++) {
			MTree.bind(commandBuffer);
			DSTree[i].bind(commandBuffer, PMeshSway, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MTree.indices.size()), 1, 0, 0, 0);
		}
		for (int i = 0; i < PINE_INSTANCES; i++) {
			MPine.bind(commandBuffer);
			DSPine[i].bind(commandBuffer, PMeshSway, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MPine.indices.size()), 1, 0, 0, 0);
		}
		for (int i = 0; i < GPLANT_INSTANCES; i++) {
			MGplant.bind(commandBuffer);
			DSGplant[i].bind(commandBuffer, PMeshSway, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MGplant.indices.size()), 1, 0, 0, 0);
		}

		PMeshNorm.bind(commandBuffer);
		for (int i = 0; i < ROCK_INSTANCES; i++) {
			MRock[rockType[i]].bind(commandBuffer);
			DSRock[i].bind(commandBuffer, PMeshNorm, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MRock[rockType[i]].indices.size()), 1, 0, 0, 0);
		}
		for (int i = 0; i < WALL_INSTANCES; i++) {
			MWall.bind(commandBuffer);
			DSWall[i].bind(commandBuffer, PMeshNorm, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MWall.indices.size()), 1, 0, 0, 0);
		}
		for (int i = 0; i < SCROLL_INSTANCES; i++) {
			MScroll.bind(commandBuffer);
			DSScroll[i].bind(commandBuffer, PMeshNorm, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MScroll.indices.size()), 1, 0, 0, 0);
		}
		for (int i = 0; i < SPIKE_INSTANCES; i++) {
			MSpike.bind(commandBuffer);
			DSSpike[i].bind(commandBuffer, PMeshNorm, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MSpike.indices.size()), 1, 0, 0, 0);
		}

		PSkyBox.bind(commandBuffer);
		MSkyBox.bind(commandBuffer);
		DSGubo.bind(commandBuffer, PSkyBox, 0, currentImage);
		DSSkyBox.bind(commandBuffer, PSkyBox, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MSkyBox.indices.size()), 1, 0, 0, 0);

		PDebug.bind(commandBuffer);
		for (int i = 0; i < bbCollection.size(); i++)
		{
			MBoundingBox[i].bind(commandBuffer);
			DSBoundingBox[i].bind(commandBuffer, PDebug, 0, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MBoundingBox[i].indices.size()), 1, 0, 0, 0);
		}

		POverlay.bind(commandBuffer);
		MText.bind(commandBuffer);
		DSText.bind(commandBuffer, POverlay, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MText.indices.size()), 1, 0, 0, 0);
		
		for (int i = 0; i < 4; i++)
		{
			MHUD[i].bind(commandBuffer);
			DSHUD[i].bind(commandBuffer, POverlay, 0, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MHUD[i].indices.size()), 1, 0, 0, 0);

		}
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		auto currTime = std::chrono::high_resolution_clock::now();
		float elapsed_seconds = std::chrono::duration<float, std::chrono::seconds::period>
			(currTime - startTime).count();

		// Integration with the timers and the controllers
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool jump = false;
		bool start = false;
		getSixAxis(deltaT, m, r, jump, start);
		
		glm::mat4 World;
		glm::mat4 Prj;
		glm::mat4 View;

		int toShow = gameState != GAME_STATE_START_SCREEN ? 1 : 0;
		if (gameState == GAME_STATE_START_SCREEN) {
			freqDay = 0.15f;

			float elapsedSecondsLastPress = std::chrono::duration<float, std::chrono::seconds::period>
				(currTime - lastPressTime).count();
			if (start && elapsedSecondsLastPress >= minimumPressDelay) {
				lastPressTime = currTime;
				gameState = GAME_STATE_PLAY;
			}
			glm::vec3 CamPos = glm::vec3(0, 0, 5); //Starting camera position, now we find actual position by rotating first and then translating
			glm::mat4 CamTransform =
				glm::translate(glm::mat4(1.0f), glm::vec3(0)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(-25.0f), glm::vec3(-1, 0, 0));
			CamPos = CamTransform * glm::vec4(CamPos.x, CamPos.y, CamPos.z, 1);

			View = glm::lookAt(CamPos, glm::vec3(0), glm::vec3(0, 1, 0));
			
			Prj = glm::perspective(glm::radians(75.0f), Ar, 0.1f, 100.0f);
			Prj[1][1] *= -1;

			World = glm::translate(glm::mat4(1), glm::vec3(-3, 0, -0.5)) *
					glm::rotate(glm::mat4(1), glm::radians(180.f), glm::vec3(0, 1, 0)) *
					glm::scale(glm::mat4(1), glm::vec3(1.f));
			uboShroom.amb = 0.4f; uboShroom.roughOff = 0.1f; uboShroom.aoOff = 0.0f; uboShroom.sColor = glm::vec3(1.0f);
			uboShroom.mvpMat = Prj * View * World;
			uboShroom.mMat = World;
			uboShroom.nMat = glm::inverse(glm::transpose(World));
			DSShroom.map(currentImage, &uboShroom, sizeof(uboShroom), 0);
		}
		else if (gameState == GAME_STATE_PLAY) {
			freqDay = 0.025f;
			
			float elapsedSecondsLastPress = std::chrono::duration<float, std::chrono::seconds::period>
				(currTime - lastPressTime).count();
			if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && elapsedSecondsLastPress >= minimumPressDelay) {
				lastPressTime = currTime;
				DEBUG = DEBUG == 1? 0 : 1;
			}
			//Ask controller to move player character and camera based on inputs
			//Update immediately the charachter to better check for enemy collisions
			controller.moveCharacterAndCamera(Ar, deltaT, m, r, jump, bbCollection);
			Prj = controller.getPrj();
			View = controller.getView();

			World = controller.getWorld();
			uboShroom.amb = 0.4f; uboShroom.roughOff = 0.1f; uboShroom.aoOff = 0.0f; uboShroom.sColor = glm::vec3(1.0f);
			uboShroom.mvpMat = Prj * View * World;
			uboShroom.mMat = World;
			uboShroom.nMat = glm::inverse(glm::transpose(World));
			DSShroom.map(currentImage, &uboShroom, sizeof(uboShroom), 0);
			//Update moving objects
			for (int i = 0; i < SPIKE_INSTANCES; i++) {
				spikeEnemy[i].moveEnemy(deltaT);
				if (spikeEnemy[i].checkCollision(bsShroom))
					controller.onHit(elapsed_seconds);
			}
			if (controller.getHealth() == 0) {
				gameState = GAME_STATE_GAME_OVER;
				controller.initGameOver(elapsed_seconds);
			}
			for (int i = 0; i < SCROLL_INSTANCES; i++) {
				scroll[i].updateScroll(deltaT, elapsed_seconds);
				if (scroll[i].checkCollision(bsShroom))
					scroll[i].getScroll(elapsed_seconds);
			}
		}
		else if (gameState == GAME_STATE_GAME_OVER) {
			if (controller.gameOver(elapsed_seconds)) {
				reset();
				gameState = 0;
			}
			else
			{
				Prj = controller.getPrj();
				View = controller.getView();

				World = controller.getWorld();
				uboShroom.amb = 0.4f; uboShroom.roughOff = 0.1f; uboShroom.aoOff = 0.0f; uboShroom.sColor = glm::vec3(1.0f);
				uboShroom.mvpMat = Prj * View * World;
				uboShroom.mMat = World;
				uboShroom.nMat = glm::inverse(glm::transpose(World));
				DSShroom.map(currentImage, &uboShroom, sizeof(uboShroom), 0);
			}
		}


		gubo.DlightDir = glm::rotate(glm::mat4(1), elapsed_seconds * freqDay + 3.2f*(3.14f/2.f), glm::vec3(-1,0,-1)) * glm::normalize(glm::vec4(0, 1, 0, 0));
		gubo.DlightColor = glm::vec3(gubo.DlightDir.y >= 0 ? gubo.DlightDir.y : 0);
		gubo.AmbLightColor = glm::vec3(0.35f);
		gubo.eyePos = controller.getCamPos();

		gubo.plantPointPos0 = gplantPosition[0] + glm::vec3(-0.2, 1, 0);
		gubo.plantPointPos1 = gplantPosition[1] + glm::vec3(-0.2, 1, 0);
		gubo.plantPointPos2 = gplantPosition[2] + glm::vec3(-0.2, 1, 0);
		gubo.plantPointColor = (glm::vec3(119, 160, 201) / 255.f) * (float)toShow;
		gubo.plantPointDist = 1.f;
		gubo.plantPointDecay = 0.6f;
		DSGubo.map(currentImage, &gubo, sizeof(gubo), 0);

		for (int i = 0; i < GRASS_INSTANCES; i++)
		{
			World = glm::translate(glm::mat4(1), grassPosition[i]) *
					glm::rotate(glm::mat4(1), glm::radians(grassRot[i]), glm::vec3(0, 1, 0)) *
					glm::scale(glm::mat4(1), grassScale[i] * (float)toShow);
			uboGrass[i].amb = 0.2f; uboGrass[i].roughOff = 0.3f; uboGrass[i].aoOff = 0.0f; uboGrass[i].sColor = glm::vec3(1.0f); 
			uboGrass[i].time = (elapsed_seconds + grassSwayRand[i])*2; uboGrass[i].ratio_xz = 0.5f; uboGrass[i].scale = 50; uboGrass[i].offset = 0;
			uboGrass[i].mvpMat = Prj * View * World;
			uboGrass[i].mMat = World;
			uboGrass[i].nMat = glm::inverse(glm::transpose(World));
			DSGrass[i].map(currentImage, &uboGrass[i], sizeof(uboGrass[i]), 0);
		}
		for (int i = 0; i < BIRCH_INSTANCES; i++)
		{
			World = glm::translate(glm::mat4(1), birchPosition[i]) *
					glm::rotate(glm::mat4(1), glm::radians(birchRot[i]), glm::vec3(0, 1, 0)) *
					glm::scale(glm::mat4(1), birchScale[i] * ((i == 0 || i == 1 || i == 2) ? 1 - (float)toShow : (float)toShow));
			uboBirch[i].amb = 0.2f; uboBirch[i].roughOff = 0.0f; uboBirch[i].aoOff = 0.0f; uboBirch[i].sColor = glm::vec3(1.0f); 
			uboBirch[i].time = elapsed_seconds + birchSwayRand[i]; uboBirch[i].ratio_xz = 0.7f; uboBirch[i].scale = 6; uboBirch[i].offset = 0.2f;
			uboBirch[i].mvpMat = Prj * View * World;
			uboBirch[i].mMat = World;
			uboBirch[i].nMat = glm::inverse(glm::transpose(World));
			DSBirch[i].map(currentImage, &uboBirch[i], sizeof(uboBirch[i]), 0);
		}
		for (int i = 0; i < TREE_INSTANCES; i++)
		{
			World = glm::translate(glm::mat4(1), treePosition[i]) *
					glm::rotate(glm::mat4(1), glm::radians(treeRot[i]), glm::vec3(0, 1, 0)) *
					glm::scale(glm::mat4(1), treeScale[i] * (float)toShow);
			uboTree[i].amb = 0.2f; uboTree[i].roughOff = 0.0f; uboTree[i].aoOff = 0.0f; uboTree[i].sColor = glm::vec3(1.0f);
			uboTree[i].time = elapsed_seconds + treeSwayRand[i]; uboTree[i].ratio_xz = 0.5f; uboTree[i].scale = 8; uboTree[i].offset = 0.5;
			uboTree[i].mvpMat = Prj * View * World;
			uboTree[i].mMat = World;
			uboTree[i].nMat = glm::inverse(glm::transpose(World));
			DSTree[i].map(currentImage, &uboTree[i], sizeof(uboTree[i]), 0);
		}
		for (int i = 0; i < PINE_INSTANCES; i++)
		{
			World = glm::translate(glm::mat4(1), pinePosition[i]) *
					glm::rotate(glm::mat4(1), glm::radians(pineRot[i]), glm::vec3(0, 1, 0)) *
					glm::scale(glm::mat4(1), pineScale[i] * (float)toShow);
			uboPine[i].amb = 0.2f; uboPine[i].roughOff = 0.0f; uboPine[i].aoOff = 0.0f; uboPine[i].sColor = glm::vec3(1.0f);
			uboPine[i].time = elapsed_seconds + pineSwayRand[i]; uboPine[i].ratio_xz = 0.7f; uboPine[i].scale = 9; uboPine[i].offset = 0.2f;
			uboPine[i].mvpMat = Prj * View * World;
			uboPine[i].mMat = World;
			uboPine[i].nMat = glm::inverse(glm::transpose(World));
			DSPine[i].map(currentImage, &uboPine[i], sizeof(uboPine[i]), 0);
		}
		for (int i = 0; i < ROCK_INSTANCES; i++)
		{
			World = glm::translate(glm::mat4(1), rockPosition[i]) *
					glm::rotate(glm::mat4(1), glm::radians(rockRot[i]), glm::vec3(0, 1, 0))*
					glm::scale(glm::mat4(1), rockScale[i] * ( i == 0? 1-(float)toShow : (float)toShow));
			uboRock[i].amb = 0.2f; uboRock[i].roughOff = 0.5f; uboRock[i].aoOff = 0.7f; uboRock[i].sColor = glm::vec3(1.0f);
			uboRock[i].mvpMat = Prj * View * World;
			uboRock[i].mMat = World;
			uboRock[i].nMat = glm::inverse(glm::transpose(World));
			DSRock[i].map(currentImage, &uboRock[i], sizeof(uboRock[i]), 0);
		}
		for (int i = 0; i < WALL_INSTANCES; i++)
		{
			World = glm::translate(glm::mat4(1), wallPosition[i]) *
					glm::rotate(glm::mat4(1), glm::radians(wallRot[i]), glm::vec3(0, 1, 0))*
					glm::scale(glm::mat4(1), wallScale[i] * (float)toShow);
			uboWall[i].amb = 0.2f; uboWall[i].roughOff = 0.1f; uboWall[i].aoOff = 0.5f; uboWall[i].sColor = glm::vec3(1.0f);
			uboWall[i].mvpMat = Prj * View * World;
			uboWall[i].mMat = World;
			uboWall[i].nMat = glm::inverse(glm::transpose(World));
			DSWall[i].map(currentImage, &uboWall[i], sizeof(uboWall[i]), 0);
		}
		for (int i = 0; i < SCROLL_INSTANCES; i++)
		{
			World = gameState != GAME_STATE_START_SCREEN ?
					scroll[i].getWorld() :
					glm::scale(glm::mat4(1), glm::vec3(0));
			uboScroll[i].amb = 0.2f; uboScroll[i].roughOff = 0.1f; uboScroll[i].aoOff = 0.5f; uboScroll[i].sColor = glm::vec3(1.0f);
			uboScroll[i].mvpMat = Prj * View * World;
			uboScroll[i].mMat = World;
			uboScroll[i].nMat = glm::inverse(glm::transpose(World));
			DSScroll[i].map(currentImage, &uboScroll[i], sizeof(uboScroll[i]), 0);
		}
		for (int i = 0; i < GPLANT_INSTANCES; i++)
		{
			World = glm::translate(glm::mat4(1), gplantPosition[i]) *
					glm::rotate(glm::mat4(1), glm::radians(gplantRot[i]), glm::vec3(0, 1, 0)) *
					glm::scale(glm::mat4(1), gplantScale[i] * (float)toShow);
			uboGplant[i].amb = 0.2f; uboGplant[i].roughOff = 0.3f; uboGplant[i].aoOff = 0.5f; uboGplant[i].sColor = glm::vec3(1.0f);
			uboGplant[i].time = elapsed_seconds + gplantSwayRand[i]; uboGplant[i].ratio_xz = 0.5f; uboGplant[i].scale = 10; uboGplant[i].offset = 0;
			uboGplant[i].mvpMat = Prj * View * World;
			uboGplant[i].mMat = World;
			uboGplant[i].nMat = glm::inverse(glm::transpose(World));
			DSGplant[i].map(currentImage, &uboGplant[i], sizeof(uboGplant[i]), 0);
		}
		for (int i = 0; i < FLOAT_INSTANCES; i++)
		{
			World = glm::translate(glm::mat4(1), floatPosition[i]) *
				glm::rotate(glm::mat4(1), glm::radians(floatRot[i]), glm::vec3(0, 1, 0)) *
				glm::scale(glm::mat4(1), floatScale[i] * (float)toShow);
			uboFloat[i].amb = 0.2f; uboFloat[i].roughOff = 0.3f; uboFloat[i].aoOff = 0.5f; uboFloat[i].sColor = glm::vec3(1.0f);
			uboFloat[i].mvpMat = Prj * View * World;
			uboFloat[i].mMat = World;
			uboFloat[i].nMat = glm::inverse(glm::transpose(World));
			DSFloat[i].map(currentImage, &uboFloat[i], sizeof(uboFloat[i]), 0);
		}
		for (int i = 0; i < SPIKE_INSTANCES; i++)
		{
			World = gameState != GAME_STATE_START_SCREEN ?
					spikeEnemy[i].getWorld() :
					glm::scale(glm::mat4(1), glm::vec3(0));
			uboSpike[i].amb = 0.05f; uboSpike[i].roughOff = 0.0f; uboSpike[i].aoOff = 0.5f; uboSpike[i].sColor = glm::vec3(1.0f);
			uboSpike[i].mvpMat = Prj * View * World;
			uboSpike[i].mMat = World;
			uboSpike[i].nMat = glm::inverse(glm::transpose(World));
			DSSpike[i].map(currentImage, &uboSpike[i], sizeof(uboSpike[i]), 0);
		}
		for (int i = 0; i < TERRAIN_INSTANCES; i++)
		{
			World = glm::translate(glm::mat4(1), terrainPosition[i]) *
					glm::rotate(glm::mat4(1), glm::radians(terrainRot[i]), glm::vec3(0, 1, 0)) *
					glm::scale(glm::mat4(1), terrainScale[i] * (float)toShow);
			uboTerrain[i].amb = 0.2f; uboTerrain[i].roughOff = 0.0f; uboTerrain[i].aoOff = 0.0f; uboTerrain[i].sColor = glm::vec3(1.0f);
			uboTerrain[i].mvpMat = Prj * View * World;
			uboTerrain[i].mMat = World;
			uboTerrain[i].nMat = glm::inverse(glm::transpose(World));
			DSTerrain[i].map(currentImage, &uboTerrain[i], sizeof(uboTerrain[i]), 0);
		}

		World = glm::translate(glm::mat4(1), glm::vec3(0,0,0)) * 
				glm::scale(glm::mat4(1), glm::vec3(1));
		//Values calculated via a parabola defined by a known point, a vertex and the a coefficient
		uboSkyBox.dayValue = aCoeffDay * glm::pow(gubo.DlightDir.y - vertexDay.x, 2) + vertexDay.y;
		uboSkyBox.sunriseValue = aCoeffSunrise * glm::pow(gubo.DlightDir.y - vertexSunrise.x, 2) + vertexSunrise.y;
		uboSkyBox.nightValue = aCoeffNight * glm::pow(gubo.DlightDir.y - vertexNight.x, 2) + vertexNight.y;
		uboSkyBox.mvpMat = Prj * View * World;
		uboSkyBox.mMat = World;
		uboSkyBox.nMat = glm::inverse(glm::transpose(World));
		DSSkyBox.map(currentImage, &uboSkyBox, sizeof(uboSkyBox), 0);
	
		uboText.visible = (gameState == GAME_STATE_START_SCREEN) ? 1.0f : 0.0f;
		DSText.map(currentImage, &uboText, sizeof(uboText), 0);

		for (int i = 0; i < 4; i++)
		{
			uboHUD[i].visible = (gameState == GAME_STATE_PLAY && i == controller.getHealth()) ? 1.0f : 0.0f;
			DSHUD[i].map(currentImage, &uboHUD[i], sizeof(uboHUD[i]), 0);
		}

		for (int i = 0; i < bbCollection.size(); i++)
		{
			World = *(bbCollection[i]->mMat);
			World = World * 
					glm::translate(glm::mat4(1), bbCollection[i]->pos) *
					glm::mat4(DEBUG);
			uboBoundingBox[i].mvpMat = Prj * View * World;
			uboBoundingBox[i].mMat = World;
			uboBoundingBox[i].nMat = glm::inverse(glm::transpose(World));
			DSBoundingBox[i].map(currentImage, &uboBoundingBox[i], sizeof(uboBoundingBox[i]), 0);
		}
	}	

	void reset() {
		controller.reset();
		for (int i = 0; i < SCROLL_INSTANCES; i++)
		{
			scroll[i].reset();
		}
		startTime = std::chrono::high_resolution_clock::now();
	}
};




// This is the main: probably you do not need to touch this!
int main() {
    CGProject app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
