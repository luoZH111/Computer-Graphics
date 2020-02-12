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

point4 points[NumVertices];				// 顶点数组
color4 colors[NumVertices];				// 颜色数组
point4 normals[NumVertices];			// 法向量数组

// 机器人各部件的颜色
point4 color_hair = point4(0.5, 0.5, 0.5, 1);
point4 color_head = point4(0, 1, 0, 1);
point4 color_neck = point4(0.5, 0.5, 0.5, 1);
point4 color_torso = point4(0, 0, 1, 1);
point4 color_upper_arm = point4(1, 1, 0, 1);
point4 color_lower_arm = point4(1, 0, 0, 1);
point4 color_upper_leg = point4(0, 1, 1, 1);
point4 color_lower_leg = point4(0.5, 0.5, 0.5, 1);

// 单个正方体的顶点坐标
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

// 各种颜色向量
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

mat4 model;							// 机器人的模型-视图矩阵
mat4 rotateMatrix;					// 旋转矩阵，用于旋转阴影投影平面
mat4 projMatrix;					// 投影矩阵
mat4 shadowProjMatrix;				// 阴影投影矩阵

GLuint program;
GLuint modelViewMatrixID;
GLuint projMatrixID;
GLuint rotateMatrixID;
GLuint isShadowID;
GLuint lightPosID;
GLuint draw_color;

//----------------------------------------------------------------------------
// 机器人各部件的宽和高
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
		// 正交投影矩阵的计算
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
		// 透视投影矩阵的计算
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
		// 相机观察矩阵的计算
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

vec3 lightPos(0.0, 25.0, 20.0);		// 初始时的光源位置
float isShadow = 0.0;				// 判断当前绘制的是阴影还是物体的变量，为1说明在绘制阴影

int mainWindow;						// 主窗口
int mainMenu;						// 主菜单
int subMenu;						// 子菜单
bool toChangeLightPos = false;		// 是否改变光源位置的变量

// 相机参数
float radius = 4.0;					// 相机到物体中心的距离
float pitch = 0.0;					// 相机绕X轴旋转的角度
float yaw = 90.0;					// 相机绕Y轴旋转的角度
const double PI = 3.1415926;

vec4 eye(0.0, 0.0, 4.0, 1.0);
vec4 at(0.0, 0.0, 0.0, 1.0);
vec4 up(0.0, 1.0, 0.0, 0.0);

// 透视投影的参数
float fovy = 45.0;
float aspect = 1.0;
float zNear = 0.1;
float zFar = 100.0;

// 正交投影的参数
float orthoscale = 10;

float proscale = 1.0;					// 机器人缩放的参数

int startTime;
float ballTranDelta1 = 0.0;				// 朝球门踢，球的平移的z分量
float ballTranDelta2 = 0.0;				// 朝墙壁踢，球的平移的z分量
float outBallTranHeight = 0.0;			// 踢球操作时，踢飞的球的平移高度
float inBallTranHeight = 0.0;			// 踢球操作时，踢中的球的平移高度
int kickCount = 0;						// 计数，隔一定时间收回脚
int walkCount = 0;						// 计数				
int sum_walkCount = 0;					// 计数	
int playCount = 0;						// 计数	
int sum_playCount = 0;					// 计数	
int flag = 0;							// 在走路或者颠球操作的时候，通过其奇偶来控制交替过程的变量
float walkStep = 0;						// 计数	
float robotTranX = TORSO_WIDTH;			// 初始时机器人位置，足球在中心，所以机器人要往右平移一点
bool isKick = false;					// 是否选中了“踢球”的操作
bool isWalk = false;					// 是否选中了“走路”的操作
bool isPlay = false;					// 是否选中了“颠球”的操作
int randomNum = 0;						// 随机数，用于判定踢出的球射不射中以及是不是好球
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

void colorcube(void)					//	这里生成单位立方体的六个表面和6个法向量
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

	// 从着色器中获取相应变量的位置
	modelViewMatrixID = glGetUniformLocation(program, "modelViewMatrix");
	projMatrixID = glGetUniformLocation(program, "projMatrix");
	rotateMatrixID = glGetUniformLocation(program, "rotateMatrix");
	isShadowID = glGetUniformLocation(program, "isShadow");	
	lightPosID = glGetUniformLocation(program, "lightPos");
	draw_color = glGetUniformLocation(program, "draw_color");

	// OpenGL相应状态设置
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
//	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glLightModeli(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 设置窗口背景颜色
	glClearColor(0.28, 0.28, 0.28, 1.0);
}

