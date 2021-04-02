#include "pch.h"
#include "Directional.h"
#include "mymath.h"

Directional::Directional() {
	this->width = 0;
	this->height = 0;

	this->viewFrom = Vector3 { 0, 0, 0 };
	this->viewAt = Vector3{ 0, 0, 0 };

	this->fovY = deg2rad(45);
	this->fovX = deg2rad(45);

	this->tNear = 1.0f;
	this->tFar = 1000.0f;
}

Directional::Directional(Vector3 viewFrom, Vector3 viewAt, float fovY, int width, int height, float tNear, float tFar) {
	this->width = width;
	this->height = height;

	this->viewFrom = viewFrom;
	this->viewAt = viewAt;

	float aspect = (float)this->width / (float)this->height;
	this->fovY = fovY;
	this->fovX = 2 * atan(aspect * tan(this->fovY / 2.0f));

	this->tNear = tNear;
	this->tFar = tFar;
}

Matrix4x4 Directional::getM() {
	return Matrix4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
}

Matrix4x4 Directional::getV() {
	Vector3 z_e = this->getViewFrom() - this->getViewAt();
	z_e.Normalize();
	Vector3 x_e = Vector3{ 0,0,-1 }.CrossProduct(z_e);
	Vector3 y_e = z_e.CrossProduct(x_e);

	return Matrix4x4::EuclideanInverse(Matrix4x4(
		x_e.x, y_e.x, z_e.x, viewFrom.x,
		x_e.y, y_e.y, z_e.y, viewFrom.y,
		x_e.z, y_e.z, z_e.z, viewFrom.z,
		0, 0, 0, 1.0f
	));
}

Matrix4x4 Directional::getP() {
	float a = (this->tNear + this->tFar) / (this->tNear - this->tFar);
	float b = (2 * this->tNear * this->tFar) / (this->tNear - this->tFar);

	float w = 2 * this->tNear * tan(this->fovX / 2);
	float h = 2 * this->tNear * tan(this->fovY / 2);

	auto M = Matrix4x4(
		this->tNear, 0, 0, 0,
		0, this->tNear, 0, 0,
		0, 0, a, b,
		0, 0, -1, 0
	);

	auto N = Matrix4x4(
		2 / w, 0, 0, 0,
		0, 2 / h, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);

	return N * M;
}

Matrix4x4 Directional::getMVP() {
	return this->getP() * this->getV() * this->getM();
}

Matrix4x4 Directional::getMV() {
	return this->getV() * this->getM();
}

Matrix4x4 Directional::getMVn() {
	auto MVn = this->getV() * this->getM();
	MVn.EuclideanInverse();
	MVn.Transpose();

	return MVn;
}