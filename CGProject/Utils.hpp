#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GAME_STATE_START_SCREEN 0
#define GAME_STATE_PLAY 1
#define GAME_STATE_GAME_OVER 2

#define STRETCH_NONE 0
#define STRETCH_JUMPING 1
#define STRETCH_LANDING 2

struct GlobalUniformBlock {
	alignas(16) glm::vec3 DlightDir;
	alignas(16) glm::vec3 DlightColor;
	alignas(16) glm::vec3 AmbLightColor;
	alignas(16) glm::vec3 eyePos;

	alignas(16) glm::vec3 plantPointPos0;
	alignas(16) glm::vec3 plantPointPos1;
	alignas(16) glm::vec3 plantPointPos2;
	alignas(16) glm::vec3 plantPointColor;
	alignas(4) float plantPointDist;
	alignas(4) float plantPointDecay;
};

struct MeshUniformBlock {
	alignas(4) float amb;
	alignas(4) float roughOff;	//Higher values -> Higher roughness -> less focused specular (We simply offset the roughness map closer to 1 for positive values)
	alignas(4) float aoOff;		//Higher values -> Higher less ambient occlusion -> less black on obj (We simply offset the roughness map closer to 1 for positive values)
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct MeshSwayUniformBlock {
	alignas(4) float amb;
	alignas(4) float roughOff;	//Higher values -> Higher roughness -> less focused specular (We simply offset the roughness map closer to 1 for positive values)
	alignas(4) float aoOff;		//Higher values -> Higher less ambient occlusion -> less black on obj (We simply offset the roughness map closer to 1 for positive values)
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;

	alignas(4) float time;		//Time elapsed used for continuous value
	alignas(4) float ratio_xz;	//Ratio between swaying on x and z plane
	alignas(4) float scale;		//Controls how much the object sways
	alignas(4) float offset;	//Controls the height starting point of the swaying effect
};

struct SkyBoxUniformBlock {
	alignas(4) float dayValue;
	alignas(4) float sunriseValue;
	alignas(4) float nightValue;

	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct OverlayUniformBlock {
	alignas(4) float visible;
};

// The vertices data structures
struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec4 tangent;
	glm::vec2 UV;
};

struct VertexTerrain {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec4 tangent;
	glm::vec2 UV;
	glm::vec2 UVNoise;
};

struct VertexSkyBox {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct VertexOverlay {
	glm::vec2 pos;
	glm::vec2 UV;
};

//DEBUG
struct VertexBoundingBox {
	glm::vec3 pos;
};

struct BoundingBoxUniformBlock {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct BoundingBox {
	//Position of bounding box relative to the center of the object
	glm::vec3 pos; 
	//Half lenght of every side of the box
	glm::vec3 halfExtents; 
	//Wheter this bounding box should be considered for collision
	bool canCollide; 
	//Wheter this bounding box should be considered for enemy contact
	bool isEnemy;
	//Pointer to the world matrix in the ubo: when the ubo is modifide, so is the bounding box
	glm::mat4* mMat;
	glm::mat4 *nMat;
};

struct BoundingSphere {
	glm::vec3 pos;
	float radius;
	glm::mat4* mMat;
};

float RandFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)

{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

void CreateBoundingBoxModel(std::vector<VertexBoundingBox>& vDef, std::vector<uint32_t>& vIdx, BoundingBox* bb) {
	vDef.push_back({ {bb->halfExtents.x, bb->halfExtents.y, bb->halfExtents.z} });		//0
	vDef.push_back({ {bb->halfExtents.x, bb->halfExtents.y, -bb->halfExtents.z} });		//1
	vDef.push_back({ {-bb->halfExtents.x, bb->halfExtents.y, bb->halfExtents.z} });		//2
	vDef.push_back({ {-bb->halfExtents.x, bb->halfExtents.y, -bb->halfExtents.z} });	//3
	vDef.push_back({ {bb->halfExtents.x, -bb->halfExtents.y, bb->halfExtents.z} });		//4
	vDef.push_back({ {bb->halfExtents.x, -bb->halfExtents.y, -bb->halfExtents.z} });	//5
	vDef.push_back({ {-bb->halfExtents.x, -bb->halfExtents.y, bb->halfExtents.z} });	//6
	vDef.push_back({ {-bb->halfExtents.x, -bb->halfExtents.y, -bb->halfExtents.z} });	//7

	//top
	vIdx.push_back(0); vIdx.push_back(1); vIdx.push_back(2);
	vIdx.push_back(1); vIdx.push_back(2); vIdx.push_back(3);
	//bottom
	vIdx.push_back(4); vIdx.push_back(5); vIdx.push_back(6);
	vIdx.push_back(5); vIdx.push_back(6); vIdx.push_back(7);
	//l-side
	vIdx.push_back(2); vIdx.push_back(3); vIdx.push_back(6);
	vIdx.push_back(3); vIdx.push_back(6); vIdx.push_back(7);
	//r-side
	vIdx.push_back(0); vIdx.push_back(1); vIdx.push_back(4);
	vIdx.push_back(1); vIdx.push_back(4); vIdx.push_back(5);
	//back
	vIdx.push_back(1); vIdx.push_back(3); vIdx.push_back(5);
	vIdx.push_back(3); vIdx.push_back(5); vIdx.push_back(7);
	//front
	vIdx.push_back(0); vIdx.push_back(2); vIdx.push_back(4);
	vIdx.push_back(2); vIdx.push_back(4); vIdx.push_back(6);
}

//Checks if a ray starting from a point in a certain direction that extends for a given distance collides with an (not necesseraly axis aligned) bounding box.
//We use the slab method for Axis Aligned Bounding Boxes to compute it. 
//We transform the ray to object space by multiplying the ray with the inverse of the world matrix, so we can then apply AABB based algorithm in object space
//If we wanted to retrieve the collision point, it would have to be converted back to world space from object space
bool rayCollision(glm::vec3 rayOrigin, glm::vec3 rayDir, float rayDistance, BoundingBox* obj) {
	glm::vec3 boxPos = obj->pos;
	glm::vec3 he = obj->halfExtents;

	glm::vec3 boxMin = glm::vec3(boxPos.x - he.x, boxPos.y - he.y, boxPos.z - he.z);
	glm::vec3 boxMax = glm::vec3(boxPos.x + he.x, boxPos.y + he.y, boxPos.z + he.z);

	glm::mat4 local = *(obj->mMat);
	local = glm::inverse(local);
	rayOrigin = local * glm::vec4(rayOrigin, 1);
	glm::mat4 localn = *obj->nMat;
	rayDir = localn * glm::vec4(rayDir, 1);
	rayDir = glm::normalize(rayDir);


	double tx1 = (boxMin.x - rayOrigin.x) / rayDir.x;
	double tx2 = (boxMax.x - rayOrigin.x) / rayDir.x;

	double tmin = glm::min(tx1, tx2);
	double tmax = glm::max(tx1, tx2);

	double ty1 = (boxMin.y - rayOrigin.y) / rayDir.y;
	double ty2 = (boxMax.y - rayOrigin.y) / rayDir.y;

	tmin = glm::max(tmin, glm::min(ty1, ty2));
	tmax = glm::min(tmax, glm::max(ty1, ty2));

	double tz1 = (boxMin.z - rayOrigin.z) / rayDir.z;
	double tz2 = (boxMax.z - rayOrigin.z) / rayDir.z;

	tmin = glm::max(tmin, glm::min(tz1, tz2));
	tmax = glm::min(tmax, glm::max(tz1, tz2));

	return tmax >= glm::max(0.0, tmin) && tmin < rayDistance;
}

// Returns the squared distance between a point p and an AABB b
glm::vec3 SqDistPointAABB(glm::vec3 p, BoundingBox bb)
{
	glm::vec3 boxMin = glm::vec3(bb.pos.x - bb.halfExtents.x, bb.pos.y - bb.halfExtents.y, bb.pos.z - bb.halfExtents.z);
	glm::vec3 boxMax = glm::vec3(bb.pos.x + bb.halfExtents.x, bb.pos.y + bb.halfExtents.y, bb.pos.z + bb.halfExtents.z);
	
	glm::vec3 q;
	// For each coordinate axis, if the point coordinate value is
	// outside box, clamp it to the box, else keep it as is
	for (int i = 0; i < 3; i++) {
		float v = p[i];
		if (v < boxMin[i]) v = boxMin[i]; // v = max( v, b.min[i] )
		if (v > boxMax[i]) v = boxMax[i]; // v = min( v, b.max[i] )
		q[i] = v;
	}
	return q;
}
// Returns true if sphere s intersects AABB b, false otherwise
bool sphereBoxCollision(BoundingBox bb, BoundingSphere bs)
{
	glm::vec3 center = (*bs.mMat) * glm::vec4(bs.pos, 1.0f);
	glm::vec3 spherePoint = (*bs.mMat) * glm::vec4(bs.pos + glm::vec3(0,1,0) * bs.radius, 1.0f);
	//Move center in object space so bb can be considered an AABB, and recalculate radius in object space
	center = glm::inverse(*bb.mMat) * glm::vec4(center, 1.0f);
	spherePoint = glm::inverse(*bb.mMat) * glm::vec4(spherePoint, 1.0f);
	float scaledRadius = glm::distance(spherePoint, center);

	glm::vec3 closestPoint = SqDistPointAABB(center, bb);

	// Compute squared distance between sphere center and AABB
	float sqDist = glm::pow(glm::distance(center, closestPoint), 2);

	// Sphere and AABB intersect if the (squared) distance between them is
	// less than the (squared) sphere radius.
	return sqDist <= scaledRadius * scaledRadius;
}

//Checks if the two boxes collide.
//The second box is transformed into the first's objects space and treated as an AABB in that space to simplify matters.
//Does not work well since boxes are not actually considered in their correct position and orientation
bool boxCollision(BoundingBox obj, BoundingBox other, float extraDist, glm::quat otherRot) {
	//Realign other with axis (only done with Y as this function is used only for the character which is only rotated on the Y
	glm::mat4 aligned = (*other.mMat) * glm::mat4(glm::inverse(otherRot));
	glm::vec3 otherPosMin = aligned * glm::vec4(other.pos - other.halfExtents - extraDist, 1.0f);
	glm::vec3 otherPosMax = aligned * glm::vec4(other.pos + other.halfExtents + extraDist, 1.0f);


	glm::mat4 local = *(obj.mMat);
	local = glm::inverse(local);

	glm::vec3 newOtherPosMin = local * glm::vec4(otherPosMin, 1);
	glm::vec3 newOtherPosMax = local * glm::vec4(otherPosMax, 1);

	// collision x-axis?
	bool collisionX = obj.pos.x + obj.halfExtents.x >= newOtherPosMin.x &&
		newOtherPosMax.x >= obj.pos.x - obj.halfExtents.x;
	// collision y-axis?
	bool collisionY = obj.pos.y + obj.halfExtents.y >= newOtherPosMin.y&&
		newOtherPosMax.y >= obj.pos.y - obj.halfExtents.y;
	// collision z-axis?
	bool collisionZ = obj.pos.z + obj.halfExtents.z >= newOtherPosMin.z &&
		newOtherPosMax.z >= obj.pos.z - obj.halfExtents.z;
	// collision only if on all axes
	return collisionX && collisionY && collisionZ;
}

float aCoefficientParabola(glm::vec2 knownPoint, glm::vec2 vertex) {
	return (knownPoint.y - vertex.y) / (glm::pow(knownPoint.x - vertex.x, 2));
}