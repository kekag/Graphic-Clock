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

// Screen size (pixels) 
double screen_x = 500;
double screen_y = 500;
// Center coordinates for shorter definitions
double cx = screen_x / 2;
double cy = screen_y / 2;

// Clock hand radii
double secRadius = min(cx, cy) - (min(cx, cy) / 5);
double minRadius = min(cx, cy) - (min(cx, cy) / 4);
double hourRadius = min(cx, cy) - (min(cx, cy) / 2);
double dash = min(cx, cy) - (min(cx, cy) / 10);

// Display 60 frames per 1000 milliseconds, for timer func
static int redisplayInterval = 1000 / 60;

// Synchronization sample variables 
vector<int> samples{ 64, 128, 256, 512 };
int iter = 0;
int frame = 0;
chrono::milliseconds startMS;
chrono::milliseconds endMS;
bool doneSampling = false;

// Reshaping consistency
double initialDT;

ParticleSystem PS;

struct Time {
	int ms, s, m, h;
};

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
	glVertex2d(x1, y1);
	glVertex2d(x2, y2);
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

void displayDashes() {
	// Draw reference dashes
	glColor3dv(black);
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
		} else {
			double inX = cos(deg * M_PI / 180) * dash + cx;
			double inY = -sin(deg * M_PI / 180) * dash + cy;
			displayLine(inX, inY, outX, outY, 1.0f);
		}
	}
}

Time getTime(bool print) {
	chrono::milliseconds current = chrono::duration_cast<chrono::milliseconds>(
		chrono::system_clock::now().time_since_epoch()
	);
	
	int millisecond = current.count() % 1000;
	int second = (current.count() / 1000) % 60;
	int minute = (current.count() / 60000) % 60;
	// This gets unix hour since epoch, need to use tm struct and localtime function to get local time
	// int hour = (current.count() / 3600000) % 12;

	clock_t start = clock();
	time_t t = time(0);
	tm bt{};
#if WIN32
	localtime_s(&bt, &t);
#else 
	localtime_r(&t, &bt);
#endif
	// Gets hour accurate to system's local timezone
	int hour = bt.tm_hour % 12;

	if (print) {
		char str[16];
		int printHour;
		char meridiem;
		hour == 0 ? printHour = 12 : printHour = hour;
		bt.tm_hour >= 12 ? meridiem = 'P' : meridiem = 'A';
		snprintf(str, 16, "%d:%02d:%02d.%03d %cM", printHour, minute, second, millisecond, meridiem);
		cout << "Current time: " << str << "\n\n";
	}

	return { millisecond, second, minute, hour };
}

