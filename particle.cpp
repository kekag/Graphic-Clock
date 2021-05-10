// particle.cpp
// Bart Stander
// January, 2002
// For CS 3600 projects.
// Taken from SIGGRAPH course notes on physically based modeling

#include "particle.h"
#include "forces.h"

Particle::Particle(double pos_temp[DIM], double dir_temp[DIM], double r_temp, bool anchored_temp) {
	for(int d=0; d<DIM; d++) {
		pos[d] = pos_temp[d];
		dir[d] = dir_temp[d];
	}
	r = r_temp;
	anchored = anchored_temp;
	m = r*r;
}

Particle::Particle() {
}

Particle::Particle(double x, double y, double xDir, double yDir, double r_temp, bool anchored_temp) {
	pos[0] = x;
	pos[1] = y;
	dir[0] = xDir;
	dir[1] = yDir;
	r = r_temp;
	anchored = anchored_temp;
	m = r*r;
}

void Particle::GetPosition(double pos_temp[DIM]) {
	for(int d=0; d<DIM; d++)
		pos_temp[d] = pos[d];
}

void Particle::GetDirection(double dir_temp[DIM]) {
	for(int d=0; d<DIM; d++)
		dir_temp[d] = dir[d];
}

void Particle::GetForce(double f_temp[DIM]) {
	for(int d=0; d<DIM; d++)
		f_temp[d] = f[d];
}

void Particle::SetPosition(double pos_temp[DIM]) {
	for(int d=0; d<DIM; d++)
		pos[d] = pos_temp[d];
}

void Particle::SetDirection(double dir_temp[DIM]) {
	for(int d=0; d<DIM; d++)
		dir[d] = dir_temp[d];
}

void Particle::SetForce(double f_temp[DIM]) {
	for(int d=0; d<DIM; d++)
		f[d] = f_temp[d];
}

// Particle system 
ParticleSystem::ParticleSystem() {
	time = 0.0;
	DeltaT = 0.1;
}

ParticleSystem::~ParticleSystem() {
	unsigned int i;
	for(i=0; i<fArray.size(); i++) {
		delete fArray[i];
	}
	for(i=0; i<pArray.size(); i++) {
		delete pArray[i];
	}
}

int ParticleSystem::ParticleDims() {
	return GetNumParticles()*2*DIM;
}

void ParticleSystem::IncrementTime(double DeltaT) {
	time += DeltaT;
}

void ParticleSystem::ParticleGetState(double * dst) {
	int n = GetNumParticles();
	for(int i=0; i<n; i++) {
		Particle * p = GetParticle(i);
		double pos[DIM];
		double dir[DIM];
		p->GetPosition(pos);
		p->GetDirection(dir);

		int d;
		for(d=0; d<DIM; d++) {
			*(dst++) = pos[d];
		}
		for(d=0; d<DIM; d++) {
			*(dst++) = dir[d];
		}
	}
}

void ParticleSystem::ParticleSetState(double * src) {
	int n = GetNumParticles();
	for(int i=0; i<n; i++) {
		Particle * p = GetParticle(i);
		double pos[DIM];
		double dir[DIM];
		p->GetPosition(pos);
		p->GetDirection(dir);

		int d;
		for(d=0; d<DIM; d++) {
			if(p->GetAnchored()==false)
				pos[d] = *src;
			src++;
		}
		for(d=0; d<DIM; d++) {
			if(p->GetAnchored()==false)
				dir[d] = *src;
			src++;
		}

		p->SetPosition(pos);
		p->SetDirection(dir);
	}
}

void ParticleSystem::ParticleGetDerivative(double * dst) {
	ClearForces();
	ComputeForces();
	int n = GetNumParticles();
	for(int i=0; i<n; i++) {
		Particle * p = GetParticle(i);
		double pos[DIM];
		double dir[DIM];
		double f[DIM];
		p->GetPosition(pos);
		p->GetDirection(dir);
		p->GetForce(f);
		double m = p->GetMass();

		int d;
		for(d=0; d<DIM; d++) {
			*(dst++) = dir[d];
		}
		for(d=0; d<DIM; d++) {
			*(dst++) = f[d]/m;
		}
	}
}

void ParticleSystem::ClearForces() {
	int n = GetNumParticles();
	for(int i=0; i<n; i++) {
		Particle * p = GetParticle(i);
		double f[DIM];
		int d;
		for(d=0; d<DIM; d++) {
			f[d]=0.0;
		}
		p->SetForce(f);
	}
}

void ParticleSystem::ComputeForces() {
	size_t nf = fArray.size();
	for(unsigned int i=0; i<nf; i++) {
		fArray[i]->Apply();
	}
}

int ParticleSystem::GetNumParticles() {
	return (int)pArray.size();
}

Particle * ParticleSystem::GetParticle(int i) {
	return pArray[i];
}

void ParticleSystem::AddParticle(Particle * p) {
	pArray.push_back(p);
}
	
void ParticleSystem::AddForce(Force * f) {
	fArray.push_back(f);
}

int ParticleSystem::GetNumForces() {
	return (int)fArray.size();
}

Force * ParticleSystem::GetForce(int i) {
	return fArray[i];
}

double ParticleSystem::GetDeltaT() {
	return DeltaT;
}

void ParticleSystem::SetDeltaT(double DT) {
	DeltaT = DT;
}

// ODE Solver //

void AddVector(double * s1, double * s2, double * destination, int count) {
	for(int i = 0; i< count; i++) {
		*destination++ = *s1++ + *s2++;
	}
}
void CopyVector(double * destination, double * source, int count) {
	for(int i = 0; i < count; i++) {
		*destination++ = *source++;
	}
}
void ScaleVector(double * v, double scale, int count) {
	for(int i = 0; i < count; i++) {
		v[i] *= scale;
	}
}

void RungeKuttaStep(ParticleSystem &ps, double DeltaT) {
	int size = ps.ParticleDims();

	// Get original prevPos
	double prevPos, newPos;
	double k1, k2, k3, k4, p2, p3, p4;
	ps.ParticleGetState(&prevPos);

	// Calculate the delta of an Euler step
	ps.ParticleGetDerivative(&k1);
	ScaleVector(&k1, DeltaT, size);

	// Update the particle system by moving the k1/2
	CopyVector(&p2, &k1, size); // p2 gets k1
	ScaleVector(&p2, 0.5, size);
	AddVector(&prevPos, &p2, &p2, size);
	ps.ParticleSetState(&p2);

	ps.ParticleGetDerivative(&k2);
	ScaleVector(&k2, DeltaT, size);
	CopyVector(&p3, &k2, size);
	ScaleVector(&p3, 0.5, size);
	AddVector(&prevPos, &p3, &p3, size);
	ps.ParticleSetState(&p3);

	ps.ParticleGetDerivative(&k3);
	ScaleVector(&k3, DeltaT, size);
	CopyVector(&p4, &k3, size);
	AddVector(&prevPos, &p4, &p4, size);
	ps.ParticleSetState(&p4);

	ps.ParticleGetDerivative(&k4);
	ScaleVector(&k4, DeltaT, size);

	ScaleVector(&k1, (1.0/6.0), size);
	ScaleVector(&k2, (1.0/3.0), size);
	ScaleVector(&k3, (1.0/3.0), size);
	ScaleVector(&k4, (1.0/6.0), size);
	AddVector(&prevPos, &k1, &newPos, size);
	AddVector(&newPos, &k2, &newPos, size);
	AddVector(&newPos, &k3, &newPos, size);
	AddVector(&newPos, &k4, &newPos, size);
	ps.ParticleSetState(&newPos);

	ps.IncrementTime(DeltaT);
}
