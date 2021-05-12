// Written by Bart Stander

#ifndef _FORCE_HPP_
#define _FORCE_HPP_

#include "particle.hpp"

enum FORCE_TYPE { SPRING_FORCE, GRAVITY_FORCE, DRAG_FORCE };

class Particle;

// Force class, the base class
class Force {
public:
	virtual void Apply()=0;
	virtual FORCE_TYPE Type()=0;
};

// SpringForce class
class SpringForce : public Force {
public:
	SpringForce(Particle* p1, Particle *p2, double spring_constant, double size, double damping_constant, double rest_length=0);

	virtual void Apply();
	virtual FORCE_TYPE Type();

	Particle* GetParticle1();
	Particle* GetParticle2();
	
	double* getColor();
	void setColor(double spring_color[]);
	double getSize();
	void setSize(double size);

private:
	Particle* p1;
	Particle* p2;
	double rest_length;
	double spring_constant;
	double damping_constant;
	double spring_color[4];
	double spring_size;
};

#endif // _FORCE_HPP_