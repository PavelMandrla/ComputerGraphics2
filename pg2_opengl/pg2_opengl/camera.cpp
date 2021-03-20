#include "pch.h"
#include "camera.h"

Camera::Camera(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt, float tNear, float tFar) {
	this->width = width;
	this->height = height;

	float aspect = (float) this->width / (float) this->height;
	this->fovY = fovY;
	this->fovX = 2 * atan(aspect *  tan(this->fovY / 2.0f));

	this->viewFrom = viewFrom;
	auto viewDir = viewAt - viewFrom;
	viewDir.Normalize();

	this->yaw = atan2(viewDir.y, viewDir.x);
	this->pitch = asin(-viewDir.z);

	this->tNear = tNear;
	this->tFar = tFar;

	this->velocity = 1.0f;
}

void Camera::calculateFovX() {
	float aspect = (float) this->width / (float) this->height;
	this->fovX = 2 * atan(aspect *  tan(this->fovY / 2.0f));
}

Matrix4x4 Camera::getM() {
	return Matrix4x4 (
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
}

Matrix4x4 Camera::getV() {
	Vector3 z_e = this->getViewFrom() - this->getViewAt();
	z_e.Normalize();
	Vector3 x_e = Vector3{ 0,1,0 }.CrossProduct(z_e);
	Vector3 y_e = z_e.CrossProduct(x_e);

	return Matrix4x4::EuclideanInverse( Matrix4x4(
		x_e.x,	y_e.x,	z_e.x,	viewFrom.x,
		x_e.y,	y_e.y,	z_e.y,	viewFrom.y,
		x_e.z,	y_e.z,	z_e.z,	viewFrom.z,
		0,		0,		0,		1.0f
	));
}

Matrix4x4 Camera::getP() {
	float a = (this->tNear + this->tFar) / (this->tNear - this->tFar);
	float b = (2 * this->tNear * this->tFar) / (this->tNear - this->tFar);
	
	float w = 2 * this->tNear * tan(this->fovX / 2);
	float h = 2 * this->tNear * tan(this->fovY / 2);

	auto M = Matrix4x4(
		this->tNear,	0,				0,	0,
		0,				this->tNear,	0,	0,
		0,				0,				a,	b,
		0,				0,				-1,	0
	);

	auto N = Matrix4x4(
		2/w,	0,		0,	0,
		0,		2/h,	0,	0,
		0,		0,		1,	0,
		0,		0,		0,	1
	);

	return N * M;
}

Matrix4x4 Camera::getMVP() {
	return this->getP() * this->getV() * this->getM();
}

Vector3 Camera::getViewDir() {
	auto result = Vector3{
		float(cos(pitch) * cos(yaw)),
		float(sin(pitch) * sin(yaw)),
		float(-sin(pitch))
	};
	result.Normalize();
	return result;
}

void Camera::moveForward() {
	this->viewFrom += this->velocity * this->getViewDir();
}

void Camera::moveBackward() {
	this->viewFrom -= this->velocity * this->getViewDir();
}

void Camera::moveLeft() {
	auto moveDir = this->getViewDir().CrossProduct(Vector3{ 0,1,0 });
	this->viewFrom -= this->velocity * moveDir;
}

void Camera::moveRight() {
	auto moveDir = this->getViewDir().CrossProduct(Vector3{ 0,1,0 });
	this->viewFrom += this->velocity * moveDir;
}

void Camera::adjustYaw(double x) {
	this->yaw -= float(x * M_PI / 10000.0f);
}

void Camera::adjustPitch(double y) {
	this->pitch += float(y * M_PI / 10000.f);
}