//----------------------------------------------------------------------------

void torso()			// 绘制躯干
{
	mvstack.push(model);											// 保存父节点矩阵
	mat4 instance = (Translate(0.0, 0.5 * TORSO_HEIGHT, 0.0) *
		Scale(TORSO_WIDTH, TORSO_HEIGHT, TORSO_WIDTH));				// 本节点局部变换矩阵

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);		//	父节点矩阵*本节点局部变换矩阵
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_torso);
	
	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);							// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);						// 绘制躯干

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);							// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);						// 绘制阴影

	model = mvstack.pop();											// 恢复父节点矩阵
}

void neck()						// 绘制脖子
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * NECK_HEIGHT, 0.0) *
		Scale(NECK_WIDTH, NECK_HEIGHT, NECK_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_neck);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void head()					// 绘制头部
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * HEAD_HEIGHT, 0.0) *
		Scale(HEAD_WIDTH, HEAD_HEIGHT, HEAD_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_head);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void hair()				// 绘制头发
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * HAIR_HEIGHT, 0.0) *
		Scale(HAIR_WIDTH, HAIR_HEIGHT, HAIR_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_hair);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void left_upper_arm()			// 绘制左上臂
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0) *
		Scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_upper_arm);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void left_lower_arm()				// 绘制左下臂
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0) *
		Scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_lower_arm);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void right_upper_arm()				// 绘制右上臂
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0) *
		Scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_upper_arm);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void right_lower_arm()				// 绘制右下臂
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0) *
		Scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_lower_arm);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void left_upper_leg()				// 绘制左上腿
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * UPPER_LEG_HEIGHT, 0.0) *
		Scale(UPPER_LEG_WIDTH, UPPER_LEG_HEIGHT, UPPER_LEG_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_upper_leg);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void left_lower_leg()				// 绘制左下腿
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * LOWER_LEG_HEIGHT, 0.0) *
		Scale(LOWER_LEG_WIDTH, LOWER_LEG_HEIGHT, LOWER_LEG_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_lower_leg);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void right_upper_leg()				// 绘制右上腿
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * UPPER_LEG_HEIGHT, 0.0) *
		Scale(UPPER_LEG_WIDTH, UPPER_LEG_HEIGHT, UPPER_LEG_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_upper_leg);

	isShadow = 0.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * shadowProjMatrix * model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model = mvstack.pop();
}

void right_lower_leg()				// 绘制右下腿
{
	mvstack.push(model);

	mat4 instance = Translate(0.0, 0.5 * LOWER_LEG_HEIGHT, 0.0) *
		Scale(LOWER_LEG_WIDTH, LOWER_LEG_HEIGHT, LOWER_LEG_WIDTH);

	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, Camera::viewMatrix * model * instance);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, Camera::projMatrix);
	glUniform4fv(draw_color, 1, color_lower_leg);

	isShadow = 0.0;										// 当前绘制的是物体
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	isShadow = 1.0;										// 当前绘制的是阴影
	glUniform1fv(isShadowID, 1, &isShadow);				// 将表示当前绘制物体是否是阴影的变量传入着色器
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

	// 计算相机变换的参数
	float x = radius * cos(pitch * DegreesToRadians) * cos(yaw * DegreesToRadians);
	float z = radius * cos(pitch * DegreesToRadians) * sin(yaw * DegreesToRadians);
	float y = radius * sin(pitch * DegreesToRadians);

	eye = vec4(x, y, z, 1.0);

	// 计算相机变换矩阵
	Camera::viewMatrix = Camera::lookAt(eye, at, up);
//	Camera::projMatrix = Camera::perspective(fovy, aspect, zNear, zFar);

	rotateMatrix = RotateX(-5.0);								// 旋转以看到阴影
	glUniformMatrix4fv(rotateMatrixID, 1, GL_TRUE, &rotateMatrix[0][0]);

	// 投影面为y=0，计算阴影投影矩阵
	float lx = lightPos.x;
	float ly = lightPos.y;
	float lz = lightPos.z;
	shadowProjMatrix = mat4(-ly, 0.0, 0.0, 0.0,
							lx, 0.0, lz, 1.0,
							0.0, 0.0, -ly, 0.0,
							0.0, 0.0, 0.0, -ly);

	glUniform3fv(lightPosID, 1, &lightPos[0]);			// 将光源位置传给着色器

