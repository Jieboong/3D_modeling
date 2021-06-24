#include "Renderer.h"

void draw_center(void)
{
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f); /* R */
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.2f, 0.0f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'x');

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f); /* G */
	glVertex3f(0.0f, 0.2f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.2f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'y');

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f); /* B */
	glVertex3f(0.0f, 0.0f, -0.2f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.0f, -0.2f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'z');
}

void idle() {
	static GLuint previousClock = glutGet(GLUT_ELAPSED_TIME);
	static GLuint currentClock = glutGet(GLUT_ELAPSED_TIME);
	static GLfloat deltaT;

	currentClock = glutGet(GLUT_ELAPSED_TIME);
	deltaT = currentClock - previousClock;
	if (deltaT < 1000.0 / 20.0) { return; }
	else { previousClock = currentClock; }

	//char buff[256];
	//sprintf_s(buff, "Frame Rate = %f", 1000.0 / deltaT);
	//frameRate = buff;

	glutPostRedisplay();
}

void close()
{
	glDeleteTextures(1, &dispBindIndex);
	glutLeaveMainLoop();
	CloseHandle(hMutex);
}

void add_quats(float q1[4], float q2[4], float dest[4])
{
	static int count = 0;
	float t1[4], t2[4], t3[4];
	float tf[4];

	vcopy(q1, t1);
	vscale(t1, q2[3]);

	vcopy(q2, t2);
	vscale(t2, q1[3]);

	vcross(q2, q1, t3);
	vadd(t1, t2, tf);
	vadd(t3, tf, tf);
	tf[3] = q1[3] * q2[3] - vdot(q1, q2);

	dest[0] = tf[0];
	dest[1] = tf[1];
	dest[2] = tf[2];
	dest[3] = tf[3];

	if (++count > RENORMCOUNT) {
		count = 0;
		normalize_quat(dest);
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(58, (double)width / height, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	GLfloat spin_quat[4];
	float gain;
	gain = 2.0; /* trackball gain */

	if (drag_state == GLUT_DOWN)
	{
		if (button_state == GLUT_LEFT_BUTTON)
		{
			trackball(spin_quat,
				(gain * rot_x - 500) / 500,
				(500 - gain * rot_y) / 500,
				(gain * x - 500) / 500,
				(500 - gain * y) / 500);
			add_quats(spin_quat, quat, quat);
		}
		else if (button_state == GLUT_RIGHT_BUTTON)
		{
			t[0] -= (((float)trans_x - x) / 500);
			t[1] += (((float)trans_y - y) / 500);
		}
		else if (button_state == GLUT_MIDDLE_BUTTON)
			t[2] -= (((float)trans_z - y) / 500 * 4);
		else if (button_state == 3 || button_state == 4) // scroll
		{

		}
		//glutPostRedisplay();
	}

	rot_x = x;
	rot_y = y;

	trans_x = x;
	trans_y = y;
	trans_z = y;
}

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			rot_x = x;
			rot_y = y;

			//t[0] = t[0] + 1;


		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			trans_x = x;
			trans_y = y;
		}
		else if (button == GLUT_MIDDLE_BUTTON)
		{
			//trcon = trcon + 1;
			trans_z = y;
		}
		else if (button == 3 || button == 4)
		{
			const float sign = (static_cast<float>(button)-3.5f) * 2.0f;
			t[2] -= sign * 500 * 0.00015f;
		}
	}

	drag_state = state;
	button_state = button;
}

