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

layout (set=1, binding=0) uniform UniformBufferObject{
	float amb;
	vec3 sColor;
	mat4 vpMat;
}ubo;

layout(set = 1, binding = 2) uniform sampler2D tex;
layout(set = 1, binding = 3) uniform sampler2D tex_MRAO;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;


float g(vec3 n, vec3 a, float rho){
	return 2.0f / (1.0f + sqrt(1.0f+pow(rho,2.0f)*((1-pow(clamp(dot(n,a),0.00001f,1.0f),2.0f))/(pow(clamp(dot(n,a),0.00001,1.0f),2.0f)))));
}


vec3 GGX(vec3 V, vec3 N, vec3 L, vec3 Md, float F0, float metallic, float roughness) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 L  - light vector (from the light model)
	//vec3 Md - main color of the surface
	//float F0 - Base color for the Fresnel term
	//float metallic - parameter that mixes the diffuse with the specular term.
	//                 in particular, parmeter K seen in the slides is: float K = 1.0f - metallic;
	//float roughness - Material roughness (parmeter rho in the slides).
	//specular color Ms is not passed, and implicitely considered white: vec3 Ms = vec3(1.0f);
	//F0=1;
	float rho = roughness;
	
	float Pi = 3.1415926535897932384626433832795f;
	vec3 h = normalize(L + V);
	float D = pow(rho, 2.0f) / (Pi * pow(pow(clamp(dot(N,h),0.00001f,1.0f),2.0f) * (pow(rho,2.0f)-1.0f) + 1.0f,2.0f));
	float F = F0 + (1.0f-F0)*pow(1.0f-clamp(dot(V,h),0.00001f,1.0f),5.0f);
	float G = g(N,V,rho)*g(N,L,rho);
	vec3 Ms = vec3(1.0f);

	return (1.0f - metallic)*(Md * clamp(dot(L, N), 0.00001,1.0f)) + metallic * (Ms * ((D*F*G)/(4.0f*clamp(dot(V,N),0.00001,1.0f))));
}

vec3 Lambert(vec3 V, vec3 N, vec3 L, vec3 Md) {
	return Md * max(dot(L, N), 0);
}

vec3 Phong(vec3 V, vec3 N, vec3 L, vec3 Ms, float gamma){
	return Ms * pow(clamp(dot(V,-reflect(L,N)),0,1), gamma);
}

void main() {
	//Take MRAO (Metallic, Roughness, Ambient Occlusion) Texture
	vec3 MRAO = texture(tex_MRAO, fragUV).rgb;

	//Calculate Normal
	vec3 N = vec3(0,1,0);  					// surface normal. For billboard grass, "up" makes it consistent among clusters

	vec3 V = normalize(gubo.eyePos - fragPos);	// viewer direction

	vec3 albedo =  texture(tex, fragUV).rgb;	// main color by combining two textures based on noise
	vec3 MD = albedo;
	vec3 MS = ubo.sColor;

	//Ambient Light
	vec3 MA = albedo * ubo.amb * (MRAO.b);
	vec3 LA = gubo.AmbLightColor;
	vec3 Ambient =  MA * LA;

	//Direct Light
	vec3 DirectLD = normalize(gubo.DlightDir);
	vec3 DirectLM = gubo.DlightColor;

	vec3 DirectBRDF = Lambert(V, N, DirectLD, MD);// + Phong(V, N, DirectLD, MS, .1f);//GGX(V, N, DirectLD, MD, 0.5f, MRAO.r, clamp(MRAO.g, 0.0f, 1.0f));

	vec3 Direct =  DirectLM * DirectBRDF;

	//Point Lights for Plants
	vec3 PlantPoint[3];

	vec3 PlantPointLD = normalize(gubo.plantPointPos0 - fragPos);
	vec3 PlantPointLM = gubo.plantPointColor.rgb * pow(gubo.plantPointDistance / length(gubo.plantPointPos0 - fragPos), gubo.plantPointDecay);

	vec3 PlantPointBRDF = Lambert(V, N,  PlantPointLD, MD); //+ Phong(V, N,  PlantPointLD, MS, 450.f); //GGX(V, N, PlantPointLD, MD, 0.5f, MRAO.r, clamp(MRAO.g, 0.0f, 1.0f));

	PlantPoint[0] =  PlantPointLM * PlantPointBRDF;

	PlantPointLD = normalize(gubo.plantPointPos1 - fragPos);
	PlantPointLM = gubo.plantPointColor.rgb * pow(gubo.plantPointDistance / length(gubo.plantPointPos1 - fragPos), gubo.plantPointDecay);

	PlantPointBRDF = Lambert(V, N,  PlantPointLD, MD); //+ Phong(V, N,  PlantPointLD, MS, 450.f); //GGX(V, N, PlantPointLD, MD, 0.5f, MRAO.r, clamp(MRAO.g, 0.0f, 1.0f));

	PlantPoint[1] =  PlantPointLM * PlantPointBRDF;

	PlantPointLD = normalize(gubo.plantPointPos2 - fragPos);
	PlantPointLM = gubo.plantPointColor.rgb * pow(gubo.plantPointDistance / length(gubo.plantPointPos2 - fragPos), gubo.plantPointDecay);

	PlantPointBRDF = Lambert(V, N,  PlantPointLD, MD); //+ Phong(V, N,  PlantPointLD, MS, 450.f); //GGX(V, N, PlantPointLD, MD, 0.5f, MRAO.r, clamp(MRAO.g, 0.0f, 1.0f));

	PlantPoint[2] =  PlantPointLM * PlantPointBRDF;

	outColor = clamp(vec4(Direct + Ambient + PlantPoint[0] + PlantPoint[1] + PlantPoint[2], texture(tex, fragUV).a), 0.0f, 1.0f);
}