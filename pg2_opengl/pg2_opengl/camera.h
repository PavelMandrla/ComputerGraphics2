#ifndef CAMERA_H_
#define CAMERA_H_

#include "vector3.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "Directional.h"


class Camera : public Directional {
private:
	double yaw, pitch;
	float velocity;
public:
	Camera(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt, float tNear = 1.0f, float tFar = 1000.0f);
	
	void update(int width, int height);

	virtual Vector3 getViewFrom() override { return this->viewFrom; }

	Vector3 getViewDir();

	virtual Vector3 getViewAt() override { return this->getViewFrom() + this->getViewDir(); }

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
