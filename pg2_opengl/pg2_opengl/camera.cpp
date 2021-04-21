#include "pch.h"
#include "camera.h"

constexpr double PI2 = 2 * M_PI;

Camera::Camera(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt, float tNear, float tFar) {
	this->width = width;
	this->height = height;

	float aspect = (float) this->width / (float) this->height;
	this->fovY = fovY;
	this->fovX = 2 * atan(aspect *  tan(this->fovY / 2.0f));

	this->viewFrom = viewFrom;
	auto viewDir = viewAt - viewFrom;
	viewDir.Normalize();

	auto viewDir_sph = viewDir.getSphericalCoords();
	this->yaw = viewDir_sph.first;
	this->pitch = viewDir_sph.second;

	this->tNear = tNear;
	this->tFar = tFar;

	this->velocity = 1.0f;
}

void Camera::update(int width, int height) {
	this->width = width;
	this->height = height;

	float aspect = (float)this->width / (float)this->height;
	this->fovX = 2 * atan(aspect * tan(this->fovY / 2.0f));
}

Vector3 Camera::getViewDir() {
	Vector3 result(yaw, pitch);
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
	auto moveDir = this->getViewDir().CrossProduct(Vector3{ 0,0,1 });
	this->viewFrom += this->velocity * moveDir;
}

void Camera::moveRight() {
	auto moveDir = this->getViewDir().CrossProduct(Vector3{ 0,0,1 });
	this->viewFrom -= this->velocity * moveDir;
}

void Camera::adjustYaw(double x) {
	this->yaw += float(x * M_PI / 10000.0f);
	while (this->yaw < 0) this->yaw += PI2;
	while (this->yaw > PI2) this->yaw -= PI2;
}

void Camera::adjustPitch(double y) {
	this->pitch += float(y * M_PI / 10000.f);
	if (this->pitch < 0) this->pitch = 0.01;
	if (this->pitch > M_PI) this->yaw = M_PI - 0.01;
}
