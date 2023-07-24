#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
	float visible;
} ubo;


layout(set = 0, binding = 1) uniform sampler2D tex;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 color = texture(tex, fragUV);
	outColor = vec4(color.rgb, color.a);
}