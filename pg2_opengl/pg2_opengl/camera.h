#ifndef CAMERA_H_
#define CAMERA_H_

#include "vector3.h"
#include "matrix3x3.h"
#include "matrix4x4.h"

/*! \class Camera
\brief A simple pin-hole camera.

\author Tomáš Fabián
\version 1.0
\date 2018-2019
*/
class Camera {
private:
	int width, height;
	float fovX, fovY;
	float tNear, tFar;

	double yaw, pitch;
	Vector3 viewFrom;

	float velocity;

	void calculateFovX();

	Matrix4x4 getM();

	Matrix4x4 getV();

	Matrix4x4 getP();
public:
	Camera(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt, float tNear = 1.0f, float tFar = 1000.0f);
	
	Vector3 getViewFrom() { return this->viewFrom; }

	Vector3 getViewDir();

	Vector3 getViewAt() { return this->getViewFrom() + this->getViewDir(); }

	Matrix4x4 getMVP();

	Matrix4x4 getMV();

	Matrix4x4 getMVn();

	int getWidth() { return this->width; }

	int getHeight() { return this->height; }

	float getVelocity() { return this->velocity;  }

	void setVelocity(float velocity) { this->velocity = velocity; }

	void moveForward();

	void moveBackward();

	void moveLeft();

	void moveRight();

	void adjustYaw(double x);

	void adjustPitch(double y);

};

#endif
