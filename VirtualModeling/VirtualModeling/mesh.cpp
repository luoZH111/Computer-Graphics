#include "mesh.h"
#include <sstream>
#include <fstream>
#include <string>
#include <iosfwd>
#include <algorithm>
#include <gl/GL.h>
#include <math.h>
#include <algorithm>


My_Mesh::My_Mesh()
{

	vTranslation[0] = Theta[0] = 0;
	vTranslation[1] = Theta[1] = 0;
	vTranslation[2] = Theta[2] = 0;
	Theta[0] = 45;
}

My_Mesh::~My_Mesh()
{
	
}

void My_Mesh::load_obj(std::string obj_File, int box_size)
{
	this->clear_data();
	this->m_center_ = point3f(0, 0, 0);
	this->m_min_box_ = point3f(-box_size, -box_size, -box_size);
	this->m_max_box_ = point3f(box_size, box_size, box_size);

	// ʵ�ֶԺ���UV�����obj�ļ��Ķ�ȡ
	std::ifstream fin;
	fin.open(obj_File.c_str(), std::ios::in);
	if (fin.good())
	{
		int i;
		std::string str;
		std::string en;
		double vertexX, vertexY, vertexZ;
		double verTextureX, verTextureY, verTextureZ;
		double verNormalX, verNormalY, verNormalZ;
		int indexA, indexB, indexC;
		NormalList temp_m_normals_;
		VtList	temp_m_vt_list_;
		FaceList temp_m_face_A, temp_m_face_B, temp_m_face_C;
		while (fin >> en)
		{
			if (en[0] != 'v' && en[0] != 'f')
			{
				getline(fin, str, '\n');
			//	std::cout << str << std::endl;
			}
			else
			{
				if (en == "v")			// ��������
				{
					fin >> vertexX >> vertexY >> vertexZ;
					m_vertices_.push_back(vertexX);
					m_vertices_.push_back(vertexY);
					m_vertices_.push_back(vertexZ);
				}
				else if (en == "vn")		// ����������
				{
					fin >> verNormalX >> verNormalY >> verNormalZ;
					temp_m_normals_.push_back(verNormalX);
					temp_m_normals_.push_back(verNormalY);
					temp_m_normals_.push_back(verNormalZ);
				}
				else if (en == "vt")		// ��������
				{
					fin >> verTextureX >> verTextureY >> verTextureZ;
					temp_m_vt_list_.push_back(verTextureX);
					temp_m_vt_list_.push_back(verTextureY);
					temp_m_vt_list_.push_back(verTextureZ);
				}
				else if (en == "f")			// ��Ƭ��Ϣ
				{
					char ch;
					i = 3;				// һ��������Ƭ�������������
					while (i--)
					{
						// ���������� +��/��+ ������������ +��/��+ ��������������
						fin >> indexA >> ch >> indexB >> ch >> indexC;
						temp_m_face_A.push_back(indexA);
						temp_m_face_B.push_back(indexB);
						temp_m_face_C.push_back(indexC);
					}
				}
			}
		}
		i = 0;
		while (i < temp_m_face_A.size())
		{
			indexA = temp_m_face_A[i];
			indexB = temp_m_face_B[i];
			indexC = temp_m_face_C[i];
			// ���Ƚ���������ѹ����Ƭ��Ϣ����
			m_faces_.push_back(indexA - 1);
			// �����������������ҵ���Ӧ���������꣬��ѹ�������������飬������������ֻ��U��V����������ֻѹ������������
			m_vt_list_.push_back(temp_m_vt_list_[(indexB - 1) * 3 + 0]);
			m_vt_list_.push_back(temp_m_vt_list_[(indexB - 1) * 3 + 1]);
			// ���ݷ������������ҵ���Ӧ�ķ������꣬��ѹ�뷨����������
			m_normals_.push_back(temp_m_normals_[(indexC - 1) * 3 + 0]);
			m_normals_.push_back(temp_m_normals_[(indexC - 1) * 3 + 1]);
			m_normals_.push_back(temp_m_normals_[(indexC - 1) * 3 + 2]);
			// ���ݷ���������ɫ
			float r, g, b;
			verNormalX = temp_m_normals_[(indexC - 1) * 3 + 0];
			verNormalY = temp_m_normals_[(indexC - 1) * 3 + 1];
			verNormalZ = temp_m_normals_[(indexC - 1) * 3 + 2];
			My_Mesh::normal_to_color(verNormalX, verNormalY, verNormalZ, r, g, b);
			m_color_list_.push_back(r);
			m_color_list_.push_back(g);
			m_color_list_.push_back(b);

			i++;
		}
		fin.close();
	}
};

void My_Mesh::normal_to_color(float nx, float ny, float nz, float& r, float& g, float& b)
{
	r = float(std::min(std::max(0.5 * (nx + 1.0), 0.0), 1.0));
	g = float(std::min(std::max(0.5 * (ny + 1.0), 0.0), 1.0));
	b = float(std::min(std::max(0.5 * (nz + 1.0), 0.0), 1.0));
};

