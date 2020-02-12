#include "Angel.h"

#include <vector>
#include <string>
#include <fstream>

#pragma comment(lib, "glew32.lib")

// 三角面片中的顶点序列
typedef struct vIndex {
	unsigned int a, b, c;
	vIndex(int ia, int ib, int ic) : a(ia), b(ib), c(ic) {}
} vec3i;

std::string filename;
std::vector<vec3> vertices; //从文件中读取的顶点坐标
std::vector<vec3i> faces;   //从文件中读取的面顶点索引

int nVertices = 0;
int nFaces = 0;
int nEdges = 0;

std::vector<vec3> points;   //传入着色器的绘制点
std::vector<vec3> colors;   //传入着色器的颜色

const int NUM_VERTICES = 8;

const vec3 vertex_colors[NUM_VERTICES] = {
	vec3(1.0, 1.0, 1.0),  // White 0
	vec3(1.0, 1.0, 0.0),  // Yellow 1
	vec3(0.0, 1.0, 0.0),  // Green 2
	vec3(0.0, 1.0, 1.0),  // Cyan 3--蓝绿色 
	vec3(1.0, 0.0, 1.0),  // Magenta 4--品红色 
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

	// @TODO：修改此函数读取OFF文件中三维模型的信息

	//读入首行字符串"OFF"
	std::string str;
	getline(fin,str);
	//读入点、面、边数目
	fin>>nVertices>>nFaces>>nEdges;
	// 读取每个顶点的坐标
	double a,b,c;
	for(int i=0;i<nVertices;i++){
		fin>>a>>b>>c;
		vertices.push_back(vec3(a,b,c));
	}
	// 读取面的顶点序列
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

	// @TODO： 修改此函数在points和colors容器中存储每个三角面片的各个点和颜色信息
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

	// 创建顶点数组对象
	GLuint vao[1];
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	// 创建并初始化顶点缓存对象
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vec3) + colors.size() * sizeof(vec3), NULL, GL_STATIC_DRAW);

	// @TODO：修改完成后再打开下面注释，否则程序会报错
	// 分别读取数据
	glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(vec3), &points[0]);
	glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(vec3), colors.size() * sizeof(vec3), &colors[0]);

	// 读取着色器并使用
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// 从顶点着色器中初始化顶点的位置
	GLuint pLocation = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(pLocation);
	glVertexAttribPointer(pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	// 从片元着色器中初始化顶点的颜色
	GLuint cLocation = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(cLocation);
	glVertexAttribPointer(cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(points.size() * sizeof(vec3)));

	// 黑色背景
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display(void)
{
	// 清理窗口，包括颜色缓存和深度缓存
//	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	// 绘制边
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// 消除背面光照
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glDrawArrays(GL_TRIANGLES, 0, points.size());
	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	// @TODO：窗口显示模式支持深度测试
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE|GLUT_DEPTH);// GLUT_DEPTH
	glutInitWindowSize(600, 600);
	glutCreateWindow("3D OFF Model 2017153005");

	glewExperimental = GL_TRUE;
	glewInit();

	// 读取off模型文件
	read_off("cube.off");

	init();
	glutDisplayFunc(display);

	// @TODO：启用深度测试
	glEnable(GL_DEPTH_TEST);
	
	glutMainLoop();
	return 0;
}
