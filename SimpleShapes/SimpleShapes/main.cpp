#include "Angel.h"

#pragma comment(lib, "glew32.lib")

const vec3 WHITE(1.0, 1.0, 1.0);
const vec3 BLACK(0.0, 0.0, 0.0);
const vec3 RED(1.0, 0.0, 0.0);
const vec3 GREEN(0.0, 1.0, 0.0);
const vec3 BLUE(0.0, 0.0, 1.0);

const int TRIANGLE_NUM_POINTS = 3;
const int SQUARE_NUM = 6;//
const int SQUARE_NUM_POINTS = 4 * SQUARE_NUM;
const int TOTAL_NUM_POINTS = TRIANGLE_NUM_POINTS + SQUARE_NUM_POINTS * SQUARE_NUM;//

// ��������ε�ÿ���Ƕ�
double getTriangleAngle(int point)
{
//    return 2 * M_PI / 3 * point;
    return 2 * M_PI / 3 * point + M_PI / 2;
}

// ��������ε�ÿ���Ƕ�
double getSquareAngle(int point)
{
    return M_PI / 4 + (M_PI / 2 * point);
}

// �����������ϵ�ÿ����
void generateTrianglePoints(vec2 vertices[], vec3 colors[], int startVertexIndex)
{
    /*�ڴ˺������޸������εĶ���λ��*/

    double scale = 0.25;
    vec2 center(0.0, 0.70);

    for (int i = 0; i < 3; ++i) {
        // ��ǰ�����Ӧ�ĽǶ�
        double currentAngle = getTriangleAngle(i);
        vertices[startVertexIndex + i] = vec2(cos(currentAngle), sin(currentAngle)) * scale + center;
    }

//    colors[startVertexIndex] = RED;
//    colors[startVertexIndex + 1] = GREEN;
//    colors[startVertexIndex + 2] = BLUE;
    
    
    colors[startVertexIndex] = RED;
    colors[startVertexIndex + 1] = BLUE;
    colors[startVertexIndex + 2] = GREEN;
}


// �����������ϵ�ÿ����

void generateSquarePoints(vec2 vertices[], vec3 colors[], int squarenumber, int startvertexindex)
{
    /*�ڴ˺������޸ģ����ɶ��Ƕ��������*/

    double scale = 0.90;
	double decrease = 0.15;
    vec2 center(0.0, -0.25);
    int vertexindex = startvertexindex;

    vec3 currentcolor = WHITE;
	for (int k = 0; k < squarenumber; ++k) {
		currentcolor = k % 2 ? BLACK : WHITE;
		for (int j = 0; j < 4; ++j) {
				// ��ǰ�����Ӧ�ĽǶ�
				double currentangle = getSquareAngle(j);
				vertices[vertexindex] = vec2(cos(currentangle), sin(currentangle)) * scale + center;
				colors[vertexindex] = currentcolor;
				vertexindex++;
			}
		scale -= decrease;
	}
    
}
//void generateSquarePoints(vec2 vertices[], vec3 colors[], int squareNumber, int startVertexIndex)
//{
//	double scale = 0.90;
//	double scaleDecrease = 0.15;
//	vec2 center(0.0, -0.25);
//	int vertexIndex = startVertexIndex;
//
//	for (int i = 0; i < squareNumber; ++i) {
//		vec3 currentColor;
//		currentColor = (i % 2) ? BLACK : WHITE;
//		for (int j = 0; j < 4; ++j) {
//			double currentAngle = getSquareAngle(j);
//			vertices[vertexIndex] = vec2(sin(currentAngle), cos(currentAngle)) * scale + center;
//			colors[vertexIndex] = currentColor;
//			vertexIndex++;
//		}
//		scale -= scaleDecrease;
//	}
//}

// �������ó������õ�������
void init()
{
    vec2 vertices[TOTAL_NUM_POINTS];
    vec3 colors[TOTAL_NUM_POINTS];

    // ����������״����λ�õĺ���
    generateTrianglePoints(vertices, colors, 0);
    generateSquarePoints(vertices, colors, SQUARE_NUM, TRIANGLE_NUM_POINTS);

    // ���������������
    GLuint vao[1];
    // ����1�������������
    glGenVertexArrays(1, vao);
    // �󶨶����������
    glBindVertexArray(vao[0]);

    // �������㻺�����
    GLuint buffer;
    // ����1�������������
    glGenBuffers(1, &buffer);
    // �󶨶��㻺�����
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // ������������Ĵ洢�ռ䣬�����ݿ�����OpenGL������ڴ�
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), NULL, GL_STATIC_DRAW);

    // �ֱ��ȡ����
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);

    // ��ȡ��ɫ����ʹ��
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    // �Ӷ�����ɫ���г�ʼ�������λ��
    GLuint pLocation = glGetAttribLocation(program, "vPosition");
    // ���ö�����������
    glEnableVertexAttribArray(pLocation);
    // ������������������ (index, size, type, normalized, stride, *pointer)
    glVertexAttribPointer(pLocation, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    // ��ƬԪ��ɫ���г�ʼ���������ɫ
    GLuint cLocation = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(cLocation);
    glVertexAttribPointer(cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)));

    // �����˵�ǰʹ�õ������ɫֵ����������Ϊ��ɫ����
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

// ������Ⱦ��Ҫ������
void display(void)
{
    // ����ָ���������ݲ�����Ϊ��ǰ�����ֵ
    glClear(GL_COLOR_BUFFER_BIT);

    // ʹ�õ�ǰ�󶨵Ķ������ݽ�������ͼԪ (mode, first, count)
    glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_NUM_POINTS);

    // ���ƶ��������
    for (int i = 0; i < SQUARE_NUM; ++i) {
        glDrawArrays(GL_TRIANGLE_FAN, TRIANGLE_NUM_POINTS + (i * 4), 4);
    }

    // ǿ�����н����е�OpenGL�������
    //glFlush()
    glutSwapBuffers();
}

// �������ڡ���ʼ���������¼�ѭ��
int main(int argc, char **argv)
{
    // ��ʼ��GLUT�⣬������Ӧ�ó�����õĵ�һ��GLUT����
    glutInit(&argc, argv);

    // ���ô��ڵ���ʾ����
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    glutCreateWindow("Simple 2D Shapes");

    glewExperimental = GL_TRUE;
    glewInit();

    init();

    // ָ����ǰ���ڽ����ػ�ʱҪ���õĺ���
    glutDisplayFunc(display);

    // ����һֱ�����ںͲ���ϵͳ���û�����Ȳ���
    glutMainLoop();
    return 0;
}
