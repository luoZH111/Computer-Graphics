#include "Angel.h"
#include "mesh.h"
#include "FreeImage.h"
#include "Mesh_Painter.h"
#include <assert.h>
#include <vector>
#include <random>
#include <time.h>
#include "irrKlang/irrKlang.h"
using namespace irrklang;

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "FreeImage.lib")
#pragma comment(lib, "irrKlang.lib")

std::vector<My_Mesh*> my_meshs;
Mesh_Painter*			mp_;

ISoundEngine *SoundEngine = createIrrKlangDevice();

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36;				//(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];				// ��������
color4 colors[NumVertices];				// ��ɫ����
point4 normals[NumVertices];			// ����������

// �����˸���������ɫ
point4 color_hair = point4(0.5, 0.5, 0.5, 1);
point4 color_head = point4(0, 1, 0, 1);
point4 color_neck = point4(0.5, 0.5, 0.5, 1);
point4 color_torso = point4(0, 0, 1, 1);
point4 color_upper_arm = point4(1, 1, 0, 1);
point4 color_lower_arm = point4(1, 0, 0, 1);
point4 color_upper_leg = point4(0, 1, 1, 1);
point4 color_lower_leg = point4(0.5, 0.5, 0.5, 1);

// ����������Ķ�������
point4 vertices[8] = {
	point4(-0.5, -0.5, 0.5, 1.0),
	point4(-0.5, 0.5, 0.5, 1.0),
	point4(0.5, 0.5, 0.5, 1.0),
	point4(0.5, -0.5, 0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5, 0.5, -0.5, 1.0),
	point4(0.5, 0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

// ������ɫ����
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 1.0, 1.0),  // white
	color4(0.0, 1.0, 1.0, 1.0)   // cyan
};

My_Mesh *my_mesh1, *my_mesh2, *my_mesh3;

//----------------------------------------------------------------------------

class MatrixStack {
	int    _index;
	int    _size;
	mat4*  _matrices;

public:
	MatrixStack(int numMatrices = 100) :_index(0), _size(numMatrices)
	{
		_matrices = new mat4[numMatrices];
	}

	~MatrixStack()
	{
		delete[]_matrices;
	}

	void push(const mat4& m) {
		assert(_index + 1 < _size);
		_matrices[_index++] = m;

	}

	mat4& pop(void) {
		assert(_index - 1 >= 0);
		_index--;
		return _matrices[_index];
	}
};

MatrixStack  mvstack;

mat4 model;							// �����˵�ģ��-��ͼ����
mat4 rotateMatrix;					// ��ת����������ת��ӰͶӰƽ��
mat4 projMatrix;					// ͶӰ����
mat4 shadowProjMatrix;				// ��ӰͶӰ����

GLuint program;
GLuint modelViewMatrixID;
GLuint projMatrixID;
GLuint rotateMatrixID;
GLuint isShadowID;
GLuint lightPosID;
GLuint draw_color;

//----------------------------------------------------------------------------
// �����˸������Ŀ�͸�
#define HAIR_HEIGHT 0.6
#define HAIR_WIDTH 2.2

#define TORSO_HEIGHT 3.8
#define TORSO_WIDTH 3.0

#define UPPER_ARM_HEIGHT 1.8
#define UPPER_ARM_WIDTH  0.9

#define LOWER_ARM_HEIGHT 1.3
#define LOWER_ARM_WIDTH  0.5

#define UPPER_LEG_HEIGHT 2.2
#define UPPER_LEG_WIDTH  1.0

#define LOWER_LEG_HEIGHT 1.5
#define LOWER_LEG_WIDTH  0.5

#define HEAD_HEIGHT 1.5
#define HEAD_WIDTH 1.8

#define NECK_HEIGHT 0.3
#define NECK_WIDTH 0.7

namespace Camera
{
	mat4 modelMatrix(1.0);
	mat4 viewMatrix(1.0);
	mat4 projMatrix(1.0);

	mat4 ortho(const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNearear, const GLfloat zFarar)
	{
		// ����ͶӰ����ļ���
		vec4 r1 = vec4(2 / (right - left), 0.0, 0.0, -(right + left) / (right - left));
		vec4 r2 = vec4(0.0, 2 / (top - bottom), 0.0, -(top + bottom) / (top - bottom));
		vec4 r3 = vec4(0.0, 0.0, -2 / (zFarar - zNearear), -(zFarar + zNearear) / (zFarar - zNearear));
		vec4 r4 = vec4(0.0, 0.0, 0.0, 1.0);
		mat4 view = mat4(r1, r2, r3, r4);
		return view;
	}

	mat4 perspective(const GLfloat fovyy, const GLfloat aspect,
		const GLfloat zNearear, const GLfloat zFarar)
	{
		// ͸��ͶӰ����ļ���
		GLfloat top = zNearear*tan(fovyy * DegreesToRadians / 2);
		GLfloat right = top*aspect;
		vec4 r1 = vec4(zNearear / right, 0.0, 0.0, 0.0);
		vec4 r2 = vec4(0.0, zNearear / top, 0.0, 0.0);
		vec4 r3 = vec4(0.0, 0.0, -(zFarar + zNearear) / (zFarar - zNearear), -(2 * zFarar*zNearear) / (zFarar - zNearear));
		vec4 r4 = vec4(0.0, 0.0, -1.0, 0.0);
		mat4 view = mat4(r1, r2, r3, r4);
		return view;
	}