//	model = Scale(0.15,0.15,0.15) * Translate(TORSO_WIDTH, 3.5, 0) * RotateY(theta[Torso]);//躯干变换矩阵
	model = Scale(proscale, proscale, proscale) * Translate(robotTranX, TORSO_HEIGHT, 0) * RotateY(theta[Torso]);//躯干变换矩阵
	torso();												//躯干绘制

	mvstack.push(model);
	model *= Translate(0.0, TORSO_HEIGHT, 0.0);
	neck();
//	model = mvstack.pop(); 

//	mvstack.push(model);
	// head1头上下转,所以是绕x轴转；head2头左右转，所以是绕y轴转
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
			lightPos.x = (x - 450) / 450.0;				// 获取鼠标x位置分量并做适当的变换
			lightPos.y = (450 - y) / 450.0;				// 获取鼠标y位置分量并做适当的变换

			// 防止实际光源位置太过靠近物体，导致某些方向的阴影显示出现问题，适当拉远光源
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
				lightPos.y = -lightPos.y + 15;			// 点击窗口中物体下方，光源的y应保持正值
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

void idle()					// 空闲回调函数，在其中实现所有动画操作
{
	if (isKick && !isWalk && !isPlay)							// 若是执行“踢球”的操作
	{
		int currentTime = glutGet(GLUT_ELAPSED_TIME);			// 再次获取程序时间（第一次获取是在程序初始化时）
		if (currentTime - startTime >= 20)						// 将两次获取到的时间作差，以判断（20ms）的时间差
		{
			kickCount++;
			if (kickCount == 12)
			{
				if (yaw >= 90 && yaw <= 260 && theta[LeftUpperLeg] > 180)		// 若面朝球门且是抬腿时
				{
					theta[LeftUpperLeg] = 180;					// 收回腿
				}
				else if ((yaw < 90 || yaw > 260) && theta[LeftUpperLeg] < 180)		// 若面背对球门且是抬腿时
				{
					theta[LeftUpperLeg] = 180;					// 收回腿
				}
			}
			if (yaw >= 90 && yaw <= 260)			// 踢向球门
			{
				ballTranDelta1 -= 0.1;
				if (randomNum > 6)					// 随机数大于6，会踢出界外
				{
					outBallTranHeight += 0.07;		// 每次上升的高度增量
				}
				if (ballTranDelta1 >= -10.0)		// 若球向球门移动
				{
					if (randomNum < 3)				// 随机数小于3，是一个弧线球（好球）
					{
						if (ballTranDelta1 >= -5.0)			// 在移动向球门的前半段路程
						{
							inBallTranHeight += 0.02;		// 在往球门平移的深度小于5时，inBallTranHeight递增，足球在边向球门平移时还会边向上平移
						}
						else								// 在移动向球门的后半段路程
						{
							inBallTranHeight -= 0.02;		// 在往球门平移的深度大于5时，inBallTranHeight递减，足球在边向球门平移时还会边向下平移
						}
					}
					my_mesh2->set_translate(-0.3, -0.3 + outBallTranHeight + inBallTranHeight, ballTranDelta1);			// 设置球的平移
					startTime = currentTime;							// 更新开始时间为当前时间，便于下次调用空闲回调函数时作差
				}
				else								// 移动的深度的绝对值达到10后
				{
					kickCount = 0;
					my_mesh2->set_theta_step(0, 0, 0);			// 球停止旋转
					if (randomNum > 6)				// 若刚才的随机数大于6，说明该球不中
					{
						SoundEngine->play2D("audio/xusheng.wav", GL_FALSE);			// 播放观众喝倒彩的嘘声
					}
					else							// 若刚才的随机数小于等于6，说明该球命中
					{
						SoundEngine->play2D("audio/huanhu.wav", GL_FALSE);			// 播放观众鼓掌欢呼的声音
					}
					isKick = false;
					glutIdleFunc(NULL); 					// 停止调用空闲回调函数
				}
			}
			else if (yaw < 90 || yaw > 260)			// 踢向球门反面
			{
				ballTranDelta2 += 0.1;
				if (randomNum > 6)
				{
					outBallTranHeight += 0.07;
				}
				if (ballTranDelta2 <= 10.0)
				{
					my_mesh2->set_translate(-0.3, -0.3 + outBallTranHeight, ballTranDelta2);
					startTime = currentTime;							// 更新开始时间为当前时间，便于下次调用空闲回调函数时作差
				}
				else
				{
					kickCount = 0;
					my_mesh2->set_theta_step(0, 0, 0);						// 球停止
					SoundEngine->play2D("audio/xusheng.wav", GL_FALSE);		// 因为射向球门反面，怎样都不可能命中，所以播放嘘声
					isKick = false;
					glutIdleFunc(NULL);						// 停止调用空闲回调函数
				}
			}
		}
	}
	else if (!isKick && isWalk && !isPlay)					// 若是执行“走路”的操作
	{
		int currentTime = glutGet(GLUT_ELAPSED_TIME);			// 再次获取程序时间（第一次获取是在程序初始化时）
		if (currentTime - startTime >= 20)					// 将两次获取到的时间作差，以判断（20ms）的时间差
		{
			walkCount++;
			sum_walkCount++;
			if (sum_walkCount < 160 && walkCount % 20 == 0 && flag % 2 == 0)		// flag每次自加，奇数偶数控制交替过程
			{
				// 收回手臂和腿
				theta[RightUpperArm] = 180.0;
				theta[RightLowerArm] = 0.0;
				theta[LeftUpperArm] = 180.0;
				theta[LeftLowerArm] = 0.0;
				theta[RightUpperLeg] = 180.0;
				theta[RightLowerLeg] = 0.0;
				theta[LeftUpperLeg] = 180.0;
				theta[LeftLowerLeg] = 0.0;
				if (theta[Torso] >= 45 && theta[Torso] <= 135)			// 向左走
				{
					robotTranX -= 0.6*TORSO_WIDTH;
				}
				else if (theta[Torso] >= 225 && theta[Torso] <= 315)	// 向右走
				{
					robotTranX += 0.6*TORSO_WIDTH;
				}
				startTime = currentTime;							// 更新开始时间为当前时间，便于下次调用空闲回调函数时作差
				walkCount = 0;
				flag++;
			//	std::cout << flag << std::endl;
			}
			else if (sum_walkCount < 160 && walkCount % 20 == 0 && flag % 2 == 1)
			{
				// 摆臂 + 跨步
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
				startTime = currentTime;							// 更新开始时间为当前时间，便于下次调用空闲回调函数时作差
			//	std::cout << flag << std::endl;
			}
			else if (sum_walkCount >= 160)
			{
				sum_walkCount = 0;
				walkCount = 0;
				isWalk = false;
				flag = 0;
				glutIdleFunc(NULL);		 			// 停止调用空闲回调函数
			}
		}
	}
	else if (!isKick && !isWalk && isPlay)					// 若是执行“颠球”的操作
	{
		int currentTime = glutGet(GLUT_ELAPSED_TIME);			// 再次获取程序时间（第一次获取是在程序初始化时）
		if (currentTime - startTime >= 20)					// 将两次获取到的时间作差，以判断（20ms）的时间差
		{
			playCount++;
			sum_playCount++;
			if (sum_playCount<160 && playCount % 20 == 0 && flag % 2 == 0)		// flag每次自加，奇数偶数控制交替过程
			{
				if (theta[LeftUpperLeg] > 180)
				{
					playBallHeight -= 0.6;					// 球落地
					theta[LeftUpperLeg] = 180;				// 收回腿
					my_mesh2->set_translate(-0.3, -0.3 + playBallHeight, ballTranDelta2);
				}
				playCount = 0;
				flag++;
				startTime = currentTime;							// 更新开始时间为当前时间，便于下次调用空闲回调函数时作差
			}
			else if (sum_playCount<160 && playCount % 20 == 0 && flag % 2 == 1)
			{
				if (theta[LeftUpperLeg] <= 180)
				{
					playBallHeight += 0.6;					// 球上升
					theta[LeftUpperLeg] = 225;				// 踢腿
					my_mesh2->set_translate(-0.3, -0.3 + playBallHeight, ballTranDelta2);
				}
				playCount = 0;
				flag++;
				startTime = currentTime;							// 更新开始时间为当前时间，便于下次调用空闲回调函数时作差
			}
			else if (sum_playCount >= 160)
			{
				flag = 0;
				playCount = 0;
				sum_playCount = 0;
				playBallHeight = 0;
				my_mesh2->set_translate(-0.3, -0.3, 0);		// 球落回原地
				my_mesh2->set_theta_step(0, 0, 0);			// 球停止
				isPlay = false;
				glutIdleFunc(NULL); 					// 停止调用空闲回调函数
			}
		}
	}
	glutPostWindowRedisplay(mainWindow);
}

void reset()				// 重置所有变量和位置
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
	randomNum = rand() % 10;						// 产生[0,10)的随机数
//	std::cout << randomNum << std::endl;
	isKick = true;
	if (yaw >= 90 && yaw <= 260)					// 视角朝着球门
	{
		theta[LeftUpperLeg] = 245;					// 向球门踢球腿抬起的角度
	}
	else											// 视角背对球门
	{
		theta[LeftUpperLeg] = 120;					// 向球门相反的方向踢球腿抬起的角度
	}
	my_mesh2->set_theta(0.0, 0.0, 90.0);
	my_mesh2->set_theta_step(0, 10, 0);				// 设置足球旋转速度
	startTime = glutGet(GLUT_ELAPSED_TIME);			// 获取开始时间
	ballTranDelta1 = 0.0;
	ballTranDelta2 = 0.0;
	SoundEngine->play2D("audio/kickOrWalk.wav", GL_FALSE);		// 播放踢球的音效
	glutIdleFunc(idle);								// 调用空闲回调函数
}

void walk()
{
	isWalk = true;
	// 机器人走路时各部件的角度
	theta[RightUpperArm] = 135.0;
	theta[RightLowerArm] = 15.0;
	theta[LeftUpperArm] = 225.0;
	theta[LeftLowerArm] = 15.0;
	theta[RightUpperLeg] = 220.0;
	theta[RightLowerLeg] = 350.0;
	theta[LeftUpperLeg] = 150.0;
	theta[LeftLowerLeg] = 345.0;
	startTime = glutGet(GLUT_ELAPSED_TIME);			// 获取开始时间
	glutIdleFunc(idle);								// 调用空闲回调函数
}

