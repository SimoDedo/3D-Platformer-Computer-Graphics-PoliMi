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

layout(set = 1, binding = 1) uniform samplerCube cubeMapDay;
layout(set = 1, binding = 2) uniform samplerCube cubeMapSunrise;
layout(set = 1, binding = 3) uniform samplerCube cubeMapNight;


layout(location = 0) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = clamp(
				texture(cubeMapDay, fragPos + vec3(0,0.15,0)) * clamp(ubo.dayValue,0,0.9) * (1-clamp(ubo.sunriseValue,0,1)) + 
				texture(cubeMapSunrise, fragPos + vec3(0,0.15,0)) * clamp(ubo.sunriseValue,0,1) +
				texture(cubeMapNight, fragPos + vec3(0,0.15,0)) * clamp(ubo.nightValue,0,0.8) 
				,0,1);
}