#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <float.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#define _CRT_SECURE_NO_WARNINGS

struct Vector3
{
	float			x, y, z;
};

struct Triangle
{
	unsigned int 	indices[3];
};

std::vector<Vector3>	gPositions;
std::vector<Vector3>	gNormals;
std::vector<Triangle>	gTriangles;

void tokenize(char* string, std::vector<std::string>& tokens, const char* delimiter)
{
	char* token = strtok(string, delimiter);
	while (token != NULL)
	{
		tokens.push_back(std::string(token));
		token = strtok(NULL, delimiter);
	}
}

int face_index(const char* string)
{
	int length = strlen(string);
	char* copy = new char[length + 1];
	memset(copy, 0, length + 1);
	strcpy(copy, string);

	std::vector<std::string> tokens;
	tokenize(copy, tokens, "/");
	delete[] copy;
	if (tokens.front().length() > 0 && tokens.back().length() > 0 && atoi(tokens.front().c_str()) == atoi(tokens.back().c_str()))
	{
		return atoi(tokens.front().c_str());
	}
	else
	{
		printf("ERROR: Bad face specifier!\n");
		exit(0);
	}
}

void load_mesh(std::string fileName)
{
	std::ifstream fin(fileName.c_str());
	if (!fin.is_open())
	{
		printf("ERROR: Unable to load mesh from %s!\n", fileName.c_str());
		exit(0);
	}

	float xmin = FLT_MAX;
	float xmax = -FLT_MAX;
	float ymin = FLT_MAX;
	float ymax = -FLT_MAX;
	float zmin = FLT_MAX;
	float zmax = -FLT_MAX;

	while (true)
	{
		char line[1024] = { 0 };
		fin.getline(line, 1024);

		if (fin.eof())
			break;

		if (strlen(line) <= 1)
			continue;

		std::vector<std::string> tokens;
		tokenize(line, tokens, " ");

		if (tokens[0] == "v")
		{
			float x = atof(tokens[1].c_str());
			float y = atof(tokens[2].c_str());
			float z = atof(tokens[3].c_str());

			xmin = std::min(x, xmin);
			xmax = std::max(x, xmax);
			ymin = std::min(y, ymin);
			ymax = std::max(y, ymax);
			zmin = std::min(z, zmin);
			zmax = std::max(z, zmax);

			Vector3 position = { x, y, z };
			gPositions.push_back(position);
		}
		else if (tokens[0] == "vn")
		{
			float x = atof(tokens[1].c_str());
			float y = atof(tokens[2].c_str());
			float z = atof(tokens[3].c_str());
			Vector3 normal = { x, y, z };
			gNormals.push_back(normal);
		}
		else if (tokens[0] == "f")
		{
			unsigned int a = face_index(tokens[1].c_str());
			unsigned int b = face_index(tokens[2].c_str());
			unsigned int c = face_index(tokens[3].c_str());
			Triangle triangle;
			triangle.indices[0] = a - 1;
			triangle.indices[1] = b - 1;
			triangle.indices[2] = c - 1;
			gTriangles.push_back(triangle);
		}
	}

	fin.close();

	printf("Loaded mesh from %s. (%lu vertices, %lu normals, %lu triangles)\n", fileName.c_str(), gPositions.size(), gNormals.size(), gTriangles.size());
	printf("Mesh bounding box is: (%0.4f, %0.4f, %0.4f) to (%0.4f, %0.4f, %0.4f)\n", xmin, ymin, zmin, xmax, ymax, zmax);
}

float  					gTotalTimeElapsed = 0;
int 					gTotalFrames = 0;
bool					gFPSPrinted1 = false;
bool					gFPSPrinted3 = false;
bool					gFPSPrinted5 = false;
bool					gFPSPrinted10 = false;
GLuint 					gTimer;

void init_timer()
{
	glGenQueries(1, &gTimer);
}

void start_timing()
{
	glBeginQuery(GL_TIME_ELAPSED, gTimer);
}

float stop_timing()
{
	glEndQuery(GL_TIME_ELAPSED);

	GLint available = GL_FALSE;
	while (available == GL_FALSE)
		glGetQueryObjectiv(gTimer, GL_QUERY_RESULT_AVAILABLE, &available);

	GLint result;
	glGetQueryObjectiv(gTimer, GL_QUERY_RESULT, &result);

	float timeElapsed = result / (1000.0f * 1000.0f * 1000.0f);
	return timeElapsed;
}

void render_immediate_mode() {
	glBegin(GL_TRIANGLES);
	for (const Triangle& tri : gTriangles) {
		for (int i = 0; i < 3; ++i) {
			unsigned int idx = tri.indices[i];
			Vector3 normal = gNormals[idx];
			Vector3 pos = gPositions[idx];
			glNormal3f(normal.x, normal.y, normal.z);
			glVertex3f(pos.x, pos.y, pos.z);
		}
	}
	glEnd();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	start_timing();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// 모델 변환
	glTranslatef(0.1f, -1.0f, -1.5f);
	glScalef(10.0f, 10.0f, 10.0f);

	render_immediate_mode();

	float timeElapsed = stop_timing();
	gTotalFrames++;
	gTotalTimeElapsed += timeElapsed;
	float fps = gTotalFrames / gTotalTimeElapsed;

	if (!gFPSPrinted1 && gTotalTimeElapsed >= 1.0f) {
		printf("Average FPS after 1 seconds: %.2f\n", fps);
		gFPSPrinted1 = true;
	}

	if (!gFPSPrinted3 && gTotalTimeElapsed >= 3.0f) {
		printf("Average FPS after 3 seconds: %.2f\n", fps);
		gFPSPrinted3 = true;
	}

	if (!gFPSPrinted5 && gTotalTimeElapsed >= 5.0f) {
		printf("Average FPS after 5 seconds: %.2f\n", fps);
		gFPSPrinted5 = true;
	}

	if (!gFPSPrinted10 && gTotalTimeElapsed >= 10.0f) {
		printf("Average FPS after 10 seconds: %.2f\n", fps);
		gFPSPrinted10 = true;
	}

	char string[1024] = { 0 };
	sprintf(string, "OpenGL Bunny: %0.2f FPS", fps);
	glutSetWindowTitle(string);

	glutPostRedisplay();
	glutSwapBuffers();
}

void reshape(int w, int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.1, 0.1, -0.1, 0.1, 0.1, 1000.0);  // Perspective

	glViewport(0, 0, w, h);  // Viewport 1280 x 1280

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 27 || key == 'q' || key == 'Q')  // ESC or 'q' or 'Q'
	{
		exit(0);  // 종료
	}
}

//------------------ Main Entry Point ------------------
int main(int argc, char** argv) {
	load_mesh("bunny.obj");

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1280, 1280);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("OpenGL Viewer");

	glewInit();

	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);

	// Material properties (bunny)
	float ka[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Ambient
	float kd[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Diffuse
	float ks[] = { 0.0f, 0.0f, 0.0f, 1.0f };  // Specular
	float p = 0.0f;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ka);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ks);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, p);

	// Ambient light (Ia)
	float Ia[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float l[] = { 1.0f, 1.0f, 1.0f, 0.0f };  // w = 0 => directional
	float la[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float ld[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float ls[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Ia);
	glLightfv(GL_LIGHT0, GL_POSITION, l);
	glLightfv(GL_LIGHT0, GL_AMBIENT, la);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ld);
	glLightfv(GL_LIGHT0, GL_SPECULAR, ls);

	init_timer();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}