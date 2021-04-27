#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require 

#define M_PI 3.1415926535897932384626433832795

in vec3 v_normal;
in vec3 v_tangent;

in vec3 position_lcs;
in vec3 omega_o_es;
in vec3 omega_o;

in vec2 texCoord;
flat in int mat_index;

uniform sampler2D irradiance_map;
uniform sampler2D prefilteredEnv_map;
uniform sampler2D integration_map;
uniform sampler2D shadow_map;
uniform mat4 mvn;

out vec4 FragColor;

mat3x3 TBN;

struct Material {
	vec3 diffuse;
	uint64_t tex_diffuse;

	vec3 rma;
	uint64_t tex_rma;

	vec3 norm;
	uint64_t tex_norm;
};

layout ( std430, binding = 0) readonly buffer Materials {
	Material materials[]; // only the last member can be unsized array
};

vec3 rotateVector(vec3 v, vec3 n) {
	vec3 o1 =  normalize((abs(n.x) > abs(n.z)) ? vec3(-n.y, n.x, 0.0f) : vec3(0.0f, -n.z, n.y));
	vec3 o2 = normalize(cross(o1, n));
	return mat3(o1,o2,n) * v;
}

mat3x3 getTBN() {
	vec3 n = normalize(v_normal);
	vec3 t = normalize(v_tangent - cross(v_tangent, n) * n);
	vec3 b = cross(n, t);
	return mat3x3(t, b, n);
}

vec3 getAlbedo() {
	vec3 result = materials[mat_index].diffuse.rgb;
	if (result == vec3(1,1,1)) {
		result = texture(sampler2D( materials[mat_index].tex_diffuse), -texCoord).rgb;
	}
	return result;
}

float getReflectiveness() {
	if ( materials[mat_index].rma.rgb == vec3(1,1,1)) {
		return texture(sampler2D(materials[mat_index].tex_rma), -texCoord).r;
	} else {
		return materials[mat_index].rma.r;
	}
}

float getMetalness() {
	if ( materials[mat_index].rma.rgb == vec3(1,1,1)) {
		return texture(sampler2D(materials[mat_index].tex_rma), -texCoord).g;
	} else {
		return materials[mat_index].rma.g;
	}
}

vec3 getNormal_raw() {
	vec3 norm = materials[mat_index].norm.rgb;
	if (norm == vec3(1,1,1)) {
		norm = 2 * texture(sampler2D(materials[mat_index].tex_norm), -texCoord).rgb - vec3(1,1,1);
		return TBN * norm;
	}
	return v_normal;
}

vec3 getNormal_unified() {
	return normalize((mvn * vec4(getNormal_raw().xyz, 0.0f)).xyz);
}

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
	vec2 uv = getUV(getNormal_raw());

	return texture(irradiance_map, uv).rgb;
}

vec3 getPrefEnv(float alpha) {
	const float maxLevel = 8;
	
	float x = (log(alpha) + 7.0f) / 7.0f;
	vec3 omega_i = omega_o - 2 * dot(omega_o, getNormal_raw()) * getNormal_raw();

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
	float alpha = getReflectiveness();
	float ior2 = 4.0;

	// CALCULATED VALUES
	float ct_o = dot(getNormal_unified(), omega_o_es);
	float k_s = Fresnell(ct_o, 1.0f, ior2);
	float k_d = (1 - k_s) * (1 - getMetalness());

	vec3 sb = getIntegrationMapVal(alpha, ct_o);	
	vec3 Ld =  getAlbedo() * getIrradiance();
	vec3 Lr =  getPrefEnv(alpha);

	return  k_d*Ld + (k_s*sb.x + sb.y) * Lr;
}

void main( void ) {	
	TBN = getTBN();
	/*
	float bias = 0.001f;
	vec2 a_tc = ( position_lcs.xy+ vec2( 1.0f ) ) * 0.5f;
	float depth = texture( shadow_map, a_tc).r;
	depth = depth * 2.0f - 1.0f;
	float shadow = ( depth + bias >= position_lcs.z) ? 1.0f : 0.0f;

	FragColor = vec4(getColorVal(), 1.0f) * shadow;
	*/


	//FragColor = vec4(getColorVal(), 1.0f) * getShadow();
	FragColor = vec4(getColorVal(), 1.0f);

	//NORMAL SHADER
	//vec3 color  = (getNormal_unified() + 1) / 2;
	//vec3 color = (getNormal_raw() + 1) / 2;
	//FragColor = vec4( color.xyz, 1.0f );
}
