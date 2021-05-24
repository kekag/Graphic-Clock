// Written by Bart Stander
// Taken from SIGGRAPH course notes on physically based modeling

#ifndef _PARTICLE_HPP_
#define _PARTICLE_HPP_

#include <vector>

using namespace std;

class Force;

const int DIM = 2;
const int MAX_PARTICLES = 1000;

class Particle {
public:
	Particle();
	Particle(double pos_temp[DIM], double dir_temp[DIM], double r_temp, bool anchored_temp);
	Particle(double x, double y, double xDir, double yDir, double r_temp, bool anchored_temp);

	// accessor methods:
	double GetRadius() { return r; }
	double GetMass() { return m; }
	bool GetAnchored() { return anchored; }

	void GetPosition(double pos_temp[DIM]);
	void GetDirection(double dir_temp[DIM]);
	void GetForce(double f_temp[DIM]);
	void SetPosition(double pos_temp[DIM]);
	void SetDirection(double dir_temp[DIM]);
	void SetForce(double f_temp[DIM]);

	double GetPositionX() { return pos[0]; }
	double GetDirectionX() { return dir[0]; }
	double GetForceX() { return f[0]; }
	void SetPositionX(double x) { pos[0] = x; }
	void SetDirectionX(double x) { dir[0] = x; }
	void SetForceX(double x) { f[0] = x; }

	double GetPositionY() { return pos[1]; }
	double GetDirectionY() { return dir[1]; }
	double GetForceY() { return f[1]; }
	void SetPositionY(double y) { pos[1] = y; }
	void SetDirectionY(double y) { dir[1] = y; }
	void SetForceY(double y) { f[1] = y; }

private:
	double pos[DIM]; // position
	double dir[DIM]; // velocity
	double f[DIM];	 // force
	double r;		 // radius
	double m;		 // mass
	bool anchored;   // fixed or moving?
};

class ParticleSystem {
public:
	ParticleSystem();
	~ParticleSystem();

	void AddParticle(Particle* p);
	int GetNumParticles();
	Particle* GetParticle(int i);

	void AddForce(Force* f);
	int GetNumForces();
	Force* GetForce(int i);
	double GetDeltaT();
	void SetDeltaT(double DT);

	// Methods for ODE solvers:
	int ParticleDims();
	void ParticleGetState(double* dst);
	void ParticleSetState(double* src);
	void ParticleGetDerivative(double* dst);
	void IncrementTime(double DeltaT);

private:
	void ClearForces();
	void ComputeForces();

	vector<Particle*> pArray;
	vector<Force*> fArray;
	double time;
	double DeltaT;
};

void RungeKuttaStep(ParticleSystem& ps, double DeltaT);

#endif // _PARTICLE_HPP_