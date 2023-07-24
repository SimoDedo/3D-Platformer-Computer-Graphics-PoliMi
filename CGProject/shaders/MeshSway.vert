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

	float time;
	float ratio_xz;
	float scale;
	float offset;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec4 inTan;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 outUV;

void main() {
	float swayIntensity = sin(ubo.time);
	float swayMulitplier = pow( max(inPosition.y - ubo.offset, 0)/ubo.scale, 2);
	float swayValue = swayIntensity * swayMulitplier;
	vec3 sway = vec3(
					swayValue * clamp(ubo.ratio_xz, 0, 1),
					0,
					swayValue * clamp(1-ubo.ratio_xz, 0, 1));

	gl_Position = ubo.mvpMat * vec4(inPosition + sway, 1.0);
	fragPos = (ubo.mMat * vec4(inPosition + sway, 1.0)).xyz;

	fragNorm = (ubo.nMat * vec4(inNorm, 0.0)).xyz;
	outUV = inUV;
}