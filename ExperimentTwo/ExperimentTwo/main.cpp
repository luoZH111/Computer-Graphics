#include "include/Angel.h"
#include "TriMesh.h"

#include <cstdlib>
#include <iostream>

#pragma comment(lib, "glew32.lib")

using namespace std;

GLuint programID;
GLuint vertexArrayID;
GLuint vertexBufferID;
GLuint vertexNormalID;
GLuint vertexIndexBuffer;

GLuint vPositionID;
GLuint vNormalID;
GLuint transformMatrixID;
GLuint modelViewMatrixID;
GLuint projMatrixID;

GLuint drawShadowID;
GLuint shadowColorID;

GLuint lightPosID;

// 光源位置（阴影投影中心）
float xpos = -0.5;
float ypos = 2.0;
float zpos = 0.5;
float lightPos[3] = { xpos, ypos, zpos };

// 相机设置参数（位置和角度）
float rad = 4.0;
float tAngle = 15.0;
float pAngle = 0.0;

// 物体旋转参数
float translateY = 0.0;

TriMesh *mesh = new TriMesh;

namespace Camera
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projMatrix;

	// 正交投影矩阵
	mat4 ortho(const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar)
	{
		mat4 c;
		c[0][0] = 2.0 / (right - left);
		c[1][1] = 2.0 / (top - bottom);
		c[2][2] = 2.0 / (zNear - zFar);
		c[3][3] = 1.0;
		c[0][3] = -(right + left) / (right - left);
		c[1][3] = -(top + bottom) / (top - bottom);
		c[2][3] = -(zFar + zNear) / (zFar - zNear);
		return c;
	}

	// 透视投影矩阵
	mat4 perspective(const GLfloat fovy, const GLfloat aspect,
		const GLfloat zNear, const GLfloat zFar)
	{
		GLfloat top = tan(fovy*DegreesToRadians / 2) * zNear;
		GLfloat right = top * aspect;

		mat4 c;
		c[0][0] = zNear / right;
		c[1][1] = zNear / top;
		c[2][2] = -(zFar + zNear) / (zFar - zNear);
		c[2][3] = (-2.0*zFar*zNear) / (zFar - zNear);
		c[3][2] = -1.0;
		c[3][3] = 0.0;
		return c;
	}

	// 相机模视变换矩阵
	mat4 lookAt(const vec4& eye, const vec4& at, const vec4& up)
	{
		vec4 n = normalize(at - eye);
		vec4 u = normalize(vec4(cross(n, up), 0.0));
		vec4 v = normalize(vec4(cross(u, n), 0.0));

		vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
		mat4 c = mat4(u, v, -n, t);
		return c * Translate(-eye);
	}
}

// OpenGL初始化
void init()
{
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// 读入三维模型，并计算法向量
	mesh->read_off("sphere.off");

	std::vector<vec3f> vs = mesh->v();
	std::vector<vec3i> fs = mesh->f();
	std::vector<vec3f> ns;

	for (int i = 0; i < vs.size(); ++i) {
		ns.push_back(vs[i] - vec3(0.0, 0.0, 0.0));
	}

	// 为方便观察阴影，将球往上移动0.5。
	for (int i = 0; i < vs.size(); ++i) {
		vs[i] += vec3(0.0, 0.5, 0.0);
	}

	programID = InitShader("vshader.glsl", "fshader.glsl");
	vPositionID = glGetAttribLocation(programID, "vPosition");
	vNormalID = glGetAttribLocation(programID, "vNormal");

	transformMatrixID = glGetUniformLocation(programID, "transformMatrix");
	modelViewMatrixID = glGetUniformLocation(programID, "modelViewMatrix");
	projMatrixID = glGetUniformLocation(programID, "projMatrix");

	drawShadowID = glGetUniformLocation(programID, "drawShadow");
	shadowColorID = glGetUniformLocation(programID, "shadowColor");

	lightPosID = glGetUniformLocation(programID, "lightPos");

	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(vec3f), vs.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &vertexNormalID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexNormalID);
	glBufferData(GL_ARRAY_BUFFER, ns.size() * sizeof(vec3f), ns.data(), GL_STATIC_DRAW);;

	glGenBuffers(1, &vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, fs.size() * sizeof(vec3i), fs.data(), GL_STATIC_DRAW);;

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}