void vzero(float* v)
{
	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

void vset(float* v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void vsub(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] - src2[0];
	dst[1] = src1[1] - src2[1];
	dst[2] = src1[2] - src2[2];
}

void vcopy(const float *v1, float *v2)
{
	register int i;
	for (i = 0; i < 3; i++)
		v2[i] = v1[i];
}

void vcross(const float *v1, const float *v2, float *cross)
{
	float temp[3];

	temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	vcopy(temp, cross);
}

float vlength(const float *v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float *v, float div)
{
	v[0] *= div;
	v[1] *= div;
	v[2] *= div;
}

void vnormal(float *v)
{
	vscale(v, 1.0f / vlength(v));
}

float vdot(const float *v1, const float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void vadd(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] + src2[0];
	dst[1] = src1[1] + src2[1];
	dst[2] = src1[2] + src2[2];
}

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{
	float a[3]; /* Axis of rotation */
	float phi;  /* how much to rotate about axis */
	float p1[3], p2[3], d[3];
	float t;

	if (p1x == p2x && p1y == p2y) {
		/* Zero rotation */
		vzero(q);
		q[3] = 1.0;
		return;
	}

	/*
	 * First, figure out z-coordinates for projection of P1 and P2 to
	 * deformed sphere
	 */
	vset(p1, p1x, p1y, tb_project_to_sphere(TRACKBALLSIZE, p1x, p1y));
	vset(p2, p2x, p2y, tb_project_to_sphere(TRACKBALLSIZE, p2x, p2y));

	/*
	 *  Now, we want the cross product of P1 and P2
	 */
	vcross(p2, p1, a);

	/*
	 *  Figure out how much to rotate around that axis.
	 */
	vsub(p1, p2, d);
	t = vlength(d) / (2.0f*TRACKBALLSIZE);

	/*
	 * Avoid problems with out-of-control values...
	 */
	if (t > 1.0) t = 1.0;
	if (t < -1.0) t = -1.0;
	phi = 2.0f * asin(t);

	axis_to_quat(a, phi, q);
}

void axis_to_quat(float a[3], float phi, float q[4])
{
	vnormal(a);
	vcopy(a, q);
	vscale(q, sin(phi / 2.0f));
	q[3] = cos(phi / 2.0f);
}

float tb_project_to_sphere(float r, float x, float y)
{
	float d, t, z;

	d = sqrt(x*x + y*y);
	if (d < r * 0.70710678118654752440f) {    /* Inside sphere */
		z = sqrt(r*r - d*d);
	}
	else {           /* On hyperbola */
		t = r / 1.41421356237309504880f;
		z = t*t / d;
	}
	return z;
}

void normalize_quat(float q[4])
{
	int i;
	float mag;

	mag = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	for (i = 0; i < 4; i++) q[i] /= mag;
}

void build_rotmatrix(float m[4][4], float q[4])
{
	m[0][0] = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
	m[0][1] = 2.0f * (q[0] * q[1] - q[2] * q[3]);
	m[0][2] = 2.0f * (q[2] * q[0] + q[1] * q[3]);
	m[0][3] = 0.0f;

	m[1][0] = 2.0f * (q[0] * q[1] + q[2] * q[3]);
	m[1][1] = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
	m[1][2] = 2.0f * (q[1] * q[2] - q[0] * q[3]);
	m[1][3] = 0.0f;

	m[2][0] = 2.0f * (q[2] * q[0] - q[1] * q[3]);
	m[2][1] = 2.0f * (q[1] * q[2] + q[0] * q[3]);
	m[2][2] = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void InitializeWindow(int argc, char* argv[])
{
	// initialize glut settings
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1000 / 2, 1000 / 2);

	glutInitWindowPosition(0, 0);

	dispWindowIndex = glutCreateWindow("3D Model");

	trackball(quat, 90.0, 0.0, 0.0, 0.0);

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutCloseFunc(close);
	//GLuint image = load   ("./my_texture.bmp");
	
	//glBindTexture(1,)

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// bind textures
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	reshape(1000, 1000);

	/*glGenTextures(1, &dispBindIndex);
	glBindTexture(GL_TEXTURE_2D, dispBindIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 0.1, 200);
	glTranslatef(t[0], t[1], t[2] - 1.0f);
	glScalef(1, 1, 1);
	GLfloat m[4][4], m1[4][4];
	build_rotmatrix(m, quat);
	gluLookAt(0, 0.2, 2.0, 0, 0, 0, 0, 1.0, 0);

	GLfloat r, g, b;
	glMultMatrixf(&m[0][0]);



	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat diffuse0[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambient0[4] = { 0.1, 0.1, 0.1, 1.0 };
	//GLfloat specular0[4] = { 1.0, 1.0, 1.0, 1.0 };
	//GLfloat light0_pos[4] = { 2.0, 2.0, 2.0, 1.0 };
	//GLfloat light0_pos[4] = { 0.3, 0.3, 0.5, 1.0 };
	//GLfloat spot_dir[3] = { -2.0f, 0.0f, -1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse0);

	//floor
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 276, 276, 0, GL_RGB, GL_UNSIGNED_BYTE, mytexels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	for (float fl = -1.0; fl < 1.0; fl = fl + 0.2)
	{
		for (float fr = -1.0; fr < 1.0; fr = fr + 0.2)
		{
			glTexCoord2d(0.0, 0.0);
			glVertex3f(-0.2 + fl, 0.0, -0.2 + fr);
			glTexCoord2d(1.0, 0.0);
			glVertex3f(-0.2 + fl, 0.0, 0.2 + fr);
			glTexCoord2d(1.0, 1.0);
			glVertex3f(0.2 + fl, 0.0, 0.2 + fr);
			glTexCoord2d(0.0, 1.0);
			glVertex3f(0.2 + fl, 0.0, -0.2 + fr);
		}

	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//wall
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, wall);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	for (float fl = -1.0; fl < 1.0; fl = fl + 0.2)
	{
		for (float fr = -1.0; fr < 1.0; fr = fr + 0.2)
		{
			glTexCoord2d(0.0, 0.0);
			glVertex3f(-0.2, 0.0, 0.2);
			glTexCoord2d(1.0, 0.0);
			glVertex3f(-0.2, 0.4, 0.2);
			glTexCoord2d(1.0, 1.0);
			glVertex3f(0.2, 0.4, 0.2);
			glTexCoord2d(0.0, 1.0);
			glVertex3f(0.2, 0.0, 0.2);

			glTexCoord2d(1.0, 1.0);
			glVertex3f(-0.2, 0.4, -0.2);
			glTexCoord2d(1.0, 0.0);
			glVertex3f(-0.2, 0.4, 0.2);
			glTexCoord2d(0.0, 0.0);
			glVertex3f(-0.2, 0.0, 0.2);
			glTexCoord2d(0.0, 1.0);
			glVertex3f(-0.2, 0.0, -0.2);

			glTexCoord2d(0.0, 1.0);
			glVertex3f(0.2, 0.0, -0.2);
			glTexCoord2d(0.0, 0.0);
			glVertex3f(0.2, 0.0, 0.2);
			glTexCoord2d(1.0, 0.0);
			glVertex3f(0.2, 0.4, 0.2);
			glTexCoord2d(1.0, 1.0);
			glVertex3f(0.2, 0.4, -0.2);

		}

	}
	glEnd();
	//milk

	glScalef(0.8, 0.8, 0.8);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 4096, 4096, 0, GL_RGB, GL_UNSIGNED_BYTE, milk_texel);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	for (int jj = 0; jj < 49; jj++) {
		glTexCoord2d(milk_c[mmilk[jj].T1 - 1].X, milk_c[mmilk[jj].T1 - 1].Y);
		glVertex3f(milk_v[mmilk[jj].V1 - 1].X, milk_v[mmilk[jj].V1 - 1].Y, milk_v[mmilk[jj].V1 - 1].Z);
		glTexCoord2d(milk_c[mmilk[jj].T2 - 1].X, milk_c[mmilk[jj].T2 - 1].Y);
		glVertex3f(milk_v[mmilk[jj].V2 - 1].X, milk_v[mmilk[jj].V2 - 1].Y, milk_v[mmilk[jj].V2 - 1].Z);
		glTexCoord2d(milk_c[mmilk[jj].T3 - 1].X, milk_c[mmilk[jj].T3 - 1].Y);
		glVertex3f(milk_v[mmilk[jj].V3 - 1].X, milk_v[mmilk[jj].V3 - 1].Y, milk_v[mmilk[jj].V3 - 1].Z);
		glTexCoord2d(milk_c[mmilk[jj].T4 - 1].X, milk_c[mmilk[jj].T4 - 1].Y);
		glVertex3f(milk_v[mmilk[jj].V4 - 1].X, milk_v[mmilk[jj].V4 - 1].Y, milk_v[mmilk[jj].V4 - 1].Z);
	}
	glEnd();

	glBegin(GL_QUADS);
	for (int jj = 0; jj < 15200; jj++) {
		glTexCoord2d(cup_c[mcup[jj].T1 - 1].X, cup_c[mcup[jj].T1 - 1].Y);
		glVertex3f(cup_v[mcup[jj].V1 - 1].X, cup_v[mcup[jj].V1 - 1].Y, cup_v[mcup[jj].V1 - 1].Z);
		glTexCoord2d(cup_c[mcup[jj].T2 - 1].X, cup_c[mcup[jj].T2 - 1].Y);
		glVertex3f(cup_v[mcup[jj].V2 - 1].X, cup_v[mcup[jj].V2 - 1].Y, cup_v[mcup[jj].V2 - 1].Z);
		glTexCoord2d(cup_c[mcup[jj].T3 - 1].X, cup_c[mcup[jj].T3 - 1].Y);
		glVertex3f(cup_v[mcup[jj].V3 - 1].X, cup_v[mcup[jj].V3 - 1].Y, cup_v[mcup[jj].V3 - 1].Z);
		glTexCoord2d(cup_c[mcup[jj].T4 - 1].X, cup_c[mcup[jj].T4 - 1].Y);
		glVertex3f(cup_v[mcup[jj].V4 - 1].X, cup_v[mcup[jj].V4 - 1].Y, cup_v[mcup[jj].V4 - 1].Z);
	}
	glEnd();

	glBegin(GL_TRIANGLES);

	for (int jj = 0; jj < 20; jj++) {
		glTexCoord2d(milk_c[mmilk3[jj].T1 - 1].X, milk_c[mmilk3[jj].T1 - 1].Y);
		glVertex3f(milk_v[mmilk3[jj].V1 - 1].X, milk_v[mmilk3[jj].V1 - 1].Y, milk_v[mmilk3[jj].V1 - 1].Z);
		glTexCoord2d(milk_c[mmilk3[jj].T2 - 1].X, milk_c[mmilk3[jj].T2 - 1].Y);
		glVertex3f(milk_v[mmilk3[jj].V2 - 1].X, milk_v[mmilk3[jj].V2 - 1].Y, milk_v[mmilk3[jj].V2 - 1].Z);
		glTexCoord2d(milk_c[mmilk3[jj].T3 - 1].X, milk_c[mmilk3[jj].T3 - 1].Y);
		glVertex3f(milk_v[mmilk3[jj].V3 - 1].X, milk_v[mmilk3[jj].V3 - 1].Y, milk_v[mmilk3[jj].V3 - 1].Z);
	}
	glEnd();

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 319, 319, 0, GL_RGB, GL_UNSIGNED_BYTE, wood2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	for (int jj = 0; jj < 2100; jj++) {
		glTexCoord2d(tray_c[mtray[jj].T1 - 1].X, tray_c[mtray[jj].T1 - 1].Y);
		glVertex3f(tray_v[mtray[jj].V1 - 1].X, tray_v[mtray[jj].V1 - 1].Y, tray_v[mtray[jj].V1 - 1].Z);
		glTexCoord2d(tray_c[mtray[jj].T2 - 1].X, tray_c[mtray[jj].T2 - 1].Y);
		glVertex3f(tray_v[mtray[jj].V2 - 1].X, tray_v[mtray[jj].V2 - 1].Y, tray_v[mtray[jj].V2 - 1].Z);
		glTexCoord2d(tray_c[mtray[jj].T3 - 1].X, tray_c[mtray[jj].T3 - 1].Y);
		glVertex3f(tray_v[mtray[jj].V3 - 1].X, tray_v[mtray[jj].V3 - 1].Y, tray_v[mtray[jj].V3 - 1].Z);
		glTexCoord2d(tray_c[mtray[jj].T4 - 1].X, tray_c[mtray[jj].T4 - 1].Y);
		glVertex3f(tray_v[mtray[jj].V4 - 1].X, tray_v[mtray[jj].V4 - 1].Y, tray_v[mtray[jj].V4 - 1].Z);
	}

	glEnd();

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1000, 1000, 0, GL_RGB, GL_UNSIGNED_BYTE, wood_texel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);


	glBegin(GL_QUADS);
	for (int jj = 0; jj < 23616; jj++) {
		glTexCoord2d(table_c[mtable[jj].T1 - 1].X, table_c[mtable[jj].T1 - 1].Y);
		glVertex3f(table_v[mtable[jj].V1 - 1].X, table_v[mtable[jj].V1 - 1].Y, table_v[mtable[jj].V1 - 1].Z);
		glTexCoord2d(table_c[mtable[jj].T2 - 1].X, table_c[mtable[jj].T2 - 1].Y);
		glVertex3f(table_v[mtable[jj].V2 - 1].X, table_v[mtable[jj].V2 - 1].Y, table_v[mtable[jj].V2 - 1].Z);
		glTexCoord2d(table_c[mtable[jj].T3 - 1].X, table_c[mtable[jj].T3 - 1].Y);
		glVertex3f(table_v[mtable[jj].V3 - 1].X, table_v[mtable[jj].V3 - 1].Y, table_v[mtable[jj].V3 - 1].Z);
		glTexCoord2d(table_c[mtable[jj].T4 - 1].X, table_c[mtable[jj].T4 - 1].Y);
		glVertex3f(table_v[mtable[jj].V4 - 1].X, table_v[mtable[jj].V4 - 1].Y, table_v[mtable[jj].V4 - 1].Z);
	}
	glEnd();

	glTranslatef(0.0f, 0.0f, -0.1f);
	glBegin(GL_QUADS);
	for (int jj = 0; jj < 43698; jj++) {
		glTexCoord2d(stool_c[mstool[jj].T1 - 1].X, stool_c[mstool[jj].T1 - 1].Y);
		glVertex3f(stool_v[mstool[jj].V1 - 1].X, stool_v[mstool[jj].V1 - 1].Y, stool_v[mstool[jj].V1 - 1].Z);
		glTexCoord2d(stool_c[mstool[jj].T2 - 1].X, stool_c[mstool[jj].T2 - 1].Y);
		glVertex3f(stool_v[mstool[jj].V2 - 1].X, stool_v[mstool[jj].V2 - 1].Y, stool_v[mstool[jj].V2 - 1].Z);
		glTexCoord2d(stool_c[mstool[jj].T3 - 1].X, stool_c[mstool[jj].T3 - 1].Y);
		glVertex3f(stool_v[mstool[jj].V3 - 1].X, stool_v[mstool[jj].V3 - 1].Y, stool_v[mstool[jj].V3 - 1].Z);
		glTexCoord2d(stool_c[mstool[jj].T4 - 1].X, stool_c[mstool[jj].T4 - 1].Y);
		glVertex3f(stool_v[mstool[jj].V4 - 1].X, stool_v[mstool[jj].V4 - 1].Y, stool_v[mstool[jj].V4 - 1].Z);
	}
	glEnd();

	glBegin(GL_TRIANGLES);
	for (int jj = 0; jj < 510; jj++) {
		glTexCoord2d(stool_c[mstool3[jj].T1 - 1].X, stool_c[mstool3[jj].T1 - 1].Y);
		glVertex3f(stool_v[mstool3[jj].V1 - 1].X, stool_v[mstool3[jj].V1 - 1].Y, stool_v[mstool3[jj].V1 - 1].Z);
		glTexCoord2d(stool_c[mstool3[jj].T2 - 1].X, stool_c[mstool3[jj].T2 - 1].Y);
		glVertex3f(stool_v[mstool3[jj].V2 - 1].X, stool_v[mstool3[jj].V2 - 1].Y, stool_v[mstool3[jj].V2 - 1].Z);
		glTexCoord2d(stool_c[mstool3[jj].T3 - 1].X, stool_c[mstool3[jj].T3 - 1].Y);
		glVertex3f(stool_v[mstool3[jj].V3 - 1].X, stool_v[mstool3[jj].V3 - 1].Y, stool_v[mstool3[jj].V3 - 1].Z);

	}
	glEnd();

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 2000, 2000, 0, GL_RGB, GL_UNSIGNED_BYTE,box);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	for (int jj = 0; jj < 1176; jj++){
		glTexCoord2d(box_c[mbox[jj].T1 - 1].X, box_c[mbox[jj].T1 - 1].Y);
		glVertex3f(box_v[mbox[jj].V1 - 1].X, box_v[mbox[jj].V1 - 1].Y, box_v[mbox[jj].V1 - 1].Z);
		glTexCoord2d(box_c[mbox[jj].T2 - 1].X, box_c[mbox[jj].T2 - 1].Y);
		glVertex3f(box_v[mbox[jj].V2 - 1].X, box_v[mbox[jj].V2 - 1].Y, box_v[mbox[jj].V2 - 1].Z);
		glTexCoord2d(box_c[mbox[jj].T3 - 1].X, box_c[mbox[jj].T3 - 1].Y);
		glVertex3f(box_v[mbox[jj].V3 - 1].X, box_v[mbox[jj].V3 - 1].Y, box_v[mbox[jj].V3 - 1].Z);
		glTexCoord2d(box_c[mbox[jj].T4 - 1].X, box_c[mbox[jj].T4 - 1].Y);
		glVertex3f(box_v[mbox[jj].V4 - 1].X, box_v[mbox[jj].V4 - 1].Y, box_v[mbox[jj].V4 - 1].Z);
	}
	glEnd();


	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	v_vertex = new Vertex[100000];
	vertex_color = new Vertex[100000];
	//mymesh = new Meshmodel[100000];
	
	int i,j,k=0;
	FILE* f = fopen("carpet.bmp", "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);
	for (i = 0; i < width; i ++)
		for (j = 0; j < height; j ++)
		{
			mytexels[j][i][0] = data[k * 3 + 2];
			mytexels[j][i][1] = data[k * 3 + 1];
			mytexels[j][i][2] = data[k * 3];
			k++;
		}

	k = 0;
	FILE* f2 = fopen("mymap.bmp", "rb");
	unsigned char info2[54];
	fread(info2, sizeof(unsigned char), 54, f2); // read the 54-byte header
	int width2 = *(int*)&info2[18];
	int height2 = *(int*)&info2[22];
	int size2 = 3 * width2 * height2;
	unsigned char* data2 = new unsigned char[size2]; // allocate 3 bytes per pixel
	fread(data2, sizeof(unsigned char), size2, f2); // read the rest of the data at once
	fclose(f2);
	for (i = 0; i < width2; i++)
		for (j = 0; j < height2; j++)
		{
			wall[j][i][0] = data2[k * 3 + 2];
			wall[j][i][1] = data2[k * 3 + 1];
			wall[j][i][2] = data2[k * 3];
			k++;
		}
	//milk
	FILE* f3 = fopen("milk.bmp", "r");
	unsigned char info3[54];
	fread(info3, sizeof(unsigned char), 54, f3);

	int width3 = *(int*)&info3[18];
	int height3 = *(int*)&info3[22];
	int size3 = 3 * width3 * height3;
	unsigned char* data3 = new unsigned char[size3];
	fread(data3, sizeof(unsigned char), size3, f3);
	fclose(f3);

	k = 0;
	for (i = 0; i < width3; i++) {
		for (j = 0; j < height3; j++) {
			milk_texel[j][i][0] = data3[k * 3 + 2];
			milk_texel[j][i][1] = data3[k * 3 + 1];
			milk_texel[j][i][2] = data3[k * 3];
			k++;
		}
	}
	milk_v = new Vertex[100000];
	milk_c = new Vertex[100000];
	mmilk = new MMesh[100000];
	mmilk3 = new MMesh3[10000];

	FILE* fpp;
	fpp = fopen("milk.obj", "r");
	int count = 0;
	int num = 0;
	char ch;
	float x, y, z;
	float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

	char buffer[256];
	count = 0;
	int cnt = 0, cnt2 = 0;
	int cnt3 = 0, cnt4 = 0;
	while (fscanf(fpp, "%s", buffer) != EOF) {

		if (!strcmp("vt", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 3) {
				milk_c[cnt2].X = x;
				milk_c[cnt2].Y = y;
				milk_c[cnt2].Z = z;
				cnt2++;
			}
		}
		else if (!strcmp("v", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 3) {
				milk_v[cnt].X = (x / 3000)+0.07f;
				milk_v[cnt].Y = (y / 3000) + 0.2f;
				milk_v[cnt].Z = (z / 3000) + 0.05;
				cnt++;
			}
		}

		else if (!strcmp("f", buffer)) {
			count = fscanf(fpp, "%f/%f/%f %f/%f/%f %f/%f/%f %f/%f/%f\n", &x1, &y1,&z1, &x2, &y2,&z2, &x3, &y3,&z3, &x4, &y4,&z4);
			if (count == 12) {
				mmilk[cnt3].V1 = x1;
				mmilk[cnt3].V2 = x2;
				mmilk[cnt3].V3 = x3;
				mmilk[cnt3].V4 = x4;
				mmilk[cnt3].T1 = y1;
				mmilk[cnt3].T2 = y2;
				mmilk[cnt3].T3 = y3;
				mmilk[cnt3].T4 = y4;
				cnt3++;
			}
			else if (count == 9) {
				mmilk3[cnt4].V1 = x1;
				mmilk3[cnt4].V2 = x2;
				mmilk3[cnt4].V3 = x3;
				mmilk3[cnt4].T1 = y1;
				mmilk3[cnt4].T2 = y2;
				mmilk3[cnt4].T3 = y3;
				cnt4++;
			}
		}
		else continue;
	}

	tray_v = new Vertex[100000];
	tray_c = new Vertex[100000];
	mtray = new MMesh[100000];

	FILE* f4 = fopen("wood.bmp", "r");
	unsigned char info4[54];
	fread(info4, sizeof(unsigned char), 54, f4);

	int width4 = *(int*)&info4[18];
	int height4 = *(int*)&info4[22];
	int size4 = 3 * width4 * height4;
	unsigned char* data4 = new unsigned char[size4];
	fread(data4, sizeof(unsigned char), size4, f4);
	fclose(f4);

	k = 0;
	for (i = 0; i < width4; i++) {
		for (j = 0; j < height4; j++) {
			wood_texel[j][i][0] = data4[k * 3 + 2];
			wood_texel[j][i][1] = data4[k * 3 + 1];
			wood_texel[j][i][2] = data4[k * 3];
			k++;
		}
	}


	fpp = fopen("tray.obj", "r");

	for (j = 0; j < 100000; j++) {
		count = fscanf(fpp, "v %f %f %f\n", &x, &y, &z);
		if (count == 3) {
			tray_v[j].X = (x /100)+0.005;
			tray_v[j].Y = (y / 100)+0.17;
			tray_v[j].Z = (z / 100)+0.05;
		}
	}

	fclose(fpp);
	FILE* fp = fopen("tray_vt.txt", "r");
	if (fp == NULL) {
		cout << "파일 없음";
	}
	for (j = 0; j < 100000; j++) {
		count = fscanf(f, "vt %f %f %f\n", &x, &y, &z);
		if (count == 3) {
			tray_c[j].X = x;
			tray_c[j].Y = y;
			tray_c[j].Z = z;
		}
		else break;
	}
	fclose(fp);
	FILE* pf = fopen("tray_f.txt", "r");
	cnt = 0;
	for (j = 0; j < 100000; j++) {
		count = fscanf(pf, "f %f/%f/%f %f/%f/%f %f/%f/%f %f/%f/%f\n", &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3, &x4, &y4, &z4);
		if (count == 12) {
			mtray[cnt].V1 = x1;
			mtray[cnt].V2 = x2;
			mtray[cnt].V3 = x3;
			mtray[cnt].V4 = x4;
			mtray[cnt].T1 = y1;
			mtray[cnt].T2 = y2;
			mtray[cnt].T3 = y3;
			mtray[cnt].T4 = y4;
			cnt++;
		}
	}
	fclose(pf);

	table_v = new Vertex[100000];
	table_c = new Vertex[100000];
	mtable = new MMesh[100000];

	FILE* f5 = fopen("wood2_small.bmp", "r");
	unsigned char info5[54];
	fread(info5, sizeof(unsigned char), 54, f5);

	int width5 = *(int*)&info5[18];
	int height5 = *(int*)&info5[22];
	int size5 = 3 * width5 * height5;
	unsigned char* data5 = new unsigned char[size5];
	fread(data5, sizeof(unsigned char), size5, f5);
	fclose(f5);


	k = 0;
	for (i = 0; i < width5; i++) {
		for (j = 0; j < height5; j++) {
			wood2[j][i][0] = data5[k * 3 + 2];
			wood2[j][i][1] = data5[k * 3 + 1];
			wood2[j][i][2] = data5[k * 3];
			k++;
		}
	}

	fpp = fopen("table.obj", "r");
	count = 0;
	cnt = 0, cnt2 = 0, cnt3 = 0;
	float x5, y5, z5;
	while (fscanf(fpp, "%s", buffer) != EOF) {

		if (!strcmp("vt", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 3) {
				table_c[cnt2].X = x;
				table_c[cnt2].Y = y;
				table_c[cnt2].Z = z;
				cnt2++;
			}
		}
		else if (!strcmp("v", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 3) {
				table_v[cnt].X = (x / 100) - 1.45;
				table_v[cnt].Y = (y / 100) + 0.09;
				table_v[cnt].Z = (z / 100)+0.05;
				cnt++;
			}
		}

		else if (!strcmp("f", buffer)) {
			count = fscanf(fpp, "%f/%f %f/%f %f/%f %f/%f\n", &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4);
			if (count == 8) {
				mtable[cnt3].V1 = x1;
				mtable[cnt3].V2 = x2;
				mtable[cnt3].V3 = x3;
				mtable[cnt3].V4 = x4;
				mtable[cnt3].T1 = y1;
				mtable[cnt3].T2 = y2;
				mtable[cnt3].T3 = y3;
				mtable[cnt3].T4 = y4;
				cnt3++;
			}
		}
		else continue;
	}
	fpp = fopen("Stool.obj", "r");
	count = 0;
	cnt = 0, cnt2 = 0,cnt3 = 0, cnt4 = 0;

	stool_c = new Vertex[100000];
	stool_v = new Vertex[100000];
	mstool = new MMesh[100000];
	mstool3 = new MMesh3[10000];

	while (fscanf(fpp, "%s", buffer) != EOF) {

		if (!strcmp("vt", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 3) {
				stool_c[cnt2].X = x;
				stool_c[cnt2].Y = y;
				stool_c[cnt2].Z = z;
				cnt2++;
			}
		}
		else if (!strcmp("v", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 3) {
				stool_v[cnt].X = (x / 320);
				stool_v[cnt].Y = (y / 320);
				stool_v[cnt].Z = (z / 320);
				cnt++;
			}
		}

		else if (!strcmp("f", buffer)) {
			count = fscanf(fpp,"%f/%f/%f %f/%f/%f %f/%f/%f %f/%f/%F\n", &x1, &y1,&z1, &x2, &y2, &z2, &x3, &y3,&z3, &x4, &y4, &z4);
			if (count == 12) {
				mstool[cnt3].V1 = x1;
				mstool[cnt3].V2 = x2;
				mstool[cnt3].V3 = x3;
				mstool[cnt3].V4 = x4;
				mstool[cnt3].T1 = y1;
				mstool[cnt3].T2 = y2;
				mstool[cnt3].T3 = y3;
				mstool[cnt3].T4 = y4;
				cnt3++;
			}
			else if (count == 9) {
				mstool3[cnt4].V1 = x1;
				mstool3[cnt4].V2 = x2;
				mstool3[cnt4].V3 = x3;
				mstool3[cnt4].T1 = y1;
				mstool3[cnt4].T2 = y2;
				mstool3[cnt4].T3 = y3;
				cnt4++;
			}
		}
		else continue;
	}
	fclose(fpp);

	cup_c = new Vertex[100000];
	cup_v = new Vertex[100000];
	mcup = new MMesh[100000];

	fpp = fopen("cup.obj", "r");
	count = 0;
	cnt = 0, cnt2 = 0, cnt3 = 0;
	while (fscanf(fpp, "%s", buffer) != EOF) {

		if (!strcmp("vt", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 3) {
				cup_c[cnt2].X = x;
				cup_c[cnt2].Y = y;
				cup_c[cnt2].Z = z;
				cnt2++;
			}
		}
		else if (!strcmp("v", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 3) {
				cup_v[cnt].X = (x / 500)+0.05f;
				cup_v[cnt].Y = (y / 500)+0.2f;
				cup_v[cnt].Z = (z / 500);
				cnt++;
			}
		}

		else if (!strcmp("f", buffer)) {
			count = fscanf(fpp, "%f/%f/%f %f/%f/%f %f/%f/%f %f/%f/%f\n", &x1, &y1,&z1, &x2, &y2, &z2,&x3, &y3, &z3,&x4, &y4, &z4);
			if (count == 12) {
				mcup[cnt3].V1 = x1;
				mcup[cnt3].V2 = x2;
				mcup[cnt3].V3 = x3;
				mcup[cnt3].V4 = x4;
				mcup[cnt3].T1 = y1;
				mcup[cnt3].T2 = y2;
				mcup[cnt3].T3 = y3;
				mcup[cnt3].T4 = y4;
				cnt3++;
			}
		}
		else continue;
	}

	FILE* f6 = fopen("Box.bmp", "r");
	unsigned char info6[54];
	fread(info6, sizeof(unsigned char), 54, f6);

	int width6= *(int*)&info6[18];
	int height6 = *(int*)&info6[22];
	int size6 = 3 * width6 * height6;
	unsigned char* data6 = new unsigned char[size6];
	fread(data6, sizeof(unsigned char), size6, f6);
	fclose(f6);

	k = 0;
	for (i = 0; i < width6; i++) {
		for (j = 0; j < height6; j++) {
			box[j][i][0] = data6[k * 3 + 2];
			box[j][i][1] = data6[k * 3 + 1];
			box[j][i][2] = data6[k * 3];
			k++;
		}
	}

	fpp = fopen("box.obj", "r");
	count = 0;
	cnt = 0, cnt2 = 0, cnt3 = 0;
	box_c = new Vertex[1500];
	box_v = new Vertex[1500];
	mbox = new MMesh[1500];
	while (fscanf(fpp, "%s", buffer) != EOF) {

		if (!strcmp("vt", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 2) {
				box_c[cnt2].X = x;
				box_c[cnt2].Y = y;
				box_c[cnt2].Z = 0.00000;
				cnt2++;
			}
		}
		else if (!strcmp("v", buffer)) {
			count = fscanf(fpp, "%f %f %f\n", &x, &y, &z);
			if (count == 3) {
				box_v[cnt].X = (x/2) - 0.05f;
				box_v[cnt].Y = (y/2 ) +0.2f;
				box_v[cnt].Z = (z/2)+0.12f;
				cnt++;
			}
		}

		else if (!strcmp("f", buffer)) {
			count = fscanf(fpp, "%f/%f/%f %f/%f/%f %f/%f/%f %f/%f/%f\n", &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3, &x4, &y4, &z4);
			if (count == 12) {
				mbox[cnt3].V1 = x1;
				mbox[cnt3].V2 = x2;
				mbox[cnt3].V3 = x3;
				mbox[cnt3].V4 = x4;
				mbox[cnt3].T1 = y1;
				mbox[cnt3].T2 = y2;
				mbox[cnt3].T3 = y3;
				mbox[cnt3].T4 = y4;
				cnt3++;
			}
		}
		else continue;
	}

	InitializeWindow(argc, argv);

	display();

	glutMainLoop();
	delete[] v_vertex;
	delete[] vertex_color;
	return 0;
}