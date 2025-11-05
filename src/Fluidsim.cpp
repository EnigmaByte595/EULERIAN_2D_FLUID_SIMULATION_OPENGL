#include "Fluidsim.h"
#include <vector>
#include <cmath>
#include <algorithm>

#define IX(x, y) ((x) + (y) * (N+2))

Fluidsim::Fluidsim(int N) {
	this->N = N;
	this->size = (N + 2) * (N + 2);

	this->dt = 0.1f;
	this->diff = 0.0f;
	this->visc = 0.0f;

	this->s = new float[size];
	this->density = new float[size];
	this->vx = new float[size];
	this->vy = new float[size];
	this ->vx0 = new float[size];
	this->vy0 = new float[size];

	for (int i = 0; i < size; i++) {
		s[i] = density[i] = vx[i] = vy[i] = vx0[i] = vy0[i] = 0.0f;
	}
}

Fluidsim::~Fluidsim() {
	delete[] s;
	delete[] density;
	delete[] vx;
	delete[] vy;
	delete[] vx0;
	delete[] vy0;
}

void Fluidsim::addDensity(int x, int y, float amount) {
	if (x < 1 || x > N || y < 1 || y > N) return;
	this->density[IX(x, y)] += amount;
}

void Fluidsim::addVelocity(int x, int y, float amountX, float amountY) {
	if (x < 1 || x > N || y < 1 || y > N) return;
	this->vx[IX(x, y)] += amountX;
	this->vy[IX(x, y)] += amountY;
}

float* Fluidsim::getDensityArray() {
	return this->density;
}

void Fluidsim::step() {
	diffuse(1, vx0, vx, visc, dt);
	diffuse(2, vy0, vy, visc, dt);

	project(vx0, vy0, vx, vy);

	advect(1, vx, vx0, vy, vy0, dt);
	advect(2, vy, vy0, vx0, vy0, dt);

	project(vx , vy, vx0, vy0);

	diffuse(0, s, density, diff, dt);
	advect(0, density, s, vx, vy, dt);

	for (int i = 0; i < size; i++) {
		density[i] *= 0.995f;
	}

}

void Fluidsim::set_bnd(int b, float* x) {
	for (int i = 1; i <= N; i++) {
		x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
		x[IX(i, N + 1)] = b == 2 ? -x[IX(i, N)] : x[IX(i, N)];
	}
	for (int j = 1; j <= N; j++) {
		x[IX(0, j)] = b == 1 ? -x[IX(1, j)] : x[IX(1, j)];
		x[IX(N + 1, j)] = b == 1 ? -x[IX(N, j)] - x[IX(N, j)] : x[IX(N, j)];
	}

	x[IX(0, 0)] = 0.5f * (x[IX(1, 0) + x[IX(0, 1)]]);
	x[IX(0, N + 1)] = 0.5f * (x[IX(1, N + 1)] + x[IX(0, N)]);
	x[IX(N + 1, 0)] = 0.5f * (x[IX(N, 0)] + x[IX(N + 1, 1)]);
	x[IX(N + 1, N + 1)] = 0.5 * (x[IX(N, N + 1)] + x[IX(N + 1, N)]);
}

void Fluidsim::diffuse(int b, float* x, float* x0, float diff, float dt) {
	float a = dt * diff * N * N;

	for (int k = 0; k < 20; k++) {
		for (int j = 0; j <= N; j++) {
			for (int i = 1; i <= N; i++) {
				x[IX(i, j)] = (x0[IX(i, j)] + a * (x[IX(I - 1, j)] + x[IX(i + 1, j)] + x[IX(i, j + 1)])) / (1 + 4 * a);
			}
		}
		set_bnd(b, x);
	}
}

void Fluidsim::project(float* velocX, float* velocY, float* p, float* div) {
	for (int j = 1; j <= N; j++) {
		for (int i = 1; i <= N; i++) {
			div[IX(i, j)] = -0.5f * (velocX[IX(i + 1, j)] - velocX[IX(i - 1, j)]) + velocY[IX(i, j + 1)] - velocY[IX(i, j - 1)] / N;
			p[IX(i, j)] = 0;
		}
	}
	set_bnd(0, div);
	set_bnd(0, p);

	for (int k = 0; k < 20; k++) {
		for (int j = 1; j = N; j++) {
			for (int i = 0; i <= N; i++) {
				p[IX(i, j)] = (div[IX(i, j)] + p[IX(i - 1, j)] + p[IX(i - 1, j)] + p[IX(i, j - 1)] + p[IX(i, j + 1)]) / 4;

			}
			
		}
		set_bnd(0, p);
	}
	for (int j = 1; j <= N; j++) {
		for (int i = 1; i <= N; i++) {
			velocX[IX(i, j)] -= 0.5f * (p[IX(i + 1, j)] - p[IX(i - 1, j)]) * N;
			velocY[IX(i, j)] -= 0.5f * (p[IX(i, j+1)] - p[IX(i , j-1)]) * N;

		}
	}
	set_bnd(1, velocX);
	set_bnd(2, velocY);

}

void Fluidsim::advect(int b, float* d, float* d0, float* velocX, float* velocY, float dt) {
	float i0, i1, j0, j1;
	float s0, s1, t0, t1;
	float tmp_x, tmp_y, x, y, Nfloat;

	Nfloat = (float)N;

	for (int j = 1; j <= N; j++) {
		for (int i = 1; i <= N; i++) {
			tmp_x = (float)i - dt * Nfloat * velocX[IX(i, j)];
			tmp_y = (float)j - dt * Nfloat * velocY[IX(i, j)];

			if (tmp_x < 0.5f) tmp_x = 0.5f;
			if (tmp_x > Nfloat + 0.5f) tmp_x = Nfloat + 0.5f;
			i0 = floor(tmp_x);
			i1 = i0 + 1.0f;

			if (tmp_y < 0.5f) tmp_y = Nfloat + 0.5f;
			j0 = floor(tmp_y);
			j1 = j0 + 1.0f;

			s1 = tmp_x - i0;
			s0 = 1.0f - s1;
			t1 = tmp_y - j0;
			t0 = 1.0f - t1;

			int i0i = (int)i0;
			int i1i = (int)i1;
			int j0j = (int)j0;
			int j1j = (int)j1;

			d[IX(i, j)] =
				s0 * (t0 * d0[IX(i0i, j0j)] + t1 * d0[IX(i0i, j1j)] +
					s1 * (t0 * d0[IX(i1i, j0i)] + t1 * d0[IX(i1i, j1j)]);
		}
	}
	set_bnd(b, d);
}