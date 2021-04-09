#version 460 core
#define M_PI 3.1415926535897932384626433832795

in vec3 v_normal;
in vec3 unified_normal_es;
in vec3 position_lcs;
in vec3 omega_o_es;
in vec3 omega_o;

uniform sampler2D irradiance_map;
uniform sampler2D prefilteredEnv_map;
uniform sampler2D integration_map;
uniform sampler2D shadow_map;

out vec4 FragColor;

float getShadow(float bias = 0.001f, const int r = 10) {
	vec2 shadow_texel_size = 1.0f / textureSize(shadow_map, 0);
	float shadow = 0.0f;

	for (int y = -r; y <= r; ++y) {
		for (int x = -r; x <= r; ++x) {
			vec2 a_tc = (position_lcs.xy + vec2(1.0f)) * 0.5f;
			a_tc += vec2(x, y) * shadow_texel_size;
			float depth = texture(shadow_map, a_tc).r * 2.0f - 1.0f;
			shadow += (depth + bias >= position_lcs.z) ? 1.0f : 0.25f;
		}
	}
	shadow *= 1.0f / ((2 * r + 1) * (2 * r + 1));

	return shadow;
}

vec2 getUV(vec3 v) {
	float p = atan(v.y, v.x);
	float phi = (p < 0) ? p + 2 * M_PI : p;
	float theta = acos(v.z);

	return vec2(phi / (2 * M_PI), theta / M_PI);
}

vec3 getIrradiance() {
	vec2 uv = getUV(v_normal);

	return texture(irradiance_map, uv).rgb;
}

vec3 getPrefEnv(float alpha) {
	const float maxLevel = 8;
	
	float x = (log(alpha) + 7.0f) / 7.0f;
	vec3 omega_i = omega_o - 2 * dot(omega_o, v_normal) * v_normal;

	return texture(prefilteredEnv_map, getUV(omega_i), x * maxLevel).rgb;
}

vec3 getIntegrationMapVal(float alpha, float ct_o) {
	float x = (log(alpha) + 7.0f) / 7.0f;
	vec2 uv = vec2(ct_o, x);
	return texture(integration_map, uv).rgb;
}

float Fresnell(float ct_o, float n1 = 1.0f, float n2 = 1.0f) {
	if (ct_o == 0) return 0;
	float f_0 = pow((n1-n2)/(n1+n2), 2);
	return f_0 + (1 - f_0) * pow(1 - ct_o, 5);
}

float Fresnell(float ct_o, float n_i) {
	if (ct_o == 0) return 0;
	return n_i + (1 - n_i) * pow(1 - ct_o, 5);
}


vec3 getColorVal() {
	// MATERIAL VALUES
	float alpha = 0.1f;
	float metalness = 0.2f;
	vec3 albedo = vec3(0.95f, 0.50f, 1.0f);
	float ior2 = 4.0;

	// CALCULATED VALUES
	float ct_o = dot(unified_normal_es, omega_o_es);
	float k_s = Fresnell(ct_o, 1.0f, ior2);
	float k_d = (1 - k_s) * (1 - metalness);

	vec3 sb = getIntegrationMapVal(alpha, ct_o);	
	vec3 Ld = albedo * getIrradiance();
	vec3 Lr =  getPrefEnv(alpha);

	return  k_d*Ld + (k_s*sb.x + sb.y) * Lr;
}

void main( void ) {	
	FragColor = vec4(getColorVal(), 1.0f) * getShadow();
	
	//FragColor = vec4(getIrradiance(0.02f), 1.0f) * getShadow();

	//NORMAL SHADER
	//vec3 color  = (unified_normal_es + 1) / 2;
	//FragColor = vec4( color.xyz, 1.0f ) * getShadow();
}
