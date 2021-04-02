#version 460 core

in vec3 unified_normal_es;
in vec3 position_lcs;

uniform sampler2D shadow_map;

out vec4 FragColor;

void main( void ) {
	float bias = 0.001f;

	vec2 a_tc = (position_lcs.xy + vec2(1.0f)) * 0.5f;
	float depth = texture(shadow_map, a_tc).r;
	depth = depth * 2.0f -1.0f; // we need to do the inverse
	float shadow = (depth + bias >= position_lcs.z) ? 1.0f : 0.25f; // 0.25f represents the amount of shadowing

	vec3 color  = (unified_normal_es + 1) / 2;
	//FragColor = vec4( color.xyz, 1.0f ) * shadow;
	FragColor = vec4( 0.5, 0.7, 0.4, 1.0f ) * shadow;
}
