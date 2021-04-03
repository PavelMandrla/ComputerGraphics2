#version 460 core
#define M_PI 3.1415926535897932384626433832795

in vec3 v_normal;
in vec3 unified_normal_es;
in vec3 position_lcs;

uniform sampler2D irradiance_map;
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

vec3 getIrradiance(float alpha) {
	const float maxLevel = 8;
	float x = (alpha - 1000.0f) / -999.999f;

	float p = atan(v_normal.y, v_normal.x);
	float phi = (p < 0) ? p + 2 * M_PI : p;
	float theta = acos(v_normal.z);

	vec2 uv = vec2(phi / (2 * M_PI), theta / M_PI);

	return texture(irradiance_map, uv, x * maxLevel).rgb;
}

void main( void ) {	

	FragColor = vec4(getIrradiance(0.02f), 1.0f) * getShadow();

	//NORMAL SHADER
	//vec3 color  = (v_normal + 1) / 2;
	//FragColor = vec4( color.xyz, 1.0f ) * getShadow();
}
