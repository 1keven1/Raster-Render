#pragma once
//������������һЩȫ����Ҫ�õ�������r
#include <Eigen/Eigen>
#include "Triangle.hpp"

constexpr double PI = 3.1415926;
constexpr bool MSAA4X = false;
constexpr bool BLERP = true;

struct Camera
{
	Vector3f position;	//�������
	Vector3f g;			//�ۿ����򣬵�λ
	Vector3f t;			//������������λ
	float fov;			//��Ұ���ߣ�
	float nNear;		//��ƽ��
	float nFar;			//Զƽ��
	float aspectRatio;	//��Ļ��߱�
};

struct Object
{
	std::vector<Triangle> triangles;	//������
	Vector3f position;					//λ��
	Vector3f rotation;					//��ת
	Vector3f scale;						//����
};

struct Light
{
	Light(Vector4f p, Vector3f i, Vector3f aI) : position(p), intensity(i), ambientIntensity(aI) {}
	Vector4f position;
	Vector3f intensity;
	Vector3f ambientIntensity;
};

enum ShadingFunction
{
	EBaseVertexColor,	// ����ɫ
	ENormalColor,		// ������ɫ
	EBlinnPhongShading, // Blinn-Phong��ɫ
	ETextureShading,	// ��ͼ��ɫ
	ETextureWithBump,	// �߶�ͼ��ɫ
	ETextureWithNormal	// ������ͼ��ɫ
};