void wallClock() {
 	clock_t start = clock();

	Time t = getTime(true);

	// delta factor
	double f = 50;

	// Second degree with 0-6 deg ms offset
	double secDeg = t.s * 6 + t.ms / 166.6667;
	// Adjust so 0-degree is on 12 instead of 3
	double sd = fmod(secDeg + 270, 360);
	double secondX = cos(sd * M_PI / 180) * secRadius + cx;
	double secondY = -sin(sd * M_PI / 180) * secRadius + cy;
	double secondDX = -sin(sd * M_PI / 180) * f;
	double secondDY = -cos(sd * M_PI / 180) * f;

	// Minute degree with 0-6 deg second offset
	double minDeg = t.m * 6 + secDeg / 60;
	double md = fmod(minDeg + 270, 360);
	double minuteX = cos(md * M_PI / 180) * minRadius + cx;
	double minuteY = -sin(md * M_PI / 180) * minRadius + cy;
	double minuteDX = -sin(md * M_PI / 180) * f / 60;
	double minuteDY = -cos(md * M_PI / 180) * f / 60;

	// Hour degree with 0-30 deg minute offset
	double hourDeg = t.h * 30 + minDeg / 12;
	double hd = fmod(hourDeg + 270, 360);
	double hourX = cos(hd * M_PI / 180) * hourRadius + cx;
	double hourY = -sin(hd * M_PI / 180) * hourRadius + cy;
	double hourDX = -sin(hd * M_PI / 180) * f / /*3600*/ 1800;
	double hourDY = -cos(hd * M_PI / 180) * f / /*3600*/ 1800;

	cout << "Inital values" << endl;
	cout << "secX: " << secondX << ", secY: " << secondY << ", secDX: " << secondDX << ", secDY: " << secondDY << endl;
	cout << "minX: " << minuteX << ", minY: " << minuteY << ", minDX: " << minuteDX << ", minDY: " << minuteDY << endl;
	cout << "hourX: " << hourX << ", hourY: " << hourY << ", hourDX: " << hourDX << ", hourDY: " << hourDY << "\n\n";

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
	SpringForce* s1 = new SpringForce(p1, p2, 10000, 2.0, 10000, secRadius);
	s1->setColor(red);
	PS.AddForce(s1);
	SpringForce* s2 = new SpringForce(p1, p3, 10000, 3.5, 1000, minRadius);
	s2->setColor(black);
	PS.AddForce(s2);
	SpringForce* s3 = new SpringForce(p1, p4, 10000, 4.5, 1000, hourRadius);
	s3->setColor(black);
	PS.AddForce(s3);

	clock_t end = clock();
	double approxDT = (double)(end - start) / 5800.0;
	if (approxDT == 0) {
		approxDT = 0.0001;
	}
	cout << "Temp approximate DT: " << approxDT << endl;
	PS.SetDeltaT(approxDT);
}

