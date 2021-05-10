// Kaden Empey

#define _USE_MATH_DEFINES

#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include "forces.hpp"
#include "particle.hpp"
#include "vector.hpp"

#include <GL/freeglut.h>

using namespace std;

GLdouble red[] = { 1.0, 0.0, 0.0, 1.0 };
GLdouble white[] = { 1.0, 1.0, 1.0, 1.0 };
GLdouble black[] = { 0.0, 0.0, 0.0, 1.0 };

double screen_x = 500;
double screen_y = 500;
// Center coordinates
double cx = screen_x / 2;
double cy = screen_y / 2;

// clock hand radii
double secRadius = 200;
double minRadius = 185;
double hourRadius = 140;
double dash = 205;

int calibrationSamples = 5;
bool calibrated = false;

ParticleSystem PS;

void DrawCircle(double x1, double y1, double radius) {
	glBegin(GL_POLYGON);
	for(int i = 0; i < 32; i++) {
		double theta = (double)i/32.0 * 2.0 * M_PI;
		double x = x1 + radius * cos(theta);
		double y = y1 + radius * sin(theta);
		glVertex2d(x, y);
	}
	glEnd();
}

void DrawLine(double x1, double y1, double x2, double y2, GLfloat lineWidth) {
	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	glVertex2d(x1,y1);
	glVertex2d(x2,y2);
	glEnd();
	glLineWidth(1.0f);
}

void DrawText(double x, double y, const char *string) {
	void *font = GLUT_BITMAP_9_BY_15;

	glRasterPos2d(x, y);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
	for (int i = 0; i < (int)strlen(string); i++) {
		glutBitmapCharacter(font, string[i]);
	}
    glDisable(GL_BLEND);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3dv(white);

	RungeKuttaStep(PS, PS.GetDeltaT()*2);
	
	int N = PS.GetNumParticles();
	int NF = PS.GetNumForces();

	// degrees between expected and actual time
	// negative -> too slow, positive -> too fast
	double disparaity = 0.0;

	// Draw Spring Forces as edges
	for(int i = 0; i < NF; i++) {
		Force *f = PS.GetForce(i);
		if(f->Type() == SPRING_FORCE) {
			SpringForce *sf = (SpringForce*)f;
			Particle *p1 = sf->GetParticle1();
			Particle *p2 = sf->GetParticle2();
			glColor3dv(sf->getColor());
			DrawLine(p1->GetPositionx(), p1->GetPositiony(),  p2->GetPositionx(), p2->GetPositiony(), 4.0f);
		}
	}

	// Draw Particles
	for(int i = 0; i < N; i++) {
		Particle *p = PS.GetParticle(i);
		double radius = p->GetRadius();

		double thePos[DIM];
		p->GetPosition(thePos);
		if(p->GetAnchored())
			glColor3dv(red);
		else
			glColor3dv(white);
		DrawCircle(thePos[0], thePos[1], radius);
	}

	glColor3dv(black);
	
	// Draw reference dashes
	int i = 1;
	char str[3];
	for(int deg = -84; deg < 276; deg += 6) {
		double outX = cos(deg * M_PI / 180) * (dash+5) + cx;
		double outY = -sin(deg * M_PI / 180) * (dash+5) + cy;
		// Draw thick dashes each 5 minutes (and hour)
		if(deg % 30 == 0) {
			double inX = cos(deg * M_PI / 180) * (dash-3) + cx;
			double inY = -sin(deg * M_PI / 180) * (dash-3) + cy;
			DrawLine(inX, inY, outX, outY, 2.25f);
			// Draw clock numbers
			double textX = cos(deg * M_PI / 180) * (dash+22) + cx;
			double textY = -sin(deg * M_PI / 180) * (dash+22) + cy - 5;
			i == 12 ? (textX -= 9) : (textX -= 5);
			snprintf(str, 3, "%d", i);
			DrawText(textX, textY, str);
			i++;
		} else {
			double inX = cos(deg * M_PI / 180) * dash + cx;
			double inY = -sin(deg * M_PI / 180) * dash + cy;
			DrawLine(inX, inY, outX, outY, 1.0f);
		}
	}
	
	glutSwapBuffers();
	glutPostRedisplay();
}

