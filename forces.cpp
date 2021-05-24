#include <cmath>
#include <iostream>
#include <iomanip>
#include "forces.hpp"
#include "particle.hpp"

using namespace std;

// Spring Force
SpringForce::SpringForce(Particle* p1_temp, Particle* p2_temp, double spring_constant_temp, double size_temp, double damping_constant_temp, double rest_length_temp)
		: p1(p1_temp), p2(p2_temp), spring_constant(spring_constant_temp), spring_size(size_temp),  damping_constant(damping_constant_temp), rest_length(rest_length_temp) {

	if (rest_length_temp == 0) {
		double pos1[DIM];
		double pos2[DIM];
		p1->GetPosition(pos1);
		p2->GetPosition(pos2);
		int d;
		for (d = 0; d < DIM; d++) {
			double dif = pos1[d] - pos2[d];
			rest_length += dif * dif;
		}
		rest_length = sqrt(rest_length);
		// rest_length *= rest_length_temp;
	}
}

void SpringForce::Apply() {
	double pos1[DIM];
	double pos2[DIM];
	double dir1[DIM];
	double dir2[DIM];
	double f1[DIM];
	double f2[DIM];
	p1->GetPosition(pos1);
	p2->GetPosition(pos2);
	p1->GetDirection(dir1);
	p2->GetDirection(dir2);
	p1->GetForce(f1);
	p2->GetForce(f2);

	double pDif[DIM];
	double vDif[DIM];
	double dotProduct = 0.0;
	double pDifSize = 0.0;

	int d;
	for(d = 0; d < DIM; d++) {
		pDif[d] = pos1[d] - pos2[d];
		vDif[d] = dir1[d] - dir2[d];
		pDifSize += pDif[d]*pDif[d];
		dotProduct += pDif[d] * vDif[d];
	}
	pDifSize = sqrt(pDifSize);

	double multiplier = -(spring_constant*(pDifSize-rest_length) + damping_constant*dotProduct/pDifSize)/pDifSize;
	for(d = 0; d < DIM; d++) {
		// f1[d] += multiplier * pDif[d];
		// f2[d] += -(f1[d]);
		double newForce = multiplier * pDif[d];
		f1[d] += newForce;
		f2[d] -= newForce;
	}
	p1->SetForce(f1);
	p2->SetForce(f2);
}

Particle* SpringForce::GetParticle1() {
	return p1;
}

Particle* SpringForce::GetParticle2() {
	return p2;
}

double* SpringForce::getColor() {
	return spring_color;
}

void SpringForce::setColor(double spring_color_temp[]) {
	spring_color[0] = spring_color_temp[0];
	spring_color[1] = spring_color_temp[1];
	spring_color[2] = spring_color_temp[2];
	spring_color[3] = spring_color_temp[3];
}

double SpringForce::getSize() {
	return spring_size;
}

void SpringForce::setSize(double size) {
	spring_size = size;
}

void SpringForce::setRestLength(double length) {
	rest_length = length;
	Apply();
}

FORCE_TYPE SpringForce::Type() {
	return SPRING_FORCE;
}
