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

void Camera::update(int width, int height) {
	this->width = width;
	this->height = height;

	float aspect = (float)this->width / (float)this->height;
	this->fovX = 2 * atan(aspect * tan(this->fovY / 2.0f));
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
