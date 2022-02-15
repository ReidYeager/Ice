#version 450

struct DirectionalLight
{
	vec3 direction;
    vec4 color;
    mat4 matrix;
};

layout(set = 0, binding = 0) uniform GlobalInfo {
    mat4 viewProj;
    DirectionalLight dLight;
} global;
layout(set = 0, binding = 1, input_attachment_index = 0) uniform subpassInput inPosition;
layout(set = 0, binding = 2, input_attachment_index = 1) uniform subpassInput inNormal;
layout(set = 0, binding = 3, input_attachment_index = 2) uniform subpassInput inAlbedo;
layout(set = 0, binding = 4, input_attachment_index = 3) uniform subpassInput inMaps;
layout(set = 0, binding = 5, input_attachment_index = 4) uniform subpassInput inDepth;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outSwapchain;

// ================================================================================================

const float PI = 3.1415926535;

float CalculateAttenuation(vec3 conLinQua, float lightDist)
{
	return 1.0 / (conLinQua.x + conLinQua.y * lightDist + conLinQua.z * lightDist * lightDist);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num/denom;
}

float GeometrySchlickGGX(float NdotV , float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 CalculatePBRLighting(vec3 N, vec3 V, vec3 L, vec3 H, vec3 F0, vec3 albedo, float metallic, float roughness)
{
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, 0.001);

	float NdotL = max(dot(N, L), 0.0);
	return (kD * albedo / PI + specular) * NdotL;
}

void main() {
    vec3 position = subpassLoad(inPosition).rgb;
    vec3 albedo = subpassLoad(inAlbedo).rgb;
    float metallic = subpassLoad(inMaps).r;
    float roughness = subpassLoad(inMaps).g;
    float ao = 1.0;
    vec3 N = subpassLoad(inNormal).rgb;
    vec3 V = -normalize(vec3(global.viewProj[0][3], global.viewProj[1][3], global.viewProj[2][3]) - position);

	vec3 finalLight = vec3(0.0);
	vec3 final = vec3(0.0);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// Directional Light
	vec3 L = normalize(-global.dLight.direction);
	vec3 H = normalize(V + L);
	vec3 radiance = global.dLight.color.rgb * global.dLight.color.w;
	finalLight += CalculatePBRLighting(N, V, L, H, F0, albedo, metallic, roughness) * radiance;

	vec3 ambient = vec3(0.05) * albedo * ao;

	final = ambient + finalLight;

    outSwapchain = vec4(final, 1.0);
}