	mat4 lookAt(const vec4& eye, const vec4& at, const vec4& up)
	{
		// ����۲����ļ���
		vec4 n = normalize(eye - at);
		vec4 u = vec4(normalize(cross(up, n)), 0.0);
		vec4 v = vec4(normalize(cross(n, u)), 0.0);
		vec4 T = vec4(0.0, 0.0, 0.0, 1.0);
		mat4 view = mat4(u, v, n, T) * Translate(-eye);
		return view;
	}
}

// Set up menu item indices, which we can alos use with the joint angles
enum {
	Torso,
	Head1,
	Head2,
	RightUpperArm,
	RightLowerArm,
	LeftUpperArm,
	LeftLowerArm,
	RightUpperLeg,
	RightLowerLeg,
	LeftUpperLeg,
	LeftLowerLeg,
	NumJoinyaws,
	ChangeLightPos,
	Kick,
	Walk,
	Play,
	Reset,
	Quit
};

// Joint angles with initial values
GLfloat theta[NumJoinyaws] = {
	0.0,    // Torso
	0.0,    // Head1
	0.0,    // Head2
	180.0,  // RightUpperArm
	0.0,    // RightLowerArm
	180.0,  // LeftUpperArm
	0.0,    // LeftLowerArm
	180.0,  // RightUpperLeg
	0.0,    // RightLowerLeg
	180.0,  // LeftUpperLeg
	0.0     // LeftLowerLeg
};

GLint angle = Head2;

vec3 lightPos(0.0, 25.0, 20.0);		// ��ʼʱ�Ĺ�Դλ��
float isShadow = 0.0;				// �жϵ�ǰ���Ƶ�����Ӱ��������ı�����Ϊ1˵���ڻ�����Ӱ

int mainWindow;						// ������
int mainMenu;						// ���˵�
int subMenu;						// �Ӳ˵�
bool toChangeLightPos = false;		// �Ƿ�ı��Դλ�õı���

// �������
float radius = 4.0;					// ������������ĵľ���
float pitch = 0.0;					// �����X����ת�ĽǶ�
float yaw = 90.0;					// �����Y����ת�ĽǶ�
const double PI = 3.1415926;

vec4 eye(0.0, 0.0, 4.0, 1.0);
vec4 at(0.0, 0.0, 0.0, 1.0);
vec4 up(0.0, 1.0, 0.0, 0.0);

// ͸��ͶӰ�Ĳ���
float fovy = 45.0;
float aspect = 1.0;
float zNear = 0.1;
float zFar = 100.0;

// ����ͶӰ�Ĳ���
float orthoscale = 10;

float proscale = 1.0;					// ���������ŵĲ���

int startTime;
float ballTranDelta1 = 0.0;				// �������ߣ����ƽ�Ƶ�z����
float ballTranDelta2 = 0.0;				// ��ǽ���ߣ����ƽ�Ƶ�z����
float outBallTranHeight = 0.0;			// �������ʱ���߷ɵ����ƽ�Ƹ߶�
float inBallTranHeight = 0.0;			// �������ʱ�����е����ƽ�Ƹ߶�
int kickCount = 0;						// ��������һ��ʱ���ջؽ�
int walkCount = 0;						// ����				
int sum_walkCount = 0;					// ����	
int playCount = 0;						// ����	
int sum_playCount = 0;					// ����	
int flag = 0;							// ����·���ߵ��������ʱ��ͨ������ż�����ƽ�����̵ı���
float walkStep = 0;						// ����	
float robotTranX = TORSO_WIDTH;			// ��ʼʱ������λ�ã����������ģ����Ի�����Ҫ����ƽ��һ��
bool isKick = false;					// �Ƿ�ѡ���ˡ����򡱵Ĳ���
bool isWalk = false;					// �Ƿ�ѡ���ˡ���·���Ĳ���
bool isPlay = false;					// �Ƿ�ѡ���ˡ����򡱵Ĳ���
int randomNum = 0;						// ������������ж��߳������䲻�����Լ��ǲ��Ǻ���
float playBallHeight = 0.0;

//----------------------------------------------------------------------------

int Index = 0;

