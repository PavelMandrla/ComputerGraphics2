#include "pch.h"
#include "Scene.h"
#include "objloader.h"
#include "matrix3x3.h"
#include <random>


Scene::Scene(std::string& file_name, std::string& background_file) {
	this->background = std::make_shared<Texture3f>(background_file);

	const int no_surfaces = LoadOBJ( file_name.c_str(), surfaces_, materials_ );

	for ( auto surface : surfaces_ ) {
		for ( int i = 0, k = 0; i < surface->no_triangles(); i++) {
			Triangle & triangle = surface->get_triangle(i);
			
			for (int j = 0; j < 3; j++, k += 3) {
				const Vertex & vertex = triangle.vertex(j);
				//vertex.normal

				verticies.push_back(MyVertex {
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
					vertex.texture_coords->v
					});
			}
			
		}
	}

	this->vertex_stride = sizeof(MyVertex);
}

Texture3f Scene::getIrradianceMap(int width, int height) {
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

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0, 0.99999f);
auto rng = std::bind(distribution, generator);

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

Texture3f Scene::getPrefilteredEnvMap(float alpha, int width, int height) {
	Texture3f result(width, height);

	for (int x = 0; x < result.width(); x++) {
		float phi = float(x) * 2.0f * float(M_PI) / float(result.width());

		for (int y = 0; y < result.height(); y++) {
			float theta = float(y) * M_PI / float(result.height());
			
			Vector3 reflectedVector(phi, theta);

			int N = 100;
			Color3f sampleSum({0,0,0});
			for (int i = 0; i < N; i++) {
				auto sph_omega_i = getCosLobeVector(alpha, reflectedVector).getSphericalCoords();
				//auto sph_omega_i = reflectedVector.getSphericalCoords();
				//auto sph_omega_i = rotateVector(Vector3(0,0,1.0f), reflectedVector).getSphericalCoords();

				
				float u = sph_omega_i.first / (2 * M_PI);
				float v = sph_omega_i.second / M_PI;

				sampleSum += this->background->texel(u, v);
			}
			sampleSum *= 1 / float(N);
			result.data()[size_t(x) + size_t(y) * size_t(result.width())] = Color3f::toLinear(sampleSum);
		}
	}
	result.Save("D:\\prg\\cpp\\save_test.exr");

	return result;
}