double GetDeltaTime() {
	static clock_t start_time = clock();
	static int current_frame = 0;
	clock_t current_time = clock();
	current_frame += 1;
	double total_time = double(current_time - start_time) / CLOCKS_PER_SEC;
	if (total_time == 0)
		total_time = .00001;
	double frames_per_second = (double)current_frame / total_time;
	double DT = 1.0 / frames_per_second;
	return DT;
}

void Clock() {
	time_t t = time(0);
	tm bt{};
	#if WIN32
	localtime_s(&bt, &t);
	#else 
	localtime_r(&t, &bt);
	#endif
	int second = bt.tm_sec;
	int minute = bt.tm_min;
	int hour = bt.tm_hour % 12;

	char str[9];
	int printHour;
	hour == 0 ? printHour = 12 : printHour = hour;
	snprintf(str, 9, "%d:%02d:%02d", printHour, minute, second);
	cout << "Current time: " << str << "\n";

	// delta factor
	double f = 50;

	// Second degree, no offset needed
	double secdeg = second * 6;
	// Adjust so 0-degree is on 12 instead of 3
	double sd = fmod(secdeg-90, 360);
	double secondX = cos(sd * M_PI / 180) * secRadius + cx;
	double secondY = -sin(sd * M_PI / 180) * secRadius + cy;
	double secondDX = -sin(sd * M_PI / 180) * f;
	double secondDY = -cos(sd * M_PI / 180) * f;
	
	// Minute degree with 0-6 deg. second offset
	double mindeg = minute * 6 + secdeg / 60;
	double md =  fmod(mindeg-90, 360);
	double minuteX = cos(md * M_PI / 180) * minRadius + cx;
	double minuteY = -sin(md * M_PI / 180) * minRadius + cy;
	double minuteDX = -sin(md * M_PI / 180) * f / 60;
	double minuteDY = -cos(md * M_PI / 180) * f / 60;

	// Hour degree with 0-30 deg. minute offset
	double hrdeg = hour * 30 + mindeg / 12;
	double hd = fmod(hrdeg-90, 360);
	double hourX = cos(hd * M_PI / 180) * hourRadius + cx;
	double hourY = -sin(hd * M_PI / 180) * hourRadius + cy;
	double hourDX = -sin(hd * M_PI / 180) * f / 3600;
	double hourDY = -cos(hd * M_PI / 180) * f / 3600;

	// cout << "secX: " << secondX << ", secY: " << secondY << ", secDX: " << secondDX << ", secDY: " << secondDY << endl;
	// cout << "minX: " << minuteX << ", minY: " << minuteY << ", minDX: " << minuteDX << ", minDY: " << minuteDY << endl;
	// cout << "hourX: " << hourX << ", hourY: " << hourY << ", hourDX: " << hourDX << ", hourDY: " << hourDY << endl;
	
	// DeltaT
	PS.SetDeltaT(1000*GetDeltaTime());

	// center, unfixed
	Particle* p1 = new Particle(cx, cy, 0, 0, 10, 1);
	PS.AddParticle(p1);

	// second, minute, hour
	Particle* p2 = new Particle(secondX, secondY, secondDX, secondDY, 2, 0);
	PS.AddParticle(p2);

	Particle* p3 = new Particle(minuteX, minuteY, minuteDX, minuteDY, 2, 0);
	PS.AddParticle(p3);

	Particle* p4 = new Particle(hourX, hourY, hourDX, hourDY, 2, 0);
	PS.AddParticle(p4);

	// second, minute, hour
	SpringForce* s1 = new SpringForce(p1, p2, 10000, 200, secRadius);
	s1->setColor(red);
	PS.AddForce(s1);

	SpringForce* s2 = new SpringForce(p1, p3, 10000, 200, minRadius);
	s2->setColor(black);
	PS.AddForce(s2);

	SpringForce* s3 = new SpringForce(p1, p4, 10000, 200, hourRadius);
	s3->setColor(black);
	PS.AddForce(s3);
}

void keyboard(unsigned char c, int x, int y) {
	switch(c) {
	case 27: // ESC
		exit(0);
	default:
		return;
	}
	glutPostRedisplay();
}

void reshape(int w, int h) {
	screen_x = w;
	screen_y = h;
	cx = w/2;
	cy = h/2;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(screen_x, screen_y);
	glutInitWindowPosition(200, 200);

	glutCreateWindow("Graphics Clock");

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);

	glClearColor(0.9, 0.9, 0.9, 0);	

	Clock();

	glutMainLoop();
	return 0;
}