void NoPSrender(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	Time t = getTime(false);

	// Second degree, no offset needed
	double secDeg = t.s * 6 + t.ms / 166.6667;
	double sd = fmod(secDeg + 270, 360);
	double secondX = cos(sd * M_PI / 180) * secRadius + cx;
	double secondY = -sin(sd * M_PI / 180) * secRadius + cy;

	// Minute degree with 0-6 deg. second offset
	double minDeg = t.m * 6 + secDeg / 60;
	double md = fmod(minDeg + 270, 360);
	double minuteX = cos(md * M_PI / 180) * minRadius + cx;
	double minuteY = -sin(md * M_PI / 180) * minRadius + cy;

	// Hour degree with 0-30 deg. minute offset
	double hourDeg = t.h * 30 + minDeg / 12;
	double hd = fmod(hourDeg + 270, 360);
	double hourX = cos(hd * M_PI / 180) * hourRadius + cx;
	double hourY = -sin(hd * M_PI / 180) * hourRadius + cy;

	// Draw lines
	glColor3dv(red);
	displayLine(cx, cy, secondX, secondY, 2.0);
	glColor3dv(black);
	displayLine(cx, cy, minuteX, minuteY, 3.5);
	displayLine(cx, cy, hourX, hourY, 4.5);

	// Draw center particle
	glColor3dv(red);
	displayCircle(cx, cy, 10);

	// Draw reference dashes
	displayDashes();

	glutSwapBuffers();
	glutPostRedisplay();
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
			displayLine(p1->GetPositionX(), p1->GetPositionY(), p2->GetPositionX(), p2->GetPositionY(), (GLfloat)sf->getSize());
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
	displayDashes();

	glutSwapBuffers();

	if (frame == samples[iter] && !doneSampling) {
		endMS = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now().time_since_epoch()
		);
		if (iter < 2) {
			double DT = (endMS.count() - startMS.count()) / (2500.0 * frame);
			cout << "DT after " << samples[iter] << " frame samples: " << DT << endl;
			PS.SetDeltaT(DT);
			initialDT = DT;
		}
		
		// Get arc length disparity
		double expectedDeg = (endMS.count() % 60000 / 166.6666667);
		cout << "expected: " << expectedDeg << endl;

		// Positions relative to the center particle
		Particle* S = PS.GetParticle(1);
		double Y = S->GetPositionY() - cy;
		double X = S->GetPositionX() - cx;

		// Find theta using arc tangent
		double actualDeg;
		if (X >= 0 && Y >= 0) {        // QUADRANT I
			actualDeg = atan(Y / X) * 180 / M_PI;
		} else if (X <= 0 && Y >= 0) { // QUADRANT II
			actualDeg = abs(atan(Y / -X) * 180 / M_PI - 90) + 90;
		} else if (X <= 0 && Y <= 0) { // QUADRANT III
			actualDeg = abs(atan(-Y / -X) * 180 / M_PI) + 180;
		} else { 					   // QUADRANT IV
			actualDeg = abs(atan(-Y / X) * 180 / M_PI - 90) + 270;
		}
		actualDeg = -(fmod(actualDeg + 270, 360) - 360);
		cout << "actual: " << actualDeg << endl;
		
		double disparity;
		bool flipped = false;
		expectedDeg < actualDeg ? disparity = actualDeg - expectedDeg : disparity = expectedDeg - actualDeg;
		if (disparity > 180) {
			disparity = 360 - disparity;
			flipped = true;
		}
		cout << "disparity: " << disparity << endl;

		// Calculate absolute arc length between expected and actual second degree (2 * pi * radius * (theta / 360))
		double arc = 2 * M_PI * secRadius * (disparity / 360);
		// Negate if DT is behind
		if (expectedDeg < actualDeg && !flipped || expectedDeg >= actualDeg && flipped) {
			arc = -arc;
		} 
		cout << "arc length: " << arc << endl;

		double currentDT = PS.GetDeltaT();
		cout << "current DT: " << currentDT << endl;
		double adjustment = arc * (currentDT / 1000.0);
		cout << "adjustment: " << adjustment << endl;
		double DT = currentDT + adjustment;
		cout << "DT: " << DT << endl;
		
		PS.SetDeltaT(DT);
		initialDT = DT;
		frame = 0;
		if (iter < (samples.size() - 1)) {
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
	secRadius = min(cx, cy) - (min(cx, cy)/5);
	minRadius = min(cx, cy) - (min(cx, cy)/4);
	hourRadius = min(cx, cy) - (min(cx, cy)/2);
	dash = min(cx, cy) - (min(cx, cy)/10); 

	// Center particle
	Particle* p = PS.GetParticle(0);
	double prevX = p->GetPositionX();
	double prevY = p->GetPositionY();
	p->SetPositionX(cx);
	p->SetPositionY(cy);
	double offsetX = cx - prevX;
	double offsetY = cy - prevY;

	// Adjust hand particle positions based on offset 
	int N = PS.GetNumParticles();
	for (int i = 1; i < N; i++) {
		Particle* p = PS.GetParticle(i);
		p->SetPositionX(p->GetPositionX() + offsetX);
		p->SetPositionY(p->GetPositionY() + offsetY);
	}

	// Adjust hand sizes after repositioning
	SpringForce* second = (SpringForce*)PS.GetForce(0);
	SpringForce* minute = (SpringForce*)PS.GetForce(1);
	SpringForce* hour = (SpringForce*)PS.GetForce(2);
	second->setRestLength(secRadius);
	minute->setRestLength(minRadius);
	hour->setRestLength(hourRadius);

	// Adjust DT based on relative hand sizes
	double ratio = min(cx, cy) / 250;
	double scale = initialDT*ratio;
	double disparity = scale - PS.GetDeltaT();
	double reshape = PS.GetDeltaT() + disparity;
	if (reshape != 0) {
		PS.SetDeltaT(reshape);
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
	
	// Uses particle system with delta X and Y initializers and Runge Kutta step method to approximate time
	glutDisplayFunc(render);
	// Gets actual system time each frame and calculates X and Y positions based on that
	// glutDisplayFunc(NoPSrender);

	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);

	glutTimerFunc(redisplayInterval, timer, 0);

	glClearColor(0.9f, 0.9f, 0.9f, 0.0f);	
	wallClock();

	glutMainLoop();
	return 0;
}
