#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	vec3 DlightDir;		// direction of the direct light
	vec3 DlightColor;	// color of the direct light
	vec3 AmbLightColor;	// ambient light
	vec3 eyePos;		// position of the viewer

	vec3 plantPointPos0;
	vec3 plantPointPos1;
	vec3 plantPointPos2;
	vec3 plantPointColor;
	float plantPointDistance;
	float plantPointDecay;
} gubo;

layout(set = 1, binding = 0) uniform UniformBufferObject {
	float dayValue;
	float sunriseValue;
	float nightValue;
	
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 norm;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragPos;

void main() {
	gl_Position = (ubo.mvpMat * vec4(inPosition, 0.0)).xyww;
	fragPos = inPosition;
}