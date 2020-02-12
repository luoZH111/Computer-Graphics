#include "Angel.h"
#include "mat.h"

#include<vector>

#pragma comment(lib, "glew32.lib")

const int X_AXIS = 0;
const int Y_AXIS = 1;
const int Z_AXIS = 2;

const int TRANSFORM_SCALE = 0;
const int TRANSFORM_ROTATE = 1;
const int TRANSFORM_TRANSLATE = 2;

const double DELTA_DELTA = 0.1;
const double DEFAULT_DELTA = 0.3;

double scaleDelta = DEFAULT_DELTA;
double rotateDelta = DEFAULT_DELTA;
double translateDelta = DEFAULT_DELTA;

vec3 scaleTheta(1.0, 1.0, 1.0);
vec3 rotateTheta(0.0, 0.0, 0.0);
vec3 translateTheta(0.0, 0.0, 0.0);

GLint matrixLocation;
int currentTransform = TRANSFORM_TRANSLATE;
int mainWindow;
const int NUM_VERTICES = 8;

// 单位立方体的各个点
const vec4 vertices[NUM_VERTICES] = {
	vec4(-0.5, -0.5,  0.5, 1.0),
	vec4(-0.5,  0.5,  0.5, 1.0),
	vec4(0.5,  0.5,  0.5, 1.0),
	vec4(0.5, -0.5,  0.5, 1.0),
	vec4(-0.5, -0.5, -0.5, 1.0),
	vec4(-0.5,  0.5, -0.5, 1.0),
	vec4(0.5,  0.5, -0.5, 1.0),
	vec4(0.5, -0.5, -0.5, 1.0)
};

// 每个顶点的对应颜色
const vec4 vertexColors[NUM_VERTICES] = {
	vec4(0.0, 0.0, 0.0, 1.0),  // Black
	vec4(1.0, 0.0, 0.0, 1.0),  // Red
	vec4(1.0, 1.0, 0.0, 1.0),  // Yellow
	vec4(0.0, 1.0, 0.0, 1.0),  // Green
	vec4(0.0, 0.0, 1.0, 1.0),  // Blue
	vec4(1.0, 0.0, 1.0, 1.0),  // Magenta
	vec4(1.0, 1.0, 1.0, 1.0),  // White
	vec4(0.0, 1.0, 1.0, 1.0)   // Cyan
};

std::vector<vec4> points;
std::vector<vec4> colors;

// 存储每个三角的各个顶点并为点赋色
void storeTrianglePoints(int a, int b, int c)
{
	points.push_back(vertices[a]);
	points.push_back(vertices[b]);
	points.push_back(vertices[c]);

	colors.push_back(vertexColors[a]);
	colors.push_back(vertexColors[b]);
	colors.push_back(vertexColors[c]);
}

// 立方体生成12个三角形：36个顶点和对应颜色
void generateCube()
{
	// @TODO: 修改此函数调用storeTrianglePoints(),
	// 存储每个三角形的顶点位置和颜色信息。
	storeTrianglePoints(0, 1, 2);
	storeTrianglePoints(0, 3, 2);
	storeTrianglePoints(0, 1, 5);
	storeTrianglePoints(0, 4, 5);
	storeTrianglePoints(0, 3, 7);
	storeTrianglePoints(0, 4, 7);
	storeTrianglePoints(1, 2, 6);
	storeTrianglePoints(1, 5, 6);
	storeTrianglePoints(2, 3, 7);
	storeTrianglePoints(2, 6, 7);
	storeTrianglePoints(4, 5, 6);
	storeTrianglePoints(4, 7, 6);

}

