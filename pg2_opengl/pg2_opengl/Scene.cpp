#include "pch.h"
#include "Scene.h"
#include "objloader.h"
#include "matrix3x3.h"
#include "glutils.h"
#include <random>


std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0, 0.99999f);
auto rng = std::bind(distribution, generator);

void Scene::loadMaterials() {
	for (auto material : materials_) {
		GLMaterial m;

		// DIFUSE
		auto tex_diffuse = material->texture(Material::kDiffuseMapSlot);
		if (tex_diffuse) {
			GLuint id = 0;
			CreateBindlessTexture(id, m.tex_diffuse_handle, tex_diffuse->width(), tex_diffuse->height(), tex_diffuse->data());
			m.albedo = Color3f({ 1, 1, 1 });
		} else {
			GLuint id = 0;
			GLubyte data[] = { 255, 255, 255, 255 }; // opaque white
			CreateBindlessTexture(id, m.tex_diffuse_handle, 1, 1, data);
			m.albedo = material->diffuse();
		}

		// ROUGHNESS, METALNESS
		auto tex_rma = material->texture(Material::kRoughnessMapSlot);
		if (tex_rma) {
			GLuint id = 0;
			CreateBindlessTexture(id, m.tex_rma_handle, tex_rma->width(), tex_rma->height(), tex_rma->data());
			m.rma = Color3f({ 1, 1, 1 });
		} else {
			GLuint id = 0;
			GLubyte data[] = { 255, 255, 255, 255 }; // opaque white
			CreateBindlessTexture(id, m.tex_rma_handle, 1, 1, data);
			m.rma = Color3f({ material->roughness_, material->metallicness, 1.0f });
		}
		
		// NORMAL MAP
		auto tex_norm = material->texture(Material::kNormalMapSlot);
		if (tex_norm) {
			GLuint id = 0;
			CreateBindlessTexture(id, m.tex_normal_handle, tex_norm->width(), tex_norm->height(), tex_norm->data());
			m.normal = Color3f({ 1, 1, 1 });
		} else {
			GLuint id = 0;
			GLubyte data[] = { 255, 255, 255, 255 }; // opaque white
			CreateBindlessTexture(id, m.tex_normal_handle, 1, 1, data);
			m.normal = Color3f({ 0, 0, 1 });
		}

		this->pointerIndexMap.emplace(material, this->materials.size());
		this->materials.push_back(m);
	}
}

void Scene::loadTextures() {
	for (auto surface : surfaces_) {
		int mat_index = this->pointerIndexMap.find(surface->get_material())->second;


		for (int i = 0, k = 0; i < surface->no_triangles(); i++) {
			Triangle& triangle = surface->get_triangle(i);

			for (int j = 0; j < 3; j++, k += 3) {
				const Vertex& vertex = triangle.vertex(j);

				verticies.push_back(MyVertex{
					// position
					vertex.position.x,
					vertex.position.y,
					vertex.position.z,
					// normal
					vertex.normal.x,
					vertex.normal.y,
					vertex.normal.z,
					// tangent
					vertex.tangent.x,
					vertex.tangent.y,
					vertex.tangent.z,
					// texture coordinates
					vertex.texture_coords->u,
					vertex.texture_coords->v,
					// material index
					mat_index
					});
			}
		}
	}

	this->vertex_stride = sizeof(MyVertex);
}

Scene::Scene(std::string& file_name, std::string& background_file) {
	this->background = std::make_shared<Texture3f>(background_file);

	const int no_surfaces = LoadOBJ( file_name.c_str(), surfaces_, materials_ );

	this->loadMaterials();
	this->loadTextures();

	
}

Texture3f Scene::getPerfectIrradianceMap(int width, int height) {
	Texture3f result(width, height);

	for (int x = 0; x < result.width(); x++) {
		float phi = float(x) * 2.0f * M_PI / float(result.width());
		for (int y = 0; y < result.height(); y++) {
			float theta = float(y) * M_PI / float(result.height());

			float u = phi / (2 * M_PI);
			float v = theta / (M_PI);

			result.data()[size_t(x) + size_t(y) * size_t(result.width())] = this->background->texel(u, v);
		}
	}
	//result.Save("D:\\prg\\cpp\\save_test.exr");
	
	return result;
}

Vector3 rotateVector(Vector3 v, Vector3 n) {
	Vector3 o1 = n.CrossProduct(Vector3{ 1, 0, 0 });
	if (o1.DotProduct(o1) < 0.001) {
		o1 = n.CrossProduct(Vector3{ 0, 1, 0 });
	}
	o1.Normalize();
	Vector3 o2 = o1.CrossProduct(n);
	o2.Normalize();
	
	//return Matrix3x3{ n, o2, o1} * v;
	return Matrix3x3{ o1, o2, n } * v;
}

Vector3 getCosLobeVector(float alpha, Vector3 omega_r) {
	float xi_1 = rng();
	float xi_2 = rng();

	float t1 = 2 * float(M_PI) * xi_1;
	float t2 = sqrt(1 - pow(xi_2, 2 / (alpha + 1)));

	Vector3 dir = {
		cos(t1) * t2,
		sin(t1) * t2,
		pow(xi_2, 1 / (alpha + 1))
	};
	dir.Normalize();

	auto omega_i = rotateVector(dir, omega_r);
	omega_i.Normalize();
	return omega_i;
}

Vector3 getGGXOmega_h(float alpha, Vector3 n) {
	float xi_1 = rng();
	float xi_2 = rng();

	float phi_n = 2 * float(M_PI) * xi_1;
	float theta_n = atan(alpha * sqrt(xi_2 / (1 - xi_2)));

	Vector3 omega_h(phi_n, theta_n);

	return rotateVector(omega_h, n);
}

