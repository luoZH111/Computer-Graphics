#include "Angel.h"

#include <vector>
#include <string>
#include <fstream>

#pragma comment(lib, "glew32.lib")

// ������Ƭ�еĶ�������
typedef struct vIndex {
	unsigned int a, b, c;
	vIndex(int ia, int ib, int ic) : a(ia), b(ib), c(ic) {}
} vec3i;

std::string filename;
std::vector<vec3> vertices; //���ļ��ж�ȡ�Ķ�������
std::vector<vec3i> faces;   //���ļ��ж�ȡ���涥������

int nVertices = 0;
int nFaces = 0;
int nEdges = 0;

std::vector<vec3> points;   //������ɫ���Ļ��Ƶ�
std::vector<vec3> colors;   //������ɫ������ɫ

const int NUM_VERTICES = 8;

const vec3 vertex_colors[NUM_VERTICES] = {
	vec3(1.0, 1.0, 1.0),  // White 0
	vec3(1.0, 1.0, 0.0),  // Yellow 1
	vec3(0.0, 1.0, 0.0),  // Green 2
	vec3(0.0, 1.0, 1.0),  // Cyan 3--����ɫ 
	vec3(1.0, 0.0, 1.0),  // Magenta 4--Ʒ��ɫ 
	vec3(1.0, 0.0, 0.0),  // Red 5
	vec3(0.0, 0.0, 0.0),  // Black 6
	vec3(0.0, 0.0, 1.0)   // Blue 7
};

void read_off(const std::string filename)
{
	if (filename.empty()) {
		return;
	}

	std::ifstream fin;
	fin.open(filename);

	// @TODO���޸Ĵ˺�����ȡOFF�ļ�����άģ�͵���Ϣ

	//���������ַ���"OFF"
	std::string str;
	getline(fin,str);
	//����㡢�桢����Ŀ
	fin>>nVertices>>nFaces>>nEdges;
	// ��ȡÿ�����������
	double a,b,c;
	for(int i=0;i<nVertices;i++){
		fin>>a>>b>>c;
		vertices.push_back(vec3(a,b,c));
	}
	// ��ȡ��Ķ�������
	for(int i=0;i<nFaces;i++){
		int k,a,b,c;//
		fin>>k>>a>>b>>c;
		faces.push_back(vec3i(a,b,c));
	}
	fin.close();
}

void storeFacesPoints()
{
	points.clear();
	colors.clear();

	// @TODO�� �޸Ĵ˺�����points��colors�����д洢ÿ��������Ƭ�ĸ��������ɫ��Ϣ
	for(int i=0;i<nFaces;i++){
		unsigned int a=faces[i].a,b=faces[i].b,c=faces[i].c;
		points.push_back(vertices[a]);
		points.push_back(vertices[b]);
		points.push_back(vertices[c]);
		colors.push_back(vertex_colors[a]);
		colors.push_back(vertex_colors[b]);
		colors.push_back(vertex_colors[c]);
	}
//	for(int i=0;i<nVertices;i++){
//		points.push_back(vertices[i]);
//		colors.push_back(vertex_colors[i]);
//	}
	
}

void init()
{
	storeFacesPoints();

	// ���������������
	GLuint vao[1];
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	// ��������ʼ�����㻺�����
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vec3) + colors.size() * sizeof(vec3), NULL, GL_STATIC_DRAW);

	// @TODO���޸���ɺ��ٴ�����ע�ͣ��������ᱨ��
	// �ֱ��ȡ����
	glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(vec3), &points[0]);
	glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(vec3), colors.size() * sizeof(vec3), &colors[0]);

	// ��ȡ��ɫ����ʹ��
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// �Ӷ�����ɫ���г�ʼ�������λ��
	GLuint pLocation = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(pLocation);
	glVertexAttribPointer(pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	// ��ƬԪ��ɫ���г�ʼ���������ɫ
	GLuint cLocation = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(cLocation);
	glVertexAttribPointer(cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(points.size() * sizeof(vec3)));

	// ��ɫ����
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display(void)
{
	// �����ڣ�������ɫ�������Ȼ���
//	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	// ���Ʊ�
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// �����������
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glDrawArrays(GL_TRIANGLES, 0, points.size());
	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	// @TODO��������ʾģʽ֧����Ȳ���
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE|GLUT_DEPTH);// GLUT_DEPTH
	glutInitWindowSize(600, 600);
	glutCreateWindow("3D OFF Model 2017153005");

	glewExperimental = GL_TRUE;
	glewInit();

	// ��ȡoffģ���ļ�
	read_off("cube.off");

	init();
	glutDisplayFunc(display);

	// @TODO��������Ȳ���
	glEnable(GL_DEPTH_TEST);
	
	glutMainLoop();
	return 0;
}
