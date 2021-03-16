#ifndef TRIANGLE_H
#define	TRIANGLE_H

#include<Eigen/Eigen>
using namespace Eigen;

class Triangle
{
public:
	Vector4f v[3]; //����������������
	Vector3f color[3]; //�����������ɫ
	Vector4f normal[3]; //������������
	Vector2f texCoord[3]; //������������
	Triangle()
	{
		for (int i = 0; i < 3; i++)
		{
			v[i] << 0, 0, 0, 1;
			color[i] << 0, 0, 0;
			normal[i] << 0, 0, 0, 1;
			texCoord[1] << 0, 0;
		}		
	}
	//���ö�������
	void SetVertex(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2)
	{
		v[0] = Vector4f(v0.x(), v0.y(), v0.z(), 1.0f);
		v[1] = Vector4f(v1.x(), v1.y(), v1.z(), 1.0f);
		v[2] = Vector4f(v2.x(), v2.y(), v2.z(), 1.0f);
	}
	void SetVertex(const int& i, const Vector3f& ver)
	{
		v[i] = Vector4f(ver.x(), ver.y(), ver.z(), 1.f);
	}
	//���ö�����ɫ
	void SetColor(const Vector3f& c0, const Vector3f& c1, const Vector3f& c2)
	{
		color[0] = c0;
		color[1] = c1;
		color[2] = c2;
	}
	void SetColor(const int& i, const Vector3f& col)
	{
		color[i] = col;
	}
	//���÷���
	void SetNormal(const int& i, const Vector3f& n)
	{
		normal[i] = Vector4f(n.x(), n.y(), n.z(), 0.f);
	}
	//������������
	void SetTexCoord(const int& i, const Vector2f& tC)
	{
		texCoord[i] = tC;
	}
};

#endif