void play()
{
	isPlay = true;
	playBallHeight += 0.6;							// 足球弹起
	theta[LeftUpperLeg] = 225;						// 同时机器人有踢腿的动作
	my_mesh2->set_translate(-0.3, -0.3 + playBallHeight, ballTranDelta2);
	my_mesh2->set_theta_step(0, 10, 0);				// 设置足球旋转速度
	startTime = glutGet(GLUT_ELAPSED_TIME);			// 获取开始时间
	glutIdleFunc(idle); 							// 调用空闲回调函数
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033:								// 按下ESC键、q或Q键退出程序
	case 'q': 
	case 'Q': exit(EXIT_SUCCESS); break;
	case 'x':								// 按下x或X键，使相机绕X轴上下旋转
		pitch += 5.0;
		if (pitch > 85.0)
			pitch = 85.0;
		break;
	case 'X': 
		pitch -= 5.0;
		if (pitch < -5.0)
			pitch = -5.0;
		break;
	case 'y':								// 按下y或Y键，使相机绕Y轴左右旋转
		yaw += 5.0; 
		if (yaw == 360.0)
			yaw = 0.0;
		break;
	case 'Y': 
		yaw -= 5.0; 
		if (yaw == 0.0)
			yaw = 360.0;
		break;
	case 'z':								// 按下z或Z键，使相机拉近或拉远
		radius += 0.1;  
		break;
	case 'Z': 
		radius -= 0.1; 
		if (radius < 0.0)
			radius += 0.1;
		break;
	case 'f':								// 按下f或F键，改变透视投影参数fovy
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
	case 'a': 								// 按下a或A键，改变透视投影参数aspect
		aspect += 0.1; 
		break;
	case 'A': 
		aspect -= 0.1; 
		break;
	case 'n': 								// 按下n或N键，改变透视投影参数zNear
		zNear += 0.1; 
		break;
	case 'N': 
		zNear -= 0.1;
		if (zNear == 0)
		{
			zNear = 0.1;
		}
		break;
	case 'm': 								// 按下m或M键，改变透视投影参数zFar
		zFar += 0.1; 
		break;
	case 'M': 
		zFar -= 0.1; 
		break;
	case 'o': 								// 按下o或O键，使相机拉近或拉远
		orthoscale += 0.1; 
		break;
	case 'O': 
		orthoscale -= 0.1; 
		break;
	case ' ': 
		reset();
		break;
	case 'k':			// 按下键盘‘k’键，执行机器人踢球的动画
		kick();
		break;
	case 'w':			// 按下键盘‘w’键，执行机器人走路的动画
		walk();
		break;
	case 'p':			// 按下键盘‘p’键，执行机器人颠球的动画
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
	else if (option == ChangeLightPos)		// 改变光源
	{
		toChangeLightPos = true;
	}
	else if (option == Kick)				// 机器人“踢球”
	{
		kick();
	}
	else if (option == Walk)				// 机器人“走路”
	{
		walk();
	}
	else if (option == Play)				// 机器人“颠球”
	{
		play();
	}
	else if (option == Reset)				// 重置所有变量和位置
	{
		reset();
	}
	else
	{	
		angle = option;						// 选中机器人的某一部件
	}
}

