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
	Rasterizer(int w, int h); //构造函数
	void VertexShader(std::vector<Object>& objectList, std::vector<Light>& lightList, Camera c); //顶点着色器
	void FragmentShader(const std::vector<Object>& objectList, const std::vector<Light>& lightList); //片元着色器
	void Clear();

	//设置各种贴图
	void SetTexture(Texture tex);
	void SetBumpMap(Texture bTex);
	void SetNormalMap(Texture nTex);

	std::vector<Vector3f>& GetFrameBuffer(); //返回frame buffer

private:
	//计算旋转矩阵们
	void SetModelMatrix(const Object& o);
	void SetViewMatrix(const Camera& c);
	void SetProjectionMatrix(const Camera& c);
	//计算三角形重心坐标
	std::tuple<float, float, float> Barycentric2D(float x, float y, const Vector4f* v);
	std::tuple<float, float, float> Barycentric3D(const Vector4f& point, const Vector4f* v);

	bool InsideTriangle(const float x, const float y, const Triangle& t) const;
	void SetPixelColor(const Vector2i point, const Vector3f color);
	int GetPixelIndex(int x, int y);

	//线性插值
	float Interpolate(float alpha, float beta, float gamma, const float& vert1, const float& vert2, const float& vert3);
	Vector2f Interpolate(float alpha, float beta, float gamma, const Vector2f& vert1, const Vector2f& vert2, const Vector2f& vert3);
	Vector3f Interpolate(float alpha, float beta, float gamma, const Vector3f& vert1, const Vector3f& vert2, const Vector3f& vert3);
	Vector4f Interpolate(float alpha, float beta, float gamma, const Vector4f& vert1, const Vector4f& vert2, const Vector4f& vert3);

	float ToRadian(float angle);

private:
	int width, height; //视口宽高

	//顶点着色器相关矩阵
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

