#pragma once
#include <iostream>
#include <optional>
#include "global.h"
#include "Triangle.hpp"
#include "Log.hpp"
#include "shader.hpp"
#include "texture.hpp"
using namespace Eigen;

class Rasterizer
{
public:
	Rasterizer(int w, int h); //���캯��
	void VertexShader(std::vector<Object>& objectList, std::vector<Light>& lightList, Camera c); //������ɫ��
	void FragmentShader(const std::vector<Object>& objectList, const std::vector<Light>& lightList); //ƬԪ��ɫ��
	void Clear();

	//���ø�����ͼ
	void SetTexture(Texture tex);
	void SetBumpMap(Texture bTex);
	void SetNormalMap(Texture nTex);

	std::vector<Vector3f>& GetFrameBuffer(); //����frame buffer

private:
	//������ת������
	void SetModelMatrix(const Object& o);
	void SetViewMatrix(const Camera& c);
	void SetProjectionMatrix(const Camera& c);
	//������������������
	std::tuple<float, float, float> Barycentric2D(float x, float y, const Vector4f* v);
	std::tuple<float, float, float> Barycentric3D(const Vector4f& point, const Vector4f* v);

	bool InsideTriangle(const float x, const float y, const Triangle& t) const;
	void SetPixelColor(const Vector2i point, const Vector3f color);
	int GetPixelIndex(int x, int y);

	//���Բ�ֵ
	float Interpolate(float alpha, float beta, float gamma, const float& vert1, const float& vert2, const float& vert3);
	Vector2f Interpolate(float alpha, float beta, float gamma, const Vector2f& vert1, const Vector2f& vert2, const Vector2f& vert3);
	Vector3f Interpolate(float alpha, float beta, float gamma, const Vector3f& vert1, const Vector3f& vert2, const Vector3f& vert3);
	Vector4f Interpolate(float alpha, float beta, float gamma, const Vector4f& vert1, const Vector4f& vert2, const Vector4f& vert3);

	float ToRadian(float angle);

private:
	int width, height; //�ӿڿ��

	//������ɫ����ؾ���
	Matrix4f model, view, projection;
	Matrix4f mvp;
	Matrix4f viewport;

	//Buffer
	std::vector<Vector3f> frameBuffer;
	std::vector<float> depthBuffer;

	std::optional<Texture> texture;
	std::optional<Texture> bumpMap;
	std::optional<Texture> normalMap;
	Shader shader;
	Log logger;
};