void init()
{
	generateCube();

	// 创建顶点数组对象
	GLuint vao[1];
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	// 创建并初始化顶点缓存对象
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vec4) + colors.size() * sizeof(vec4),
		NULL, GL_STATIC_DRAW);

	// @TODO：修改完成后再打开下面注释，否则程序会报错
	// 分别读取数据
	glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(vec4), &points[0]);
	glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(vec4), colors.size() * sizeof(vec4), &colors[0]);

	// 读取着色器并使用
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// 从顶点着色器中初始化顶点的位置
	GLuint pLocation = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(pLocation);
	glVertexAttribPointer(pLocation, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	// 从片元着色器中初始化顶点的颜色
	GLuint cLocation = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(cLocation);
	glVertexAttribPointer(cLocation, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(points.size() * sizeof(vec4)));
	// 获得矩阵存储位置
	matrixLocation = glGetUniformLocation(program, "matrix");

	// 白色背景
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void display()
{
	// 清理窗口
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 生成变换矩阵
	mat4 m(1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);

	// @TODO: 在此处修改函数，计算变换矩阵
	m = Translate(translateTheta.x, translateTheta.y, translateTheta.z)
		*RotateX(rotateTheta.x)
		*RotateY(rotateTheta.y)
		*RotateZ(rotateTheta.z)
		*Scale(scaleTheta.x, scaleTheta.y, scaleTheta.z);

	// 从指定位置中传入变换矩阵
	glUniformMatrix4fv(matrixLocation, 1, GL_TRUE, m);
	glDrawArrays(GL_TRIANGLES, 0, points.size());

	glutSwapBuffers();
}

// 通过Delta值更新Theta
void updateTheta(int axis, int sign) {
	switch (currentTransform) {
	case TRANSFORM_SCALE:
		scaleTheta[axis] += sign * scaleDelta;
		break;
	case TRANSFORM_ROTATE:
		rotateTheta[axis] += sign * rotateDelta;
		break;
	case TRANSFORM_TRANSLATE:
		translateTheta[axis] += sign * translateDelta;
		break;
	}
}

// 复原Theta和Delta
void resetTheta()
{
	scaleTheta = vec3(1.0, 1.0, 1.0);
	rotateTheta = vec3(0.0, 0.0, 0.0);
	translateTheta = vec3(0.0, 0.0, 0.0);
	scaleDelta = DEFAULT_DELTA;
	rotateDelta = DEFAULT_DELTA;
	translateDelta = DEFAULT_DELTA;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
		updateTheta(X_AXIS, 1);
		break;
	case 'a':
		updateTheta(X_AXIS, -1);
		break;
	case 'w':
		updateTheta(Y_AXIS, 1);
		break;
	case 's':
		updateTheta(Y_AXIS, -1);
		break;
	case 'e':
		updateTheta(Z_AXIS, 1);
		break;
	case 'd':
		updateTheta(Z_AXIS, -1);
		break;
	case 't':
		resetTheta();
		break;
	case 033:
		// Esc按键
		exit(EXIT_SUCCESS);
		break;
	}
	glutPostWindowRedisplay(mainWindow);
}

void menuEvents(int menuChoice)
{
	currentTransform = menuChoice;
}

void setupMenu()
{
	glutCreateMenu(menuEvents);
	glutAddMenuEntry("Scale", TRANSFORM_SCALE);
	glutAddMenuEntry("Rotate", TRANSFORM_ROTATE);
	glutAddMenuEntry("Translate", TRANSFORM_TRANSLATE);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

void printHelp() {
	printf("%s\n\n", "3D Transfomations");
	printf("Keyboard options:\n");
	printf("q: Increase x\n");
	printf("a: Decrease x\n");
	printf("w: Increase y\n");
	printf("s: Decrease y\n");
	printf("e: Increase z\n");
	printf("d: Decrease z\n");
	printf("t: Reset all transformations and deltas\n");
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);    // 窗口支持双重缓冲和深度测试
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	mainWindow = glutCreateWindow("3D Transfomations");

	glewExperimental = GL_TRUE;
	glewInit();

	init();
	setupMenu();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	// 输出帮助信息
	printHelp();
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
	return 0;
}
