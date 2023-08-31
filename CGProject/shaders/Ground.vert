#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform UniformBufferObject {
	float amb;
	float roughOff;
	float aoOff;
	vec3 sColor;
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
} ubo;

layout(set = 1, binding = 1) uniform sampler2D texHeight;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec4 inTan;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec2 inUV_Noise;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec4 fragTan;
layout(location = 3) out vec2 outUV;
layout(location = 4) out vec2 outUV_Noise;


void main() {
	//Apply HeightMap to position to have ground shape
	vec4 pos = vec4(inPosition.x, inPosition.y + texture(texHeight, inUV_Noise).r * 0.2f, inPosition.z, 1.0);
	gl_Position = ubo.mvpMat * pos;
	fragPos = (ubo.mMat * pos).xyz;

	fragNorm = normalize((ubo.nMat * vec4(inNorm, 0.0))).xyz;
	fragTan = normalize(ubo.nMat * inTan);

	outUV = inUV;
	outUV_Noise = inUV_Noise;
}