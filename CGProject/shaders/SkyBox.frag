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
	////Calculate a parabolas
	////We use these parabolas to get how much how each sky we should display 
//
	////night
	//float nightStart = 0.2f;
	//vec2 vertexNight = vec2(-1, 1);
	//vec2 knownPointNight = vec2(nightStart, 0);
	//float aCoeffNight = (knownPointNight.y - vertexNight.y) / pow(knownPointNight.x - vertexNight.x, 2);
	//float nightValue = aCoeffNight * pow(gubo.DlightDir.y -vertexNight.x, 2) + vertexNight.y;
//
	////day
	//float dayStart = -0.4f; //Start very early for smooth transition
	//vec2 vertexDay = vec2(1, 1.2f);
	//vec2 knownPointDay = vec2(dayStart, 0);
	//float aCoeffDay = (knownPointDay.y - vertexDay.y) / pow(knownPointDay.x - vertexDay.x, 2);
	//float dayValue = aCoeffDay * pow(gubo.DlightDir.y -vertexDay.x, 2) + vertexDay.y;
//
	////sunrise
	//float sunriseStart = -0.2f;
	//float sunriseEnd = 0.4f;
	//vec2 vertexSunrise = vec2(sunriseStart + (sunriseEnd-sunriseStart)/2, 1);
	//vec2 knownPointSunrise = vec2(sunriseStart, 0);
	//float aCoeffSunrise = (knownPointSunrise.y - vertexSunrise.y) / pow(knownPointSunrise.x - vertexSunrise.x, 2);
	//float sunriseValue = aCoeffSunrise * pow(gubo.DlightDir.y -vertexSunrise.x, 2) + vertexSunrise.y;


	outColor = clamp(
				texture(cubeMapDay, fragPos+vec3(0,0.15,0)) * clamp(ubo.dayValue,0,0.9) * (1-clamp(ubo.sunriseValue,0,1)) + 
				texture(cubeMapSunrise, fragPos+vec3(0,0.15,0)) * clamp(ubo.sunriseValue,0,1) +
				texture(cubeMapNight, fragPos+vec3(0,0.15,0)) * clamp(ubo.nightValue,0,0.8) 
				,0,1);
}