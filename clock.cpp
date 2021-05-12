// Kaden Empey

#define _USE_MATH_DEFINES

#include <cstdio>
#include <cmath>
#include <ctime>
#include <chrono>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include "forces.hpp"
#include "particle.hpp"
#include "vector.hpp"

#include <GL/freeglut.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

GLdouble red[] = { 1.0, 0.0, 0.0, 1.0 };
GLdouble white[] = { 1.0, 1.0, 1.0, 1.0 };
GLdouble black[] = { 0.0, 0.0, 0.0, 1.0 };
GLdouble clear[] = { 0.9, 0.9, 0.9, 0.0 };

double screen_x = 500;
double screen_y = 500;
// Center coordinates
double cx = screen_x / 2;
double cy = screen_y / 2;

// clock hand radii
double secRadius = min(cx, cy) - 50;
double minRadius = min(cx, cy) - 65;
double hourRadius = min(cx, cy) - 125;
double dash = min(cx, cy) - 25;

// display 60 frames per 1000 milliseconds
static int redisplayInterval = 1000 / 60;
vector<int> samples{ 64, 128, 256 };
int iter = 0;
int frame = 0;
chrono::milliseconds startMS;
chrono::milliseconds endMS;
bool doneSampling = false;

ParticleSystem PS;

void displayCircle(double x1, double y1, double radius) {
	glBegin(GL_POLYGON);
	for(int i = 0; i < 50; i++) {
		double theta = (double)i/50.0 * 2.0 * M_PI;
		double x = x1 + radius * cos(theta);
		double y = y1 + radius * sin(theta);
		glVertex2d(x, y);
	}
	glEnd();
}

void displayLine(double x1, double y1, double x2, double y2, GLfloat lineWidth) {
	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	glVertex2d(x1,y1);
	glVertex2d(x2,y2);
	glEnd();
	glLineWidth(1.0f);
}

void displayText(double x, double y, const char *string) {
	void *font = GLUT_BITMAP_9_BY_15;

	glRasterPos2d(x, y);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
	for (int i = 0; i < (int)strlen(string); i++) {
		glutBitmapCharacter(font, string[i]);
	}
    glDisable(GL_BLEND);
}

void wallClock() {
	clock_t start = clock();

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
	double secDeg = second * 6;
	// Adjust so 0-degree is on 12 instead of 3
	double sd = fmod(secDeg - 90, 360);
	double secondX = cos(sd * M_PI / 180) * secRadius + cx;
	double secondY = -sin(sd * M_PI / 180) * secRadius + cy;
	double secondDX = -sin(sd * M_PI / 180) * f;
	double secondDY = -cos(sd * M_PI / 180) * f;

	// Minute degree with 0-6 deg. second offset
	double minDeg = minute * 6 + secDeg / 60;
	double md = fmod(minDeg - 90, 360);
	double minuteX = cos(md * M_PI / 180) * minRadius + cx;
	double minuteY = -sin(md * M_PI / 180) * minRadius + cy;
	double minuteDX = -sin(md * M_PI / 180) * f / /*60*/ 55;
	double minuteDY = -cos(md * M_PI / 180) * f / /*60*/ 55;

	// Hour degree with 0-30 deg. minute offset
	double hourDeg = hour * 30 + minDeg / 12;
	double hd = fmod(hourDeg - 90, 360);
	double hourX = cos(hd * M_PI / 180) * hourRadius + cx;
	double hourY = -sin(hd * M_PI / 180) * hourRadius + cy;
	double hourDX = -sin(hd * M_PI / 180) * f / /*3600*/ 1000;
	double hourDY = -cos(hd * M_PI / 180) * f / /*3600*/ 1000;

	// cout << "secX: " << secondX << ", secY: " << secondY << ", secDX: " << secondDX << ", secDY: " << secondDY << endl;
	// cout << "minX: " << minuteX << ", minY: " << minuteY << ", minDX: " << minuteDX << ", minDY: " << minuteDY << endl;
	// cout << "hourX: " << hourX << ", hourY: " << hourY << ", hourDX: " << hourDX << ", hourDY: " << hourDY << endl;

	// center, unfixed
	Particle* p1 = new Particle(cx, cy, 0, 0, 10, 1);
	PS.AddParticle(p1);

	// second, minute, hour
	Particle* p2 = new Particle(secondX, secondY, secondDX, secondDY, 10, 0);
	PS.AddParticle(p2);

	Particle* p3 = new Particle(minuteX, minuteY, minuteDX, minuteDY, 10, 0);
	PS.AddParticle(p3);

	Particle* p4 = new Particle(hourX, hourY, hourDX, hourDY, 10, 0);
	PS.AddParticle(p4);

	// second, minute, hour
	SpringForce* s1 = new SpringForce(p1, p2, 10000, 2.0, 200, secRadius);
	s1->setColor(red);
	PS.AddForce(s1);

	SpringForce* s2 = new SpringForce(p1, p3, 10000, 3.5, 200, minRadius);
	s2->setColor(black);
	PS.AddForce(s2);

	SpringForce* s3 = new SpringForce(p1, p4, 10000, 4.5, 200, hourRadius);
	s3->setColor(black);
	PS.AddForce(s3);

	clock_t end = clock();
	double approxDT = (double)(end-start) / 5800.0;
	if (approxDT == 0) {
		approxDT = 0.0001;
	}
	cout << "Temp approximate DT: " << approxDT << endl;
	PS.SetDeltaT(approxDT);
}

