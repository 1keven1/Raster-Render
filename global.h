#pragma once
//这里用来定义一些全局需要用到的玩意r
#include <Eigen/Eigen>
#include "Triangle.hpp"

constexpr double PI = 3.1415926;
constexpr bool MSAA4X = false;
constexpr bool BLERP = true;

struct Camera
{
	Vector3f position;	//相机坐标
	Vector3f g;			//观看方向，单位
	Vector3f t;			//向上向量，单位
	float fov;			//视野（高）
	float nNear;		//近平面
	float nFar;			//远平面
	float aspectRatio;	//屏幕宽高比
};

struct Object
{
	std::vector<Triangle> triangles;	//三角形
	Vector3f position;					//位置
	Vector3f rotation;					//旋转
	Vector3f scale;						//缩放
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
	EBaseVertexColor,	// 顶点色
	ENormalColor,		// 法线颜色
	EBlinnPhongShading, // Blinn-Phong着色
	ETextureShading,	// 贴图着色
	ETextureWithBump,	// 高度图着色
	ETextureWithNormal	// 法线贴图着色
};