void quad(int a, int b, int c, int d)
{
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

void colorcube(void)					//	�������ɵ�λ����������������6��������
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

void init(void)
{
	colorcube();

	for (int i = 0; i < NumVertices; i++)
	{
		normals[i] = points[i] - vec4(0.0, 0.0, 0.0, 1.0);
	}

	// Load shaders and use the resulting shader program
	program = InitShader("vshader82.glsl", "fshader82.glsl");
	glUseProgram(program);

	// ����ɫ���л�ȡ��Ӧ������λ��
	modelViewMatrixID = glGetUniformLocation(program, "modelViewMatrix");
	projMatrixID = glGetUniformLocation(program, "projMatrix");
	rotateMatrixID = glGetUniformLocation(program, "rotateMatrix");
	isShadowID = glGetUniformLocation(program, "isShadow");	
	lightPosID = glGetUniformLocation(program, "lightPos");
	draw_color = glGetUniformLocation(program, "draw_color");

	// OpenGL��Ӧ״̬����
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
//	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glLightModeli(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// ���ô��ڱ�����ɫ
	glClearColor(0.28, 0.28, 0.28, 1.0);
}

//----------------------------------------------------------------------------

void torso()			// ��������
{
	mvstack.push(model);											// ���游�ڵ����
	mat4 instance = (Translate(0.0, 0.5 * TORSO_HEIGHT, 0.0) *
		Scale(TORSO_WIDTH, TORSO_HEIGHT, TORSO_WIDTH));				// ���ڵ�ֲ��任����

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);		//	���ڵ����*���ڵ�ֲ��任����
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_torso);
	
	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);							// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);						// ��������

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);							// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);						// ������Ӱ

	model = mvstack.pop();											// �ָ����ڵ����
}

void neck()						// ���Ʋ���
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * NECK_HEIGHT, 0.0) *
		Scale(NECK_WIDTH, NECK_HEIGHT, NECK_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_neck);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void head()					// ����ͷ��
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * HEAD_HEIGHT, 0.0) *
		Scale(HEAD_WIDTH, HEAD_HEIGHT, HEAD_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_head);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void hair()				// ����ͷ��
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * HAIR_HEIGHT, 0.0) *
		Scale(HAIR_WIDTH, HAIR_HEIGHT, HAIR_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_hair);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void left_upper_arm()			// �������ϱ�
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0) *
		Scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_upper_arm);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void left_lower_arm()				// �������±�
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0) *
		Scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_lower_arm);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void right_upper_arm()				// �������ϱ�
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0) *
		Scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_upper_arm);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void right_lower_arm()				// �������±�
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0) *
		Scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_lower_arm);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void left_upper_leg()				// ����������
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * UPPER_LEG_HEIGHT, 0.0) *
		Scale(UPPER_LEG_WIDTH, UPPER_LEG_HEIGHT, UPPER_LEG_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_upper_leg);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void left_lower_leg()				// ����������
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * LOWER_LEG_HEIGHT, 0.0) *
		Scale(LOWER_LEG_WIDTH, LOWER_LEG_HEIGHT, LOWER_LEG_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_lower_leg);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void right_upper_leg()				// ����������
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * UPPER_LEG_HEIGHT, 0.0) *
		Scale(UPPER_LEG_WIDTH, UPPER_LEG_HEIGHT, UPPER_LEG_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_upper_leg);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void right_lower_leg()				// ����������
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * LOWER_LEG_HEIGHT, 0.0) *
		Scale(LOWER_LEG_WIDTH, LOWER_LEG_HEIGHT, LOWER_LEG_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_lower_leg);

	isShadow = 0.0;										// ��ǰ���Ƶ�������
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;										// ��ǰ���Ƶ�����Ӱ
	glUniform1fv(isShadowID, 1, &isShadow);				// ����ʾ��ǰ���������Ƿ�����Ӱ�ı���������ɫ��
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

//----------------------------------------------------------------------------

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mp_->draw_meshes(radius, yaw, pitch, orthoscale, fovy, aspect, zNear, zFar);

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(colors), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), sizeof(colors), colors);

	glUseProgram(program);
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points) + sizeof(normals)));

	// ��������任�Ĳ���
	float x = radius * cos(pitch * DegreesToRadians) * cos(yaw * DegreesToRadians);
	float z = radius * cos(pitch * DegreesToRadians) * sin(yaw * DegreesToRadians);
	float y = radius * sin(pitch * DegreesToRadians);

	eye = vec4(x, y, z, 1.0);

	// ��������任����
	Camera::viewMatrix = Camera::lookAt(eye, at, up);
//	Camera::projMatrix = Camera::perspective(fovy, aspect, zNear, zFar);

	rotateMatrix = RotateX(-5.0);								// ��ת�Կ�����Ӱ
	glUniformMatrix4fv(rotateMatrixID, 1, GL_TRUE, &rotateMatrix[0][0]);

	// ͶӰ��Ϊy=0��������ӰͶӰ����
	float lx = lightPos.x;
	float ly = lightPos.y;
	float lz = lightPos.z;
	shadowProjMatrix = mat4(-ly, 0.0, 0.0, 0.0,
							lx, 0.0, lz, 1.0,
							0.0, 0.0, -ly, 0.0,
							0.0, 0.0, 0.0, -ly);

	glUniform3fv(lightPosID, 1, &lightPos[0]);			// ����Դλ�ô�����ɫ��

//	model = Scale(0.15,0.15,0.15) * Translate(TORSO_WIDTH, 3.5, 0) * RotateY(theta[Torso]);//���ɱ任����
	model = Scale(proscale, proscale, proscale) * Translate(robotTranX, TORSO_HEIGHT, 0) * RotateY(theta[Torso]);//���ɱ任����
	torso();												//���ɻ���

	mvstack.push(model);
	model *= Translate(0.0, TORSO_HEIGHT, 0.0);
	neck();
//	model = mvstack.pop(); 

//	mvstack.push(model);
	// head1ͷ����ת,��������x��ת��head2ͷ����ת����������y��ת
//	model *= Translate(0.0, TORSO_HEIGHT + NECK_HEIGHT, 0.0) * RotateX(-theta[Head1]) * RotateY(theta[Head2]);
	model *= Translate(0.0, NECK_HEIGHT, 0.0) * RotateX(-theta[Head1]) * RotateY(theta[Head2]);
	head();
	model *= Translate(0.0, HEAD_HEIGHT, 0.0);
	hair();
	model = mvstack.pop();

	mvstack.push(model);
	model *= Translate(-0.5*(TORSO_WIDTH + UPPER_ARM_WIDTH), 0.9*TORSO_HEIGHT, 0.0)*RotateX(theta[LeftUpperArm]);
	left_upper_arm();
	model *= Translate(0.0, UPPER_ARM_HEIGHT, 0.0)*RotateX(theta[LeftLowerArm]);
	left_lower_arm();
	model = mvstack.pop();

	mvstack.push(model);
	model *= Translate(0.5*(TORSO_WIDTH + UPPER_ARM_WIDTH), 0.9*TORSO_HEIGHT, 0.0)*RotateX(theta[RightUpperArm]);
	right_upper_arm();
	model *= Translate(0.0, UPPER_ARM_HEIGHT, 0.0)*RotateX(theta[RightLowerArm]);
	right_lower_arm();
	model = mvstack.pop();

	mvstack.push(model);
	model *= Translate(-0.2*(TORSO_WIDTH + UPPER_LEG_WIDTH), 0.1*UPPER_LEG_HEIGHT, 0.0)*RotateX(theta[LeftUpperLeg]);
	left_upper_leg();
	model *= Translate(0.0, UPPER_LEG_HEIGHT, 0.0)*RotateX(theta[LeftLowerLeg]);
	left_lower_leg();
	model = mvstack.pop();

	mvstack.push(model);
	model *= Translate(0.2*(TORSO_WIDTH + UPPER_LEG_WIDTH), 0.1*UPPER_LEG_HEIGHT, 0.0)*RotateX(theta[RightUpperLeg]);
	right_upper_leg();
	model *= Translate(0.0, UPPER_LEG_HEIGHT, 0.0)*RotateX(theta[RightLowerLeg]);
	right_lower_leg();
	model = mvstack.pop();

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (toChangeLightPos)
		{
			lightPos.x = (x - 450) / 450.0;				// ��ȡ���xλ�÷��������ʵ��ı任
			lightPos.y = (450 - y) / 450.0;				// ��ȡ���yλ�÷��������ʵ��ı任

			// ��ֹʵ�ʹ�Դλ��̫���������壬����ĳЩ�������Ӱ��ʾ�������⣬�ʵ���Զ��Դ
			if (lightPos.x < 0)
			{
				lightPos.x -= 10;
			}
			else
			{
				lightPos.x += 10;
			}
			if (lightPos.y < 0)
			{
				lightPos.y = -lightPos.y + 15;			// ��������������·�����Դ��yӦ������ֵ
			}
			else
			{
				lightPos.y += 15;
			}
			toChangeLightPos = false;
		}
		else 
		{
			theta[angle] -= 5.0;
			if (theta[angle] < 0.0)
			{
				theta[angle] += 360.0;
			}
		//	std::cout << angle << " " << theta[angle] << std::endl;
		}
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		if (!toChangeLightPos)
		{
			theta[angle] += 5.0;
			if (theta[angle] > 360.0)
			{
				theta[angle] -= 360.0;
			}
		//	std::cout << angle << " " << theta[angle] << std::endl;
		}
	}
	glutPostRedisplay();
}

void idle()					// ���лص�������������ʵ�����ж�������
{
	if (isKick && !isWalk && !isPlay)							// ����ִ�С����򡱵Ĳ���
	{
		int currentTime = glutGet(GLUT_ELAPSED_TIME);			// �ٴλ�ȡ����ʱ�䣨��һ�λ�ȡ���ڳ����ʼ��ʱ��
		if (currentTime - startTime >= 20)						// �����λ�ȡ����ʱ��������жϣ�20ms����ʱ���
		{
			kickCount++;
			if (kickCount == 12)
			{
				if (yaw >= 90 && yaw <= 260 && theta[LeftUpperLeg] > 180)		// ���泯��������̧��ʱ
				{
					theta[LeftUpperLeg] = 180;					// �ջ���
				}
				else if ((yaw < 90 || yaw > 260) && theta[LeftUpperLeg] < 180)		// ���汳����������̧��ʱ
				{
					theta[LeftUpperLeg] = 180;					// �ջ���
				}
			}
			if (yaw >= 90 && yaw <= 260)			// ��������
			{
				ballTranDelta1 -= 0.1;
				if (randomNum > 6)					// ���������6�����߳�����
				{
					outBallTranHeight += 0.07;		// ÿ�������ĸ߶�����
				}
				if (ballTranDelta1 >= -10.0)		// �����������ƶ�
				{
					if (randomNum < 3)				// �����С��3����һ�������򣨺���
					{
						if (ballTranDelta1 >= -5.0)			// ���ƶ������ŵ�ǰ���·��
						{
							inBallTranHeight += 0.02;		// ��������ƽ�Ƶ����С��5ʱ��inBallTranHeight�����������ڱ�������ƽ��ʱ���������ƽ��
						}
						else								// ���ƶ������ŵĺ���·��
						{
							inBallTranHeight -= 0.02;		// ��������ƽ�Ƶ���ȴ���5ʱ��inBallTranHeight�ݼ��������ڱ�������ƽ��ʱ���������ƽ��
						}
					}
					my_mesh2->set_translate(-0.3, -0.3 + outBallTranHeight + inBallTranHeight, ballTranDelta1);			// �������ƽ��
					startTime = currentTime;							// ���¿�ʼʱ��Ϊ��ǰʱ�䣬�����´ε��ÿ��лص�����ʱ����
				}
				else								// �ƶ�����ȵľ���ֵ�ﵽ10��
				{
					kickCount = 0;
					my_mesh2->set_theta_step(0, 0, 0);			// ��ֹͣ��ת
					if (randomNum > 6)				// ���ղŵ����������6��˵��������
					{
						SoundEngine->play2D("audio/xusheng.wav", GL_FALSE);			// ���Ź��ںȵ��ʵ�����
					}
					else							// ���ղŵ������С�ڵ���6��˵����������
					{
						SoundEngine->play2D("audio/huanhu.wav", GL_FALSE);			// ���Ź��ڹ��ƻ���������
					}
					isKick = false;
					glutIdleFunc(NULL); 					// ֹͣ���ÿ��лص�����
				}
			}
			else if (yaw < 90 || yaw > 260)			// �������ŷ���
			{
				ballTranDelta2 += 0.1;
				if (randomNum > 6)
				{
					outBallTranHeight += 0.07;
				}
				if (ballTranDelta2 <= 10.0)
				{
					my_mesh2->set_translate(-0.3, -0.3 + outBallTranHeight, ballTranDelta2);
					startTime = currentTime;							// ���¿�ʼʱ��Ϊ��ǰʱ�䣬�����´ε��ÿ��лص�����ʱ����
				}
				else
				{
					kickCount = 0;
					my_mesh2->set_theta_step(0, 0, 0);						// ��ֹͣ
					SoundEngine->play2D("audio/xusheng.wav", GL_FALSE);		// ��Ϊ�������ŷ��棬���������������У����Բ�������
					isKick = false;
					glutIdleFunc(NULL);						// ֹͣ���ÿ��лص�����
				}
			}
		}
	}
	else if (!isKick && isWalk && !isPlay)					// ����ִ�С���·���Ĳ���
	{
		int currentTime = glutGet(GLUT_ELAPSED_TIME);			// �ٴλ�ȡ����ʱ�䣨��һ�λ�ȡ���ڳ����ʼ��ʱ��
		if (currentTime - startTime >= 20)					// �����λ�ȡ����ʱ��������жϣ�20ms����ʱ���
		{
			walkCount++;
			sum_walkCount++;
			if (sum_walkCount < 160 && walkCount % 20 == 0 && flag % 2 == 0)		// flagÿ���Լӣ�����ż�����ƽ������
			{
				// �ջ��ֱۺ���
				theta[RightUpperArm] = 180.0;
				theta[RightLowerArm] = 0.0;
				theta[LeftUpperArm] = 180.0;
				theta[LeftLowerArm] = 0.0;
				theta[RightUpperLeg] = 180.0;
				theta[RightLowerLeg] = 0.0;
				theta[LeftUpperLeg] = 180.0;
				theta[LeftLowerLeg] = 0.0;
				if (theta[Torso] >= 45 && theta[Torso] <= 135)			// ������
				{
					robotTranX -= 0.6*TORSO_WIDTH;
				}
				else if (theta[Torso] >= 225 && theta[Torso] <= 315)	// ������
				{
					robotTranX += 0.6*TORSO_WIDTH;
				}
				startTime = currentTime;							// ���¿�ʼʱ��Ϊ��ǰʱ�䣬�����´ε��ÿ��лص�����ʱ����
				walkCount = 0;
				flag++;
			//	std::cout << flag << std::endl;
			}
			else if (sum_walkCount < 160 && walkCount % 20 == 0 && flag % 2 == 1)
			{
				// �ڱ� + �粽
				theta[RightUpperArm] = 135.0;
				theta[RightLowerArm] = 15.0;
				theta[LeftUpperArm] = 225.0;
				theta[LeftLowerArm] = 15.0;
				theta[RightUpperLeg] = 220.0;
				theta[RightLowerLeg] = 350.0;
				theta[LeftUpperLeg] = 150.0;
				theta[LeftLowerLeg] = 345.0;
				walkCount = 0;
				flag++;
				startTime = currentTime;							// ���¿�ʼʱ��Ϊ��ǰʱ�䣬�����´ε��ÿ��лص�����ʱ����
			//	std::cout << flag << std::endl;
			}
			else if (sum_walkCount >= 160)
			{
				sum_walkCount = 0;
				walkCount = 0;
				isWalk = false;
				flag = 0;
				glutIdleFunc(NULL);		 			// ֹͣ���ÿ��лص�����
			}
		}
	}
	else if (!isKick && !isWalk && isPlay)					// ����ִ�С����򡱵Ĳ���
	{
		int currentTime = glutGet(GLUT_ELAPSED_TIME);			// �ٴλ�ȡ����ʱ�䣨��һ�λ�ȡ���ڳ����ʼ��ʱ��
		if (currentTime - startTime >= 20)					// �����λ�ȡ����ʱ��������жϣ�20ms����ʱ���
		{
			playCount++;
			sum_playCount++;
			if (sum_playCount<160 && playCount % 20 == 0 && flag % 2 == 0)		// flagÿ���Լӣ�����ż�����ƽ������
			{
				if (theta[LeftUpperLeg] > 180)
				{
					playBallHeight -= 0.6;					// �����
					theta[LeftUpperLeg] = 180;				// �ջ���
					my_mesh2->set_translate(-0.3, -0.3 + playBallHeight, ballTranDelta2);
				}
				playCount = 0;
				flag++;
				startTime = currentTime;							// ���¿�ʼʱ��Ϊ��ǰʱ�䣬�����´ε��ÿ��лص�����ʱ����
			}
			else if (sum_playCount<160 && playCount % 20 == 0 && flag % 2 == 1)
			{
				if (theta[LeftUpperLeg] <= 180)
				{
					playBallHeight += 0.6;					// ������
					theta[LeftUpperLeg] = 225;				// ����
					my_mesh2->set_translate(-0.3, -0.3 + playBallHeight, ballTranDelta2);
				}
				playCount = 0;
				flag++;
				startTime = currentTime;							// ���¿�ʼʱ��Ϊ��ǰʱ�䣬�����´ε��ÿ��лص�����ʱ����
			}
			else if (sum_playCount >= 160)
			{
				flag = 0;
				playCount = 0;
				sum_playCount = 0;
				playBallHeight = 0;
				my_mesh2->set_translate(-0.3, -0.3, 0);		// �����ԭ��
				my_mesh2->set_theta_step(0, 0, 0);			// ��ֹͣ
				isPlay = false;
				glutIdleFunc(NULL); 					// ֹͣ���ÿ��лص�����
			}
		}
	}
	glutPostWindowRedisplay(mainWindow);
}

void reset()				// �������б�����λ��
{
	radius = 4.0; yaw = 90.0; pitch = 0.0;
	fovy = 45.0; aspect = 1.0; zNear = 0.1; zFar = 100.0;
	my_mesh2->set_translate(-0.3, -0.3, 0.0);
	my_mesh2->set_theta(0.0, 0.0, 90.0);
	my_mesh2->set_theta_step(0, 0, 0);
	isKick = false;
	isWalk = false;
	isPlay = false;
	toChangeLightPos = false;
	robotTranX = TORSO_WIDTH;
	ballTranDelta1 = 0.0;
	ballTranDelta2 = 0.0;
	outBallTranHeight = 0.0;
	inBallTranHeight = 0.0;
	playBallHeight = 0.0;

	theta[Torso] = 0.0;
	theta[Head1] = 0.0;
	theta[Head2] = 0.0;
	theta[RightUpperArm] = 180.0;
	theta[RightLowerArm] = 0.0;
	theta[LeftUpperArm] = 180.0;
	theta[LeftLowerArm] = 0.0;
	theta[RightUpperLeg] = 180.0;
	theta[RightLowerLeg] = 0.0;
	theta[LeftUpperLeg] = 180.0;
	theta[LeftLowerLeg] = 0.0;
}

void kick()
{
	srand((unsigned)time(NULL));
	randomNum = rand() % 10;						// ����[0,10)�������
//	std::cout << randomNum << std::endl;
	isKick = true;
	if (yaw >= 90 && yaw <= 260)					// �ӽǳ�������
	{
		theta[LeftUpperLeg] = 245;					// ������������̧��ĽǶ�
	}
	else											// �ӽǱ�������
	{
		theta[LeftUpperLeg] = 120;					// �������෴�ķ���������̧��ĽǶ�
	}
	my_mesh2->set_theta(0.0, 0.0, 90.0);
	my_mesh2->set_theta_step(0, 10, 0);				// ����������ת�ٶ�
	startTime = glutGet(GLUT_ELAPSED_TIME);			// ��ȡ��ʼʱ��
	ballTranDelta1 = 0.0;
	ballTranDelta2 = 0.0;
	SoundEngine->play2D("audio/kickOrWalk.wav", GL_FALSE);		// �����������Ч
	glutIdleFunc(idle);								// ���ÿ��лص�����
}

void walk()
{
	isWalk = true;
	// ��������·ʱ�������ĽǶ�
	theta[RightUpperArm] = 135.0;
	theta[RightLowerArm] = 15.0;
	theta[LeftUpperArm] = 225.0;
	theta[LeftLowerArm] = 15.0;
	theta[RightUpperLeg] = 220.0;
	theta[RightLowerLeg] = 350.0;
	theta[LeftUpperLeg] = 150.0;
	theta[LeftLowerLeg] = 345.0;
	startTime = glutGet(GLUT_ELAPSED_TIME);			// ��ȡ��ʼʱ��
	glutIdleFunc(idle);								// ���ÿ��лص�����
}

void play()
{
	isPlay = true;
	playBallHeight += 0.6;							// ������
	theta[LeftUpperLeg] = 225;						// ͬʱ�����������ȵĶ���
	my_mesh2->set_translate(-0.3, -0.3 + playBallHeight, ballTranDelta2);
	my_mesh2->set_theta_step(0, 10, 0);				// ����������ת�ٶ�
	startTime = glutGet(GLUT_ELAPSED_TIME);			// ��ȡ��ʼʱ��
	glutIdleFunc(idle); 							// ���ÿ��лص�����
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033:								// ����ESC����q��Q���˳�����
	case 'q': 
	case 'Q': exit(EXIT_SUCCESS); break;
	case 'x':								// ����x��X����ʹ�����X��������ת
		pitch += 5.0;
		if (pitch > 85.0)
			pitch = 85.0;
		break;
	case 'X': 
		pitch -= 5.0;
		if (pitch < -5.0)
			pitch = -5.0;
		break;
	case 'y':								// ����y��Y����ʹ�����Y��������ת
		yaw += 5.0; 
		if (yaw == 360.0)
			yaw = 0.0;
		break;
	case 'Y': 
		yaw -= 5.0; 
		if (yaw == 0.0)
			yaw = 360.0;
		break;
	case 'z':								// ����z��Z����ʹ�����������Զ
		radius += 0.1;  
		break;
	case 'Z': 
		radius -= 0.1; 
		if (radius < 0.0)
			radius += 0.1;
		break;
	case 'f':								// ����f��F�����ı�͸��ͶӰ����fovy
		fovy += 5.0; 
		proscale -= 0.05;
		if (proscale < 0)
			proscale = 0.1;
		if (fovy >= 160.0)
			fovy = 160.0;
		break;
	case 'F': fovy -= 5.0; 
		proscale += 0.05;
		if (proscale > 1)
			proscale = 1.0;
		if (fovy <= 10.0)
			fovy = 10.0;
		break;
	case 'a': 								// ����a��A�����ı�͸��ͶӰ����aspect
		aspect += 0.1; 
		break;
	case 'A': 
		aspect -= 0.1; 
		break;
	case 'n': 								// ����n��N�����ı�͸��ͶӰ����zNear
		zNear += 0.1; 
		break;
	case 'N': 
		zNear -= 0.1;
		if (zNear == 0)
		{
			zNear = 0.1;
		}
		break;
	case 'm': 								// ����m��M�����ı�͸��ͶӰ����zFar
		zFar += 0.1; 
		break;
	case 'M': 
		zFar -= 0.1; 
		break;
	case 'o': 								// ����o��O����ʹ�����������Զ
		orthoscale += 0.1; 
		break;
	case 'O': 
		orthoscale -= 0.1; 
		break;
	case ' ': 
		reset();
		break;
	case 'k':			// ���¼��̡�k������ִ�л���������Ķ���
		kick();
		break;
	case 'w':			// ���¼��̡�w������ִ�л�������·�Ķ���
		walk();
		break;
	case 'p':			// ���¼��̡�p������ִ�л����˵���Ķ���
		play();
		break;
	default: 
		break;
	}
	glutPostWindowRedisplay(mainWindow);
}

//----------------------------------------------------------------------------

void menuEvent(int option)
{
	if (option == Quit) {
		exit(EXIT_SUCCESS);
	}
	else if (option == ChangeLightPos)		// �ı��Դ
	{
		toChangeLightPos = true;
	}
	else if (option == Kick)				// �����ˡ�����
	{
		kick();
	}
	else if (option == Walk)				// �����ˡ���·��
	{
		walk();
	}
	else if (option == Play)				// �����ˡ�����
	{
		play();
	}
	else if (option == Reset)				// �������б�����λ��
	{
		reset();
	}
	else
	{	
		angle = option;						// ѡ�л����˵�ĳһ����
	}
}

void setupMenu()
{	
	subMenu = glutCreateMenu(menuEvent);		// �����Ӳ˵�
	// �����˸�����
	glutAddMenuEntry("torso", Torso);
	glutAddMenuEntry("head1", Head1);
	glutAddMenuEntry("head2", Head2);
	glutAddMenuEntry("right_upper_arm", RightUpperArm);
	glutAddMenuEntry("right_lower_arm", RightLowerArm);
	glutAddMenuEntry("left_upper_arm", LeftUpperArm);
	glutAddMenuEntry("left_lower_arm", LeftLowerArm);
	glutAddMenuEntry("right_upper_leg", RightUpperLeg);
	glutAddMenuEntry("right_lower_leg", RightLowerLeg);
	glutAddMenuEntry("left_upper_leg", LeftUpperLeg);
	glutAddMenuEntry("left_lower_leg", LeftLowerLeg);

	mainMenu = glutCreateMenu(menuEvent);					// �������˵�
	glutAddSubMenu("action", subMenu);			// �����˵��м����Ӳ˵�
	glutAddMenuEntry("changeLightPos", ChangeLightPos);		// �ı��Դλ��
	glutAddMenuEntry("kick", Kick);				// ����������
	glutAddMenuEntry("walk", Walk);				// ��������·
	glutAddMenuEntry("play", Play);				// �����˵���
	glutAddMenuEntry("reset", Reset);			// �������б�����λ��
	glutAddMenuEntry("quit", Quit);				// �˳�����
	glutAttachMenu(GLUT_MIDDLE_BUTTON);			// ������Ҽ�������˵�
}

void generateScene()
{
	mp_ = new Mesh_Painter;

	my_mesh1 = new My_Mesh;
	int box_size = 1;
	my_mesh1->load_obj("texture/box.obj", box_size);			// ���볤���壬��ʾһ������
	my_mesh1->set_texture_file("texture/box2.png");
	my_mesh1->set_translate(0.0, -0.6, 0.0);
	my_mesh1->set_theta(0.0, 90.0, 0.0);
	my_mesh1->set_theta_step(0, 0, 0);
	my_meshs.push_back(my_mesh1);
	mp_->add_mesh(my_mesh1);

	my_mesh2 = new My_Mesh;
	my_mesh2->load_obj("texture/football.obj", box_size);		// ��������
	my_mesh2->set_texture_file("texture/football1.png");
	my_mesh2->set_translate(-0.3, -0.3, 0.0);
	my_mesh2->set_theta(0.0, 0.0, 90.0);
	my_mesh2->set_theta_step(0, 0, 0);
	my_meshs.push_back(my_mesh2);
	mp_->add_mesh(my_mesh2);

	my_mesh3 = new My_Mesh;
	my_mesh3->load_obj("texture/gate.obj", box_size);			// ��������
	my_mesh3->set_texture_file("texture/gate1.png");
	my_mesh3->set_translate(0.0, -0.6, 0.0);
	my_mesh3->set_theta(0.0, 90.0, 0.0);
	my_mesh3->set_theta_step(0, 0, 0);
	my_meshs.push_back(my_mesh3);
	mp_->add_mesh(my_mesh3);

	mp_->init_shaders("v_texture.glsl", "f_texture.glsl");
	mp_->update_vertex_buffer();
	mp_->update_texture();
}

//----------------------------------------------------------------------------

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	GLfloat left = -10.0, right = 10.0;
	GLfloat bottom = -5.0, top = 15.0;
	GLfloat zNearear = -50.0, zFarar = 50.0;

	GLfloat aspect = GLfloat(width) / height;

	if (aspect > 1.0) {
		left *= aspect;
		right *= aspect;
	}
	else {
		bottom /= aspect;
		top /= aspect;
	}

	mat4 projection = Camera::ortho(left, right, bottom, top, zNearear, zFarar);
	Camera::projMatrix = projection;
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);

	model = mat4(1.0);   // An Identity matrix
	
	reset();			// �������б�����λ��
}


// ���������Ϣ
void printHelp() {
	printf("\n%s\n", "2014150270-�����-��ĩ����ҵ");
	printf("Keyboard options:\n");
	printf("x/X: �����X����ת\n");
	printf("y/Y: �����Y����ת\n");
	printf("z/Z: ���������Զ\n");
	printf("f/F: �ı�͸��ͶӰfovy\n");
	printf("a/A: �ı�͸��ͶӰaspect\n");
	printf("k: ����������\n");
	printf("w: ��������·\n");
	printf("p: �����˵���\n");
	printf(" : �ո�����\n");
	printf("q/Q/Esc: �˳�\n");
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(900, 900);				// ��ʼ�����ڴ�С
	glutInitWindowPosition(150, 50);			// ��ʼ������λ��
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	mainWindow = glutCreateWindow("2014150270-�����-��ĩ����ҵ");		// ��������
	glewExperimental = GL_TRUE;
	glewInit();

	generateScene();			// ���ɳ���

	init();

	setupMenu();				// �����Ҽ��˵�

	// ���������Ϣ
	printHelp();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	glutMainLoop();

	for (unsigned int i = 0; i < my_meshs.size(); i++)
	{
		delete my_meshs[i];
	}
	delete mp_;

	return 0;
}