Vector3 getReflectedVector(Vector3 d, Vector3 n) {
	d.Normalize();
	n.Normalize();
	Vector3 vec = d - 2 * (d.DotProduct(n)) * n;
	return vec;
}

Vector3 getGGXOmega_i(float alpha, Vector3 n, Vector3 omega_o) {	//omega_o -> eye direction
	auto omega_h = getGGXOmega_h(alpha, n);							//omega_h ->
	return getReflectedVector(omega_o * -1, omega_h);
}

Vector3 getCosWeightedSample(Vector3 omega_r) {
	float xi_1 = rng();
	float xi_2 = rng();

	Vector3 dir = {
		float(cos(2 * M_PI * xi_1) * sqrt(1 - xi_2)),
		float(sin(2 * M_PI * xi_1) * sqrt(1 - xi_2)),
		float(sqrt(xi_2))
	};
	dir.Normalize();

	auto omega_i = rotateVector(dir, omega_r);
	omega_i.Normalize();
	return omega_i;
}

Texture3f Scene::getIrradianceMap(int width, int height) {
	Texture3f result(width, height);

	for (int x = 0; x < result.width(); x++) {
		float phi = float(x) * 2.0f * float(M_PI) / float(result.width());

		for (int y = 0; y < result.height(); y++) {
			float theta = float(y) * M_PI / float(result.height());
			
			Vector3 reflectedVector(phi, theta);

			int N = 200;
			Color3f sampleSum({0,0,0});
			for (int i = 0; i < N; i++) {
				auto sph_omega_i = getCosWeightedSample(reflectedVector).getSphericalCoords();
				
				float u = sph_omega_i.first / (2 * M_PI);
				float v = sph_omega_i.second / M_PI;

				sampleSum += this->background->texel(u, v);
			}
			sampleSum *= 1 / float(N);
			result.data()[size_t(x) + size_t(y) * size_t(result.width())] = Color3f::toSRGB(sampleSum);
		}
	}
	return result;
}

Texture3f Scene::getPrefilteredEnvMap(float alpha, int width, int height) {
	Texture3f result(width, height);

	for (int x = 0; x < result.width(); x++) {
		float phi = float(x) * 2.0f * float(M_PI) / float(result.width());

		for (int y = 0; y < result.height(); y++) {
			float theta = float(y) * M_PI / float(result.height());

			Vector3 normal(phi, theta);
			
			int N = 100;
			Color3f sampleSum({ 0,0,0 });
			for (int i = 0; i < N; i++) {
				auto sph_omega_i = getGGXOmega_i(alpha, normal, normal).getSphericalCoords();;
				float u = sph_omega_i.first / (2 * M_PI);
				float v = sph_omega_i.second / M_PI;

				sampleSum += this->background->texel(u, v);
			}
			sampleSum *= 1 / float(N);
			auto tmp = Color3f::toSRGB(sampleSum);
			result.data()[size_t(x) + size_t(y) * size_t(result.width())] = tmp;

		}
	}

	return result;
}

float G(float alpha, float ct_o, float ct_i) {
	float alpha_2 = pow(alpha, 2);
	float top = 2.0f * ct_o * ct_i;

	float b1 = ct_o * sqrt(alpha_2 + (1 - alpha_2) * pow(ct_i, 2));
	float b2 = ct_i * sqrt(alpha_2 + (1 - alpha_2) * pow(ct_o, 2));
	return top / (b1 + b2);
}

Color3f getIntegrationMapValue(float alpha, float ct_o) {
	Vector3 n = { 0.0f, 0.0f, 1.0f };
	Vector3 omega_o = { float(sqrt(1 - pow(ct_o, 2))), 0, ct_o };
	omega_o.Normalize();
	
	float sum_g = 0;
	float sum_r = 0;
	int N = 1000;

	for (int i = 0; i < N; i++) {
		Vector3 omega_h = getGGXOmega_h(alpha, n);
		float ct_h = omega_o.DotProduct(omega_h);
		while (ct_h < 0.0f) {
			omega_h = getGGXOmega_h(alpha, n);
			ct_h = omega_o.DotProduct(omega_h);
		}

		Vector3 omega_i = getReflectedVector(omega_o * -1, omega_h);
		omega_h.Normalize();
		omega_i.Normalize();

		
		float ct_n = n.DotProduct(omega_h);
		float ct_i = n.DotProduct(omega_i);

		float g = G(alpha, ct_o, ct_i);

		auto a = g * ct_h;
		auto b = ct_o * ct_n;
		auto c = pow(1 - ct_h, 5);

		auto tmp_r = (a / b) * (1 - c);
		auto tmp_g = (a / b) * c;
		sum_r += tmp_r < 0 ? 0.0f : tmp_r;
		sum_g += tmp_g < 0 ? 0.0f : tmp_g;
		
	}

	auto result = Color3f({
		sum_r / float(N),
		sum_g / float(N),
		0.0f
		});

	return Color3f::toLinear(result);
}

Texture3f Scene::getIntegrationMap(int width, int height) {
	Texture3f result(width, height);
	
	float dx = 1.0f / float(width - 1);
	float dy = 1.0f / float(height - 1);

	for (int x = 1; x < width; x++) {
		float ct_o = float(x) * dx;
	
		for (int y = 1; y < height; y++) {
			float alpha = float(y) * dy;

			int i = size_t(x) + size_t(y) * size_t(result.width());
			result.data()[i] = getIntegrationMapValue(alpha, ct_o);
		}
	}

	return result;
}

