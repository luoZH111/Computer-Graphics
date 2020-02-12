#include "Angel.h"

#pragma comment(lib, "glew32.lib")

const vec3 WHITE(1.0, 1.0, 1.0);
const vec3 BLACK(0.0, 0.0, 0.0);
const vec3 RED(1.0, 0.0, 0.0);
const vec3 GREEN(0.0, 1.0, 0.0);
const vec3 BLUE(0.0, 0.0, 1.0);

const int CIRCLE_NUM_POINTS = 100;
const int ELLIPSE_NUM = 2;//��Բ������
const int ELLIPSE_NUM_POINTS = 100;
const int TRIANGLE_NUM = 4;//����������
const int TRIANGLE_NUM_POINTS = 3*TRIANGLE_NUM;//

const int SQUARE_NUM = 2;//����������
const int SQUARE_NUM_POINTS = 4 * SQUARE_NUM;

const int LINE_NUM_POINTS = 2;
const int TOTAL_NUM_POINTS = TRIANGLE_NUM_POINTS + SQUARE_NUM_POINTS + LINE_NUM_POINTS + CIRCLE_NUM_POINTS + ELLIPSE_NUM_POINTS*ELLIPSE_NUM;

// ���Բ�ϵĵ�
vec2 getEllipseVertex(vec2 center, double scale, double verticalScale, double angle)
{
	vec2 vertex(sin(angle), cos(angle));
	vertex *= scale;
	vertex.y *= verticalScale;
	vertex += center;
	
	return vertex;
}

// ���ݽǶ�������ɫ
float generateAngleColor(double angle)
{
	return 1.0 / (2 * M_PI) * angle;
}

// ��������ε�ÿ���Ƕ�
double getTriangleAngle(int point)
{
	return 2 * M_PI / 3 * (point%3)+M_PI/2*(point/3);
}

// ��������ε�ÿ���Ƕ�
double getSquareAngle(int point)
{
	return M_PI / 4 + (M_PI / 2 * point);
}

void generateEllipsePoints(vec2 vertices[], vec3 colors[], int startVertexIndex, int numPoints,
	vec2 center, double scale, double verticalScale)
{
	double angleIncrement = (2 * M_PI) / numPoints;
	double currentAngle = M_PI / 2;

	for (int i = startVertexIndex; i < startVertexIndex + numPoints; ++i) {
		vertices[i] = getEllipseVertex(center, scale, verticalScale, currentAngle);
		if (verticalScale == 1.0) {
			colors[i] = vec3(generateAngleColor(currentAngle), 0.0, 0.0);
		} else {
			colors[i] = RED;
		}
		currentAngle += angleIncrement;
	}
}

void generateTrianglePoints(vec2 vertices[], vec3 colors[], int startVertexIndex,int TRIANGLE_NUM)
{
	double scale = 0.25;
	vec2 center(0.0, 0.70);//�����ε�����
	for (int k = 0; k < TRIANGLE_NUM; k++) {//����TRIANGLE_NUM�������Ρ�
		for (int i = 0; i < 3; ++i) {
			double currentAngle = getTriangleAngle(i+k*3);
			vertices[startVertexIndex + i+k*3] = vec2(sin(currentAngle), cos(currentAngle)) * scale + center;
		}
	}

	for (int k = 0; k < TRIANGLE_NUM; k++) {//��ÿ�������ζ�����ɫ��ÿ����������ɫ����ͬ
		colors[startVertexIndex + 3 * k] = BLUE+GREEN*0.5;
		colors[startVertexIndex + 1 + 3 * k] = BLUE + GREEN*0.5;
		colors[startVertexIndex + 2 + 3 * k] = BLUE + GREEN*0.5;
	}

}

void generateSquarePoints(vec2 vertices[], vec3 colors[], int squareNumber, int startVertexIndex)
{
	double scale = 0.90;
	double scaleDecrease = 0.15;
	vec2 center(0.0, -0.25);
	int vertexIndex = startVertexIndex;

	for (int i = 0; i < squareNumber; ++i) {
		vec3 currentColor;
		currentColor = (i % 2) ? BLACK : BLUE*0.5;//��ɫ����Ϊǳ��
		for (int j = 0; j < 4; ++j) {
			double currentAngle = getSquareAngle(j);
			vertices[vertexIndex] = vec2(sin(currentAngle), cos(currentAngle)) * scale + center;
			colors[vertexIndex] = currentColor;
			vertexIndex++;
		}
		scale -= scaleDecrease;
	}
}