// 渲染
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);

	glEnableVertexAttribArray(vPositionID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(
		vPositionID,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glEnableVertexAttribArray(vNormalID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexNormalID);
	glVertexAttribPointer(
		vNormalID,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);

	//////////////////////////////////////////////////////////////////////////
	// 绘制原始三维模型
	glUniform3fv(lightPosID, 1, &lightPos[0]);

	float z = rad * cos(tAngle * DegreesToRadians) * cos(pAngle * DegreesToRadians);
	float x = rad * cos(tAngle * DegreesToRadians) * sin(pAngle * DegreesToRadians);
	float y = rad * sin(tAngle * DegreesToRadians);

	Camera::modelMatrix = Translate(vec3(0.0, translateY, 0.0)) * mat4(1.0);
	Camera::viewMatrix = Camera::lookAt(vec4(x, y, z, 1), vec4(0, 0, 0, 1), vec4(0, 1, 0, 0.0));

	mat4 modelViewMatrix = Camera::viewMatrix * Camera::modelMatrix;
	mat4 projMatrix = Camera::perspective(45.0, 1.0, 0.1, 100);
	//mat4 projMatrix = Camera::ortho(-5, 5, -5, 5, -5, 5);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, &modelViewMatrix[0][0]);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, &projMatrix[0][0]);

	glUniform1i(drawShadowID, 0);
	glDrawElements(GL_TRIANGLES, int(mesh->f().size() * 3), GL_UNSIGNED_INT, (void*)0);

	//////////////////////////////////////////////////////////////////////////
	// 绘制阴影
	float lx = lightPos[0];
	float ly = lightPos[1];
	float lz = lightPos[2];

	// 计算阴影投影矩阵
	mat4 shadowProjMatrix(-ly, 0.0, 0.0, 0.0,
		lx, 0.0, lz, 1.0,
		0.0, 0.0, -ly, 0.0,
		0.0, 0.0, 0.0, -ly);

	vec3 shadowColor = vec3(0.0, 0.0, 0.0);
	shadowProjMatrix = Camera::viewMatrix * shadowProjMatrix  * Camera::modelMatrix;

	glUniform1i(drawShadowID, 1);
	glUniform3fv(shadowColorID, 1, &shadowColor[0]);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, &shadowProjMatrix[0][0]);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, &projMatrix[0][0]);

	glDrawElements(GL_TRIANGLES, int(mesh->f().size() * 3), GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(vPositionID);
	glDisableVertexAttribArray(vNormalID);
	glUseProgram(0);

	glutSwapBuffers();
}

// 窗口变形事件
void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
}

// 鼠标响应事件
void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		int h = glutGet(GLUT_WINDOW_HEIGHT);
		int w = glutGet(GLUT_WINDOW_WIDTH);

		int pos = (h - y) - h / 2;
		float delta = float(pos) / float(h);

		lightPos[1] = ypos + delta;
	}

	glutPostRedisplay();
	return;
}

// 键盘响应事件
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'x': pAngle += 5.0; break;
	case 'X': pAngle -= 5.0; break;

	case 'z': rad += 0.1; break;
	case 'Z': rad -= 0.1; break;

	case 'y': translateY += 0.1; break;
	case 'Y': translateY -= 0.1; break;

	case ' ': rad = 4.0; tAngle = 15.0; pAngle = 0.0; translateY = 0.0; break;

	case 033: // Both escape key and 'q' cause the game to exit
	case 'q':
		exit(EXIT_SUCCESS);
		break;
	}
	glutPostRedisplay();
}

// 空闲函数
void idle(void)
{
	glutPostRedisplay();
}

void clean()
{
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &vertexArrayID);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	glutCreateWindow("OpenGL-Tutorial");

	glewInit();
	init();

	// 回调函数
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop();

	clean();

	return 0;
}