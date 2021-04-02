#version 460 core

in vec3 unified_normal_es;
in vec3 position_lcs;

uniform sampler2D shadow_map;

out vec4 FragColor;

void main( void ) {
	float bias = 0.001f;
	vec2 shadow_texel_size = 1.0f / textureSize(shadow_map, 0);
	const int r = 10;
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
	FragColor = vec4( 0.5, 0.7, 0.4, 1.0f ) * shadow;

	//NORMAL SHADER
	//vec3 color  = (unified_normal_es + 1) / 2;
	//FragColor = vec4( color.xyz, 1.0f ) * shadow;
}