void setupMenu()
{	
	subMenu = glutCreateMenu(menuEvent);		// 创建子菜单
	// 机器人各部件
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

	mainMenu = glutCreateMenu(menuEvent);					// 创建主菜单
	glutAddSubMenu("action", subMenu);			// 在主菜单中加入子菜单
	glutAddMenuEntry("changeLightPos", ChangeLightPos);		// 改变光源位置
	glutAddMenuEntry("kick", Kick);				// 机器人踢球
	glutAddMenuEntry("walk", Walk);				// 机器人走路
	glutAddMenuEntry("play", Play);				// 机器人颠球
	glutAddMenuEntry("reset", Reset);			// 重置所有变量和位置
	glutAddMenuEntry("quit", Quit);				// 退出程序
	glutAttachMenu(GLUT_MIDDLE_BUTTON);			// 绑定鼠标右键并激活菜单
}

void generateScene()
{
	mp_ = new Mesh_Painter;

	my_mesh1 = new My_Mesh;
	int box_size = 1;
	my_mesh1->load_obj("texture/box.obj", box_size);			// 读入长方体，表示一个足球场
	my_mesh1->set_texture_file("texture/box2.png");
	my_mesh1->set_translate(0.0, -0.6, 0.0);
	my_mesh1->set_theta(0.0, 90.0, 0.0);
	my_mesh1->set_theta_step(0, 0, 0);
	my_meshs.push_back(my_mesh1);
	mp_->add_mesh(my_mesh1);

	my_mesh2 = new My_Mesh;
	my_mesh2->load_obj("texture/football.obj", box_size);		// 读入足球
	my_mesh2->set_texture_file("texture/football1.png");
	my_mesh2->set_translate(-0.3, -0.3, 0.0);
	my_mesh2->set_theta(0.0, 0.0, 90.0);
	my_mesh2->set_theta_step(0, 0, 0);
	my_meshs.push_back(my_mesh2);
	mp_->add_mesh(my_mesh2);

	my_mesh3 = new My_Mesh;
	my_mesh3->load_obj("texture/gate.obj", box_size);			// 读入球门
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
	
	reset();			// 重置所有变量和位置
}


// 输出帮助信息
void printHelp() {
	printf("\n%s\n", "2014150270-陈炜轩-期末大作业");
	printf("Keyboard options:\n");
	printf("x/X: 相机绕X轴旋转\n");
	printf("y/Y: 相机绕Y轴旋转\n");
	printf("z/Z: 相机拉近拉远\n");
	printf("f/F: 改变透视投影fovy\n");
	printf("a/A: 改变透视投影aspect\n");
	printf("k: 机器人踢球\n");
	printf("w: 机器人走路\n");
	printf("p: 机器人颠球\n");
	printf(" : 空格，重置\n");
	printf("q/Q/Esc: 退出\n");
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(900, 900);				// 初始化窗口大小
	glutInitWindowPosition(150, 50);			// 初始化窗口位置
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	mainWindow = glutCreateWindow("2014150270-陈炜轩-期末大作业");		// 创建窗口
	glewExperimental = GL_TRUE;
	glewInit();

	generateScene();			// 生成场景

	init();

	setupMenu();				// 建立右键菜单

	// 输出帮助信息
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