void generateLinePoints(vec2 vertices[], vec3 colors[], int startVertexIndex)
{
	vertices[startVertexIndex] = vec2(-1.0, -1.0);
	vertices[startVertexIndex + 1] = vec2(1.0, 1.0);


	colors[startVertexIndex] = WHITE;
	colors[startVertexIndex + 1] = BLUE;
}


void init()
{
	vec2 vertices[TOTAL_NUM_POINTS];
	vec3 colors[TOTAL_NUM_POINTS];

	// ���ɸ�����״�ϵĵ�
	generateTrianglePoints(vertices, colors, 0,TRIANGLE_NUM);
	generateSquarePoints(vertices, colors, SQUARE_NUM, TRIANGLE_NUM_POINTS);
	generateLinePoints(vertices, colors, TRIANGLE_NUM_POINTS + SQUARE_NUM_POINTS);
	

	/*����Բ�κ���Բ�ϵĵ����ɫ*/
	vec2 center1(0.0, -0.25);//Բ��Բ��λ��
	double Scale1 = 0.2;//Բ�İ뾶����
	double vertualScale1 = 1;//Բ��vertualScaleΪ1
	generateEllipsePoints(vertices, colors, TRIANGLE_NUM_POINTS + SQUARE_NUM_POINTS + LINE_NUM_POINTS, CIRCLE_NUM_POINTS, center1, Scale1, vertualScale1);


	for (int k = 0; k < ELLIPSE_NUM; k++) {//����2����Բ
		vec2 center(k*1.0-0.5, 0.7);//��Բ��Բ��λ��
		double Scale = 0.2;//��Բ�뾶����
		double vertualScale = 0.4;//����Բ�߶�
		generateEllipsePoints(vertices, colors, TRIANGLE_NUM_POINTS + SQUARE_NUM_POINTS + LINE_NUM_POINTS + CIRCLE_NUM_POINTS+k*ELLIPSE_NUM_POINTS, ELLIPSE_NUM_POINTS, center, Scale, vertualScale);
	}

	// ���������������
	GLuint vao[1];
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	// ��������ʼ�����㻺�����
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), NULL, GL_STATIC_DRAW);

	// �ֱ��ȡ����
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);

	// ��ȡ��ɫ����ʹ��
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// �Ӷ�����ɫ���г�ʼ�������λ��
	GLuint pLocation = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(pLocation);
	glVertexAttribPointer(pLocation, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	// ��ƬԪ��ɫ���г�ʼ���������ɫ
	GLuint cLocation = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(cLocation);
	glVertexAttribPointer(cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)));

	// ǳ��ɫ����
	glClearColor(0.0, 0.5, 0.0, 1.0);
}

void display(void)
{
	// ������
	glClear(GL_COLOR_BUFFER_BIT);

	// ����������
	glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_NUM_POINTS);
	// ���ƶ��������
	for (int i = 0; i < SQUARE_NUM; ++i) {
		glDrawArrays(GL_TRIANGLE_FAN, TRIANGLE_NUM_POINTS + (i * 4), 4);
	}
	// ������
	glDrawArrays(GL_LINES, TRIANGLE_NUM_POINTS + SQUARE_NUM_POINTS, LINE_NUM_POINTS);
	// ����Բ
	glDrawArrays(GL_POLYGON, TRIANGLE_NUM_POINTS + SQUARE_NUM_POINTS + LINE_NUM_POINTS, CIRCLE_NUM_POINTS);//GL_POLYGON
	// ����2����Բ
	glDrawArrays(GL_POLYGON, TRIANGLE_NUM_POINTS + SQUARE_NUM_POINTS + LINE_NUM_POINTS+ CIRCLE_NUM_POINTS, ELLIPSE_NUM_POINTS);//GL_POLYGON
	glDrawArrays(GL_POLYGON, TRIANGLE_NUM_POINTS + SQUARE_NUM_POINTS + LINE_NUM_POINTS + CIRCLE_NUM_POINTS+ ELLIPSE_NUM_POINTS, ELLIPSE_NUM_POINTS);//GL_POLYGON
	glFlush();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(500, 500);
	glutCreateWindow("�����2017153005");

	glewExperimental = GL_TRUE;
	glewInit();

	init();
	glutDisplayFunc(display);

	glutMainLoop();
	return 0;
}