const VtList&  My_Mesh::get_vts()
{
	return this->m_vt_list_;
};
void My_Mesh::clear_data()
{
	m_vertices_.clear();
	m_normals_.clear();
	m_faces_.clear();
	m_color_list_.clear();
	m_vt_list_.clear();
};
void My_Mesh::get_boundingbox(point3f& min_p, point3f& max_p) const
{
	min_p = this->m_min_box_;
	max_p = this->m_max_box_;
};
const STLVectorf&  My_Mesh::get_colors()
{
	return this->m_color_list_;
};
const VertexList& My_Mesh::get_vertices()
{
	return this->m_vertices_;
};
const NormalList& My_Mesh::get_normals()
{
	return this->m_normals_;
};
const FaceList&   My_Mesh::get_faces()
{
	return this->m_faces_;
};

int My_Mesh::num_faces()
{
	return this->m_faces_.size()/3;
};
int My_Mesh::num_vertices()
{
	return this->m_vertices_.size()/3;
};

const point3f& My_Mesh::get_center()
{
	return this->m_center_;
};

void My_Mesh::generate_cylinder(int num_division, float height)
{
	this->clear_data();
	this->m_center_ = point3f(0, 0, 0);
	this->m_min_box_ = point3f(-1, -1, -height);
	this->m_max_box_ = point3f(1, 1, height);

	int num_samples = num_division;
	float z = -height;
	float pi = 3.14159265;
	float step = 1.0 * 360 / num_samples;
	float rr = pi / 180;
	//Բ����Z�����ϣ���cos��sin����x��y����
	for (int i = 0; i < num_samples; i++)
	{
		float r_r_r = i * step * rr;
		float x = cos(r_r_r);
		float y = sin(r_r_r);
		m_vertices_.push_back(x);
		m_vertices_.push_back(y);
		m_vertices_.push_back(z);

		m_normals_.push_back(x);
		m_normals_.push_back(y);
		m_normals_.push_back(0);
		//������������
		float r;
		float g;
		float b;
		My_Mesh::normal_to_color(x, y, z, r, g, b);
		//������÷�����������ɫ��ѧ�������Զ����Լ�����ɫ���ɷ�ʽ
		m_color_list_.push_back(r);
		m_color_list_.push_back(g);
		m_color_list_.push_back(b);
	}

	z = height;
	//Բ����Z�����ϣ���cos��sin����x��y���꣬
	for (int i = 0; i < num_samples; i++)
	{
		float r_r_r = i * step * rr;
		float x = cos(r_r_r);
		float y = sin(r_r_r);
		m_vertices_.push_back(x);
		m_vertices_.push_back(y);
		m_vertices_.push_back(z);

		m_normals_.push_back(x);
		m_normals_.push_back(y);
		m_normals_.push_back(0);
		//������������
		float r;
		float g;
		float b;
		My_Mesh::normal_to_color(x, y, z, r, g, b);
		m_color_list_.push_back(r);
		m_color_list_.push_back(g);
		m_color_list_.push_back(b);
		//������÷�����������ɫ��ѧ�������Զ����Լ�����ɫ���ɷ�ʽ
	}

	for (int i = 0; i < num_samples; i++)
	{
		m_faces_.push_back(i);
		m_faces_.push_back((i + 1) % num_samples);
		m_faces_.push_back((i + num_samples) % (num_samples)+num_samples);

		m_faces_.push_back((i + num_samples) % (num_samples)+num_samples);
		m_faces_.push_back((i + 1) % num_samples);
		m_faces_.push_back((i + num_samples + 1) % (num_samples)+num_samples);
		//����������Ƭ

		m_vt_list_.push_back(1.0 * i / num_samples);
		m_vt_list_.push_back(0.0);
		//��������
		m_vt_list_.push_back(1.0 * ((i + 1)) / num_samples);
		m_vt_list_.push_back(0.0);
		//��������
		m_vt_list_.push_back(1.0 * i / num_samples);
		m_vt_list_.push_back(1.0);
		//��������

		m_vt_list_.push_back(1.0 * i / num_samples);
		m_vt_list_.push_back(1.0);
		//��������
		m_vt_list_.push_back(1.0 * ((i + 1)) / num_samples);
		m_vt_list_.push_back(0.0);
		//��������
		m_vt_list_.push_back(1.0 * ((i + 1)) / num_samples);
		m_vt_list_.push_back(1.0);
		//��������
	}

};

void My_Mesh::set_texture_file(std::string s)
{
	this->texture_file_name = s;
};
std::string My_Mesh::get_texture_file()
{
	return this->texture_file_name;
};

void My_Mesh::set_translate(float x, float y, float z)
{
	vTranslation[0] = x;
	vTranslation[1] = y;
	vTranslation[2] = z;

};
void My_Mesh::get_translate(float& x, float& y, float& z)
{
	x = vTranslation[0];
	y = vTranslation[1];
	z = vTranslation[2];
};

void My_Mesh::set_theta(float x, float y, float z)
{
	Theta[0] = x;
	Theta[1] = y;
	Theta[2] = z;
};
void My_Mesh::get_theta(float& x, float& y, float& z)
{
	x = Theta[0];
	y = Theta[1];
	z = Theta[2];
};

void My_Mesh::set_theta_step(float x, float y, float z)
{
	Theta_step[0] = x;
	Theta_step[1] = y;
	Theta_step[2] = z;
};

void My_Mesh::add_theta_step()
{
	Theta[0] = Theta[0] + Theta_step[0];
	Theta[1] = Theta[1] + Theta_step[1];
	Theta[2] = Theta[2] + Theta_step[2];
};