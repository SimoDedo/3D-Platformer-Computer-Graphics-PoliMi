#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set=1, binding=0) uniform UniformBufferObject{
	float amb;
	vec3 sColor;
	mat4 vpMat;
}ubo;

struct InstanceData{
	mat4 mMat;
	mat4 nMat;

	float time;
	float ratio_xz;
	float scale;
	float offset;
};

layout (set = 1, binding = 1) readonly buffer InstanceBuffer{
	InstanceData instances[];
}instanceBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec4 inTan;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 outUV;

void main() {
	InstanceData data = instanceBuffer.instances[gl_InstanceIndex];

	float swayIntensity = sin(data.time);
	float swayMulitplier = pow( max(inPosition.y - data.offset, 0)/data.scale, 2);
	float swayValue = swayIntensity * swayMulitplier;
	vec4 sway = vec4(
					swayValue * clamp(data.ratio_xz, 0, 1),
					0,
					swayValue * clamp(1-data.ratio_xz, 0, 1),
					1);

	//Apply sway after rotation in world matrix, so all grass sways the same direction indepentently
	mat4 swayTr = mat4(
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		sway
	);

	gl_Position = ubo.vpMat * swayTr * data.mMat * vec4(inPosition, 1.0);
	fragPos = (swayTr * data.mMat * vec4(inPosition, 1.0)).xyz;

	fragNorm = (data.nMat * vec4(inNorm, 0.0)).xyz;
	outUV = inUV;
}