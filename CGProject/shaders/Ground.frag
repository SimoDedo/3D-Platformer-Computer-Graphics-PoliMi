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
	float amb;
	float roughOff;
	float aoOff;
	vec3 sColor;
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
} ubo;

layout(set = 1, binding = 2) uniform sampler2D tex;
layout(set = 1, binding = 3) uniform sampler2D tex2;
layout(set = 1, binding = 4) uniform sampler2D tex_n;
layout(set = 1, binding = 5) uniform sampler2D tex2_n;
layout(set = 1, binding = 6) uniform sampler2D tex_MRAO;
layout(set = 1, binding = 7) uniform sampler2D tex2_MRAO;
layout(set = 1, binding = 8) uniform sampler2D texNoise;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec4 fragTan;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in vec2 fragUV_Noise;

layout(location = 0) out vec4 outColor;


vec3 Lambert(vec3 V, vec3 N, vec3 L, vec3 Md) {
	return Md * max(dot(L, N), 0);
}

vec3 Phong(vec3 V, vec3 N, vec3 L, vec3 Ms, float gamma){
	return Ms * pow(clamp(dot(V,-reflect(L,N)),0,1), gamma);
}

vec3 OrenNayar(vec3 V, vec3 N, vec3 L, vec3 Md, float sigma) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 L  - light vector (from the light model)
	//vec3 Md - main color of the surface
	//float sigma - Roughness of the model
	float theta_i = acos(dot(L,N));
	float theta_r = acos(dot(V,N));
	float alpha = max(theta_i, theta_r);
	float beta = min(theta_i, theta_r);
	float A = 1.0f - 0.5f *(pow(sigma, 2.0f)/(pow(sigma,2.0f) + 0.33f));
	float B = 0.45f *(pow(sigma, 2.0f)/(pow(sigma,2.0f) + 0.9f));
	vec3 v_i = normalize(L - dot(L,N)*N);
	vec3 v_r = normalize(V - dot(V,N)*N);
	float G = max(0.0f, dot(v_i,v_r));
	vec3 Lgt = Md * clamp(dot(L,N),0.0f,1.0f);

	
	return Lgt * (A + B*G*sin(alpha)*tan(beta));
}

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
	float rho = roughness;
	
	float Pi = 3.1415926535897932384626433832795f;
	vec3 h = normalize(L + V);
	float D = pow(rho, 2.0f) / (Pi * pow(pow(clamp(dot(N,h),0.00001f,1.0f),2.0f) * (pow(rho,2.0f)-1.0f) + 1.0f,2.0f));
	float F = F0 + (1.0f-F0)*pow(1.0f-clamp(dot(V,h),0.00001f,1.0f),5.0f);
	float G = g(N,V,rho)*g(N,L,rho);
	vec3 Ms = vec3(1.0f);

	return (1.0f - metallic)*(Md * clamp(dot(L, N), 0.00001,1.0f)) + metallic * (Ms * ((D*F*G)/(4.0f*clamp(dot(V,N),0.00001,1.0f))));
}

void main() {
	//Calculate from a Noise Texture the ratio in which we should apply tex or tex2 (which are two texture to make terrain more realistic)
	float ratio = texture(texNoise, fragUV_Noise).r;

	//Take and blend MRAO (Metallic, Roughness, Ambient Occlusion) Textures
	vec3 MRAO = texture(tex_MRAO, fragUV).rgb * (1-ratio) + texture(tex2_MRAO, fragUV).rgb * ratio;

	//Calculate Normal from Normal Map by blending the two normal maps using TBN matrix 
	vec3 Norm = normalize(fragNorm);
	vec3 Tan = normalize(fragTan.xyz); // - Norm * dot(fragTan.xyz, Norm) ?? 
	vec3 Bitan = cross(Norm, Tan) * fragTan.w;
	mat3 tbn = mat3(Tan, Bitan, Norm);
	vec4 nMap = texture(tex_n, fragUV);
	vec4 nMap2 = texture(tex2_n, fragUV);
	vec3 N = normalize(tbn * ((nMap.rgb * 2.0 - 1.0) *  (1-ratio) + (nMap2.rgb * 2.0 - 1.0) * ratio)); // surface normal

	vec3 V = normalize(gubo.eyePos - fragPos);	// viewer direction

	//We obviously also blend the main color
	vec3 albedo =  texture(tex, fragUV).rgb*(1-ratio) + texture(tex2, fragUV).rgb*ratio;	// main color by combining two textures based on noise
	vec3 MD = albedo;
	vec3 MS = ubo.sColor;
	
	//Ambient Light
	vec3 MA = albedo * ubo.amb * (ubo.aoOff + MRAO.b);
	vec3 LA = gubo.AmbLightColor;
	vec3 Ambient =  MA * LA;

	//Direct Light
	vec3 DirectLD = normalize(gubo.DlightDir);
	vec3 DirectLM = gubo.DlightColor;

	vec3 DirectBRDF = GGX(V, N, DirectLD, MD, 0.5f, MRAO.r, clamp(MRAO.g + ubo.roughOff, 0.0f, 1.0f));

	vec3 Direct =  DirectLM * DirectBRDF;

	//Point Lights for Plants
	vec3 PlantPoint[3];

	vec3 PlantPointLD = normalize(gubo.plantPointPos0 - fragPos);
	vec3 PlantPointLM = gubo.plantPointColor.rgb * pow(gubo.plantPointDistance / length(gubo.plantPointPos0 - fragPos), gubo.plantPointDecay);

	vec3 PlantPointBRDF = GGX(V, N, PlantPointLD, MD, 0.5f, MRAO.r, clamp(MRAO.g + ubo.roughOff, 0.0f, 1.0f));

	PlantPoint[0] =  PlantPointLM * PlantPointBRDF;

	PlantPointLD = normalize(gubo.plantPointPos1 - fragPos);
	PlantPointLM = gubo.plantPointColor.rgb * pow(gubo.plantPointDistance / length(gubo.plantPointPos1 - fragPos), gubo.plantPointDecay);

	PlantPointBRDF = GGX(V, N, PlantPointLD, MD, 0.5f, MRAO.r, clamp(MRAO.g + ubo.roughOff, 0.0f, 1.0f));

	PlantPoint[1] =  PlantPointLM * PlantPointBRDF;

	PlantPointLD = normalize(gubo.plantPointPos2 - fragPos);
	PlantPointLM = gubo.plantPointColor.rgb * pow(gubo.plantPointDistance / length(gubo.plantPointPos2 - fragPos), gubo.plantPointDecay);

	PlantPointBRDF = GGX(V, N, PlantPointLD, MD, 0.5f, MRAO.r, clamp(MRAO.g + ubo.roughOff, 0.0f, 1.0f));

	PlantPoint[2] =  PlantPointLM * PlantPointBRDF;

	outColor = clamp(vec4(Direct + Ambient + PlantPoint[0] + PlantPoint[1] + PlantPoint[2], 1.0f), 0.0f, 1.0f);
	// For the roughness of the GGX model we allow an offset to be set. This is because software generating roughness texture tend to have very low
	// values that lead to a very reflective surface. This could be tweaked directly in the softare to generate a better texture, but it would
	// be much slower to fine tune.
}