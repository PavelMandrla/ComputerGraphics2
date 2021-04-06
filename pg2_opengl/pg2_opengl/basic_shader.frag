#version 460 core
#define M_PI 3.1415926535897932384626433832795

in vec3 v_normal;
in vec3 unified_normal_es;
in vec3 position_lcs;
in vec3 v_cameraDir;

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

vec3 getIrradiance(float x) {
	const float maxLevel = 8;
	//float x = (alpha - 1000.0f) / -999.999f;
	vec2 uv = getUV(v_normal);

	return texture(irradiance_map, uv, x * maxLevel).rgb;
}

vec3 getPrefEnv(float alpha) {
	const float maxLevel = 8;
	float x = (log(alpha) + 7.0f) / 7.0f;

	vec3 omega_i = v_cameraDir - 2 * dot(v_cameraDir, unified_normal_es) * unified_normal_es;
	vec2 uv = getUV(omega_i);

	return texture(prefilteredEnv_map, uv, x * maxLevel).rgb;
}

vec3 getIntegrationMapVal(float alpha, float ct_o) {
	vec2 uv = vec2(ct_o, alpha);
	return texture(integration_map, uv).rgb;
}

float Fresnell(float ior, float ct_h) {
	float f_0 = pow((1.0f-ior)/(1.0f+ior), 2);

	return f_0 + (1 - f_0) * pow(1 - ct_h, 5);
}

vec3 getColorVal() {
	float alpha = 0.2f;
	float metalness = 0.54f;
	vec3 albedo = vec3(0.95f, 0.50f, 1.0f);
	float ior = 1.0f;

	float ct_o = dot(v_normal, -1 * v_cameraDir);

	float k_s = Fresnell(ior, ct_o);					// K_s
	float k_d = (1 - k_s) * (1 - metalness);			// K_d
	
	vec3 sb_tmp = getIntegrationMapVal(alpha, ct_o);	
	float s = sb_tmp.x;									// s
	float b = sb_tmp.y;									// b

	vec3 Ld = (albedo / M_PI) * getIrradiance(alpha);
	vec3 Lr = getPrefEnv(alpha);

	//return k_d * Ld;
	return sb_tmp;
	//return k_d*Ld + (k_s*s + b)*Lr;
	//return albedo;
}

void main( void ) {	

	FragColor = vec4(getColorVal(), 1.0f) * getShadow();
	
	//FragColor = vec4(getIrradiance(0.02f), 1.0f) * getShadow();

	//NORMAL SHADER
	//vec3 color  = (v_normal + 1) / 2;
	//FragColor = vec4( color.xyz, 1.0f ) * getShadow();
}