void render(void) {
	if (frame == 0 && !doneSampling) {
		startMS = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now().time_since_epoch()
		);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3dv(white);

	RungeKuttaStep(PS, PS.GetDeltaT());

	int N = PS.GetNumParticles();
	int NF = PS.GetNumForces();

	// Draw Spring Forces as edges
	for (int i = 0; i < NF; i++) {
		Force* f = PS.GetForce(i);
		if (f->Type() == SPRING_FORCE) {
			SpringForce* sf = (SpringForce*)f;
			Particle* p1 = sf->GetParticle1();
			Particle* p2 = sf->GetParticle2();
			glColor3dv(sf->getColor());
			displayLine(p1->GetPositionX(), p1->GetPositionY(), p2->GetPositionX(), p2->GetPositionY(), sf->getSize());
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Draw Particles
	for (int i = 0; i < N; i++) {
		Particle* p = PS.GetParticle(i);
		double radius = p->GetRadius();

		double thePos[DIM];
		p->GetPosition(thePos);
		if (p->GetAnchored())
			glColor4dv(red);
		else
			glColor4dv(clear);
		displayCircle(thePos[0], thePos[1], radius);
	}
	glDisable(GL_BLEND);

	glColor3dv(black);

	// Draw reference dashes
	int i = 1;
	char str[3];
	for (int deg = -84; deg < 276; deg += 6) {
		double outX = cos(deg * M_PI / 180) * (dash + 4) + cx;
		double outY = -sin(deg * M_PI / 180) * (dash + 4) + cy;
		// Draw thick dashes each 5 minutes (and hour)
		if (deg % 30 == 0) {
			double inX = cos(deg * M_PI / 180) * (dash - 4) + cx;
			double inY = -sin(deg * M_PI / 180) * (dash - 4) + cy;
			displayLine(inX, inY, outX, outY, 2.25f);
			// Draw clock numbers
			double textX = cos(deg * M_PI / 180) * (dash - 20) + cx;
			double textY = -sin(deg * M_PI / 180) * (dash - 20) + cy - 5;
			i == 12 ? (textX -= 9) : (textX -= 5);
			snprintf(str, 3, "%d", i);
			displayText(textX, textY, str);
			i++;
		}
		else {
			double inX = cos(deg * M_PI / 180) * dash + cx;
			double inY = -sin(deg * M_PI / 180) * dash + cy;
			displayLine(inX, inY, outX, outY, 1.0f);
		}
	}

	glutSwapBuffers();

	// Get DT arc length disparity
	// double expectedDeg = (endMS.count() % 60000 / 166.6666667);
	// Particle* S = PS.GetParticle(1);
	// double actualDeg = atan((S->GetPositiony() + cy) / (S->GetPositionx() + cx));
	// Get DT from arc length disparities (2 * pi * radius * (theta / 360))
	// double smallestTheta;
	// if (expectedDeg < actualDeg) {
	// 	smallestTheta = actualDeg - expectedDeg;
	// }
	// else {
	// 	smallestTheta = expectedDeg - actualDeg;
	// }
	// if (smallestTheta > 180) {
	// 	smallestTheta = 360 - smallestTheta;
	// }
	// double arcLen = 2 * M_PI * secRadius * (smallestTheta / 360);
	// negative -> DT too slow, positive -> DT too fast
	// if (expectedDeg >= actualDeg) {
	// 	arcLen = -arcLen;
	// }

	if (frame == samples[iter] && !doneSampling) {
		endMS = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now().time_since_epoch()
		);
		double DT = (endMS.count() - startMS.count()) / (2500.0 * frame);
		cout << "DT after " << samples[iter] << " frame samples: " << DT << endl;
		PS.SetDeltaT(DT);
		frame = 0;
		if (iter < samples.size() - 1) {
			iter++;
		} else {
			doneSampling = true;
		}
	} else if (!doneSampling){
		frame++;
	}

	glutPostRedisplay();
}

void keyboard(unsigned char c, int x, int y) {
	switch (c) {
	case 27: // ESC
		exit(0);
	default:
		return;
	}
	glutPostRedisplay();
}

void reshape(int width, int height) {
	double prev_x = screen_x / 2;
	double prev_y = screen_y / 2;
	screen_x = width;
	screen_y = height;
	cx = width / 2;
	cy = height / 2;

	// Change hand and dash radii
	secRadius = min(cx, cy) - 50;
	minRadius = min(cx, cy) - 65;
	hourRadius = min(cx, cy) - 125;
	dash = min(cx, cy) - 25; 

	// Adjust hand sizes
	SpringForce* second = (SpringForce*)PS.GetForce(0);
	SpringForce* minute = (SpringForce*)PS.GetForce(1);
	SpringForce* hour = (SpringForce*)PS.GetForce(2);
	second->setRestLength(secRadius);
	minute->setRestLength(minRadius);
	hour->setRestLength(hourRadius);

	// Adjust particle positions
	int N = PS.GetNumParticles();
	double offset = min(cx, cy) - min(prev_x, prev_y);
	for (int i = 0; i < N; i++) {
		Particle* p = PS.GetParticle(i);
		p->SetPositionX(p->GetPositionX() + offset);
		p->SetPositionY(p->GetPositionY() + offset);
	}

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0, width, 0, height);
	glMatrixMode(GL_MODELVIEW);
}

void timer(int) {
	glutPostRedisplay();
	glutTimerFunc(redisplayInterval, timer, 0);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize((int)screen_x, (int)screen_y);
	glutInitWindowPosition(300, 300);
	glutCreateWindow("Clock");
	
	glutDisplayFunc(render);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);

	glutTimerFunc(redisplayInterval, timer, 0);

	glClearColor(0.9f, 0.9f, 0.9f, 0.0f);	
	wallClock();

	glutMainLoop();
	return 0;
}
