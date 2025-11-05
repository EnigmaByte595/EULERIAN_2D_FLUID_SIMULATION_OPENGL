#pragma once

class Fluidsim {
public:
	Fluidsim(int N);
	~Fluidsim();

	void step();
	void addDensity(int x, int y, float amount);
	void addVelocity(int x, int y, float amountX, float amountY);

	float* getDensityArray();

private:
	int N;
	int size;

	float dt;
	float diff;
	float visc;

	float* s;
	float* density;

	float* s;
	float* density;

	float* vx;
	float* vy;

	float* vx0;
	float* vy0;

	void diffuse(int b, float* x, float* x0, float diff, float dt);
	void project(float* velocX, float* velocY, float* p, float* div);
	void advect(int b, float* d, float* d0, float* velocX, float* velocY, float dt);
	void set_bnd(int b, float* x);
};