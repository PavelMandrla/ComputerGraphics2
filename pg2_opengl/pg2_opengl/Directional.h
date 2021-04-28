#pragma once
#include "vector3.h"
#include "matrix4x4.h"

class Directional {
private:
	Matrix4x4 getM();

	Matrix4x4 getV();

	Matrix4x4 getP();
protected:
	int width, height;
	Vector3 viewFrom, viewAt;
	float fovX, fovY;
	float tNear, tFar;
public:
	Directional();

	Directional(Vector3 viewFrom, Vector3 viewAt, float fovY, int width=100, int height=100, float tNear=1.0f, float tFar=10000.0f);

	Matrix4x4 getMVP();

	Matrix4x4 getMV();

	Matrix4x4 getMVn();

	virtual Vector3 getViewFrom() { return this->viewFrom; }

	virtual Vector3 getViewAt() { return this->viewAt; }

	int getWidth() { return this->width; }

	int getHeight() { return this->height; }
};

