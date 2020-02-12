/*
*        Computer Graphics Course - Shenzhen University
*      Week 6 - Camera Position and Control Skeleton Code
* ============================================================
*
* - ����������ǲο����룬����Ҫ����ο���ҵ˵��������˳������ɡ�
* - ��������OpenGL�����������������У���ο���һ��ʵ��γ�����ĵ���
*/

#include "include/Angel.h"
#include "include/TriMesh.h"

#pragma comment(lib, "glew32.lib")

#include <cstdlib>
#include <iostream>

using namespace std;

const double Theta = 0;
const double Phi = 0;
const double R = 1.0;

double theta=Theta, phi=Phi, r=R;

double Viewx=0, Viewy=0, Viewz=1;

GLuint programID;
GLuint vertexArrayID;
GLuint vertexBufferID;
GLuint vertexIndexBuffer;

GLuint vPositionID;
GLuint modelViewprojectMatrixID;

TriMesh* mesh = new TriMesh();

namespace Camera
{
    mat4 modelMatrix; //ģ�ͱ任����
    mat4 viewMatrix;  //����۲����
	mat4 projectMatrix; //ͶӰ����

	mat4 lookAt( const vec4& eye, const vec4& at, const vec4& up )
	{
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		//return mat4(1.0);
		vec4 n = normalize(eye - at);
		vec3 uu = normalize(cross(up, n));
		vec4 u = vec4(uu.x, uu.y, uu.z, 0.0);
		vec3 vv = normalize(cross(n, u));
		vec4 v = vec4(vv.x, vv.y, vv.z, 0.0);
		vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
		mat4 c = mat4(u, v, n, t);
		return c * Translate(-eye);
	}
}
//����λ�����λ������
void update() {
	Viewx = r*sin(theta)*cos(phi);
	Viewy = r*sin(theta)*sin(phi);
	Viewz = r*cos(theta);
}
//////////////////////////////////////////////////////////////////////////
// OpenGL ��ʼ��

void init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// ����shader���һ�ȡ������λ��
	programID = InitShader("vshader.glsl", "fshader.glsl");
	vPositionID = glGetAttribLocation(programID, "vPosition");
	modelViewprojectMatrixID = glGetUniformLocation(programID, "modelViewprojectMatrix");

	// ���ⲿ��ȡ��άģ���ļ�
	mesh->read_off("cube.off");//

	vector<vec3f> vs = mesh->v();
	vector<vec3i> fs = mesh->f();

	// ����VAO
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// ����VBO�����󶨶�������
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(vec3f), vs.data(), GL_STATIC_DRAW);

	// ����VBO�����󶨶�������
	glGenBuffers(1, &vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, fs.size() * sizeof(vec3i), fs.data(), GL_STATIC_DRAW);

	// OpenGL��Ӧ״̬����
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

//////////////////////////////////////////////////////////////////////////
// ��Ⱦ

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);

	// TODO �����������
	update();//�����������
	vec4 eye = vec4(Viewx,Viewy,Viewz,1);//���λ��
	vec4 at = vec4(0,0,0,1);//����ӵ�
	vec4 up = vec4(0,1,0,0);//���Ϸ���

	
	Camera::modelMatrix = mat4(1.0);
	Camera::viewMatrix = Camera::lookAt(eye, at, up);//�������lookAt��������ȡ����
	Camera::projectMatrix = mat4(1.0);
	
	mat4 modelViewprojectMatrix = Camera::viewMatrix * Camera::modelMatrix;//��ȡmodelViewprojectMatrix����
	glUniformMatrix4fv(modelViewprojectMatrixID, 1, GL_TRUE, &modelViewprojectMatrix[0][0]);//����glUniformMatrix4fv

	
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);

	glDrawElements(
		GL_TRIANGLES,
		int(mesh->f().size() * 3),
		GL_UNSIGNED_INT,
		(void*)0
	);

	glDisableVertexAttribArray(vPositionID);
	glUseProgram(0);

	glutSwapBuffers();
}

//////////////////////////////////////////////////////////////////////////
// �������ô���

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
}


//��ӡ������ʾ�˵�
void printHelp() {
	printf("%s\n\n", "OpenGL-Tutorial");
	printf("Keyboard options:\n");
	printf("r: Increase R\n");
	printf("f: Decrease R\n");
	printf("w: Increase Theta\n");
	printf("s: Decrease Theta\n");
	printf("e: Increase Phi\n");
	printf("d: Decrease Phi\n");
	printf("t: Reset all deltas\n");
}
// ��ԭTheta��Delta
void Reset()
{
	theta = Theta, phi = Phi, r = R;
}
// ����r
void updateR(int sign) {
	r += sign*0.1;
	/*if (r >= 2.0) r -= 0.1;
	if (r <= 0) r += 0.1;*/
	glutPostRedisplay();
}
//����Theta
void updateTheta(int sign) {
	theta += sign*0.1;
	glutPostRedisplay();
}
//����Phi
void updatePhi(int sign) {
	phi += sign*0.1;
	glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////
// �����Ӧ����

void mouse(int button, int state, int x, int y)
{
	return;
}

//////////////////////////////////////////////////////////////////////////
// ������Ӧ����
void keyboard(unsigned char key, int x, int y)
{
	
	// Todo�����̿��������λ�úͳ���
	switch(key) 
	{
	case 'r':
		updateR(1);
		break;
	case 'f':
		updateR(-1);
		break;
	case 'w':
		updateTheta(1);
		break;
	case 's':
		updateTheta(-1);
		break;
	case 'e':
		updatePhi(1);
		break;
	case 'd':
		updatePhi(-1);
		break;
	case 't':
		Reset();
		break;
	case 033:	// ESC�� �� 'q' ���˳���Ϸ
		exit(EXIT_SUCCESS);
		break;
	case 'q':
		exit (EXIT_SUCCESS);
		break;
	}
	glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////

void idle(void)
{
	//phi += 0.1;//
	glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////

void clean()
{
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &vertexArrayID);

	if (mesh) {
		delete mesh;
		mesh = NULL;
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	glutCreateWindow("2017153005_OpenGL-Tutorial");

	glewInit();
	init();
	
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	// ���������Ϣ
	printHelp();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop();

	clean();

	return 0;
}