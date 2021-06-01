/*
* Made by Suzhi_Zhang on 2021-02-14
* https://1keven1.github.io/
* 
* 使用了Eigen库和OpenCV
* 简单实现光栅化渲染器的基本功能
* （MVP变换，Blinn-Phong光照模型，贴图，双线性插值，法线贴图）
* 制作介绍：
* https://1keven1.github.io/2021/02/14/%E3%80%90C-%E3%80%91%E6%9A%B4%E5%8A%9B%E5%85%89%E6%A0%85%E6%B8%B2%E6%9F%93%E5%99%A8%EF%BC%88%E4%B8%8D%E4%BD%BF%E7%94%A8%E4%BB%BB%E4%BD%95%E5%9B%BE%E5%BD%A2%E5%AD%A6API%EF%BC%89/
*/
#include <iostream>
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include "global.h"
#include "Rasterizer.h"
#include "OBJ_Loader.h"
#include "Log.hpp"
#include "texture.hpp"

Log logger;
constexpr int width = 700;
constexpr int height = 700;
std::vector<Object> objectList;
std::vector<Light> lightList;
Camera camera;
int frameCount = 0;
int key = 0;
float angle = 0;
float scale = 1;

void SetTriangles()
{
	//爱心
	Object o1;
	Triangle t1, t2, t3, t4, t5;
	t1.SetVertex(Vector3f(0, -1, 0), Vector3f(3, 3, 0), Vector3f(-3, 3, 0));
	t1.SetColor(Vector3f(225, 0, 0), Vector3f(255, 0, 100), Vector3f(255, 0, 100));
	o1.triangles.push_back(t1);
	t2.SetVertex(Vector3f(0, 3, 0), Vector3f(3, 3, 0), Vector3f(1.5, 4, 0));
	t2.SetColor(Vector3f(255, 0, 100), Vector3f(255, 0, 100), Vector3f(255, 0, 255));
	o1.triangles.push_back(t2);
	t3.SetVertex(Vector3f(0, 3, 0), Vector3f(-1.5, 4, 0), Vector3f(-3, 3, 0));
	t3.SetColor(Vector3f(255, 0, 100), Vector3f(255, 0, 255), Vector3f(255, 0, 100));
	o1.triangles.push_back(t3);
	t4.SetVertex(Vector3f(0, -1, 0), Vector3f(2.5, 1.5, 0), Vector3f(3, 3, 0));
	t4.SetColor(Vector3f(255, 0, 0), Vector3f(255, 0, 0), Vector3f(255, 0, 100));
	o1.triangles.push_back(t4);
	t5.SetVertex(Vector3f(0, -1, 0), Vector3f(-3, 3, 0), Vector3f(-2.5, 1.5, 0));
	t5.SetColor(Vector3f(255, 0, 0), Vector3f(255, 0, 100), Vector3f(255, 0, 0));
	o1.triangles.push_back(t5);

	o1.position = Vector3f(0, 0, -2);
	o1.rotation = Vector3f(0, 0, 0);
	o1.scale = Vector3f(1, 1, 1);
	objectList.push_back(o1);

	/*
	//o2
	Object o2;
	Triangle t2;
	t2.SetVertex(Vector3f(2, 1, 0), Vector3f(-2, 4, -5), Vector3f(-1, -3, -5));
	//t2.SetColor(Vector3f(100, 100, 100), Vector3f(100, 100, 100), Vector3f(100, 100, 100));
	o2.triangles.push_back(t2);
	o2.position = Vector3f(0, 0, 0);
	o2.rotation = Vector3f(0, 0, 0);
	o2.scale = Vector3f(1, 1, 1);
	//objectList.push_back(o2);
	*/
}

void SetModel()
{
	objl::Loader loader;
	std::string objPath = "./";
	std::string objName = "Model.obj";
	bool bLoad = loader.LoadFile(objPath + objName);
	if (bLoad)
	{
		for (const auto& mesh : loader.LoadedMeshes)
		{
			Object* o = new Object();
			for (int i = 0; i < mesh.Vertices.size(); i += 3)
			{
				Triangle* t = new Triangle();
				for (int j = 0; j < 3; j++)
				{
					t->SetVertex(j, Vector3f(mesh.Vertices[i + j].Position.X, mesh.Vertices[i + j].Position.Y, mesh.Vertices[i + j].Position.Z));
					t->SetColor(j, Vector3f(255, 255, 255));
					t->SetNormal(j, Vector3f(mesh.Vertices[i + j].Normal.X, mesh.Vertices[i + j].Normal.Y, mesh.Vertices[i + j].Normal.Z));
					t->SetTexCoord(j, Vector2f(mesh.Vertices[i + j].TextureCoordinate.X, mesh.Vertices[i + j].TextureCoordinate.Y));
				}
				o->triangles.push_back(*t);
			}
			o->position = Vector3f(0, 0, 0);
			o->rotation = Vector3f(0, 135, 0);
			angle = o->rotation.y();
			o->scale = Vector3f(2, 2, 2);
			scale = 2;
			objectList.push_back(*o);
			logger.ModelLoadSuccess(objName);
		}
	}
	else logger.ModelLoadFail();
}

void SetLight()
{
	Light* l;
	l = new Light({ 20,20,20,1 }, { 700,700,700 }, { 10,10,10 });
	lightList.push_back(*l);
	l = new Light({ -20,20,0,1 }, { 500,500,500 }, { 5,5,5 });
	lightList.push_back(*l);
}

void SetCamera()
{
	camera.position = Vector3f(0, 0, 10);
	camera.g = Vector3f(0, 0, -1).normalized();
	camera.t = Vector3f(0, 1, 0).normalized();
	camera.fov = 45.f;
	camera.nNear = 0.1f;
	camera.nFar = 50.f;
	camera.aspectRatio = width / height;
}

void SetTexture(Rasterizer& r)
{
	std::string texPath = "./";
	std::string texName = "Texture.png";
	// std::string bumpName = "BumpTest01.png";
	std::string normalName = "Normal.png";
	r.SetTexture(texPath + texName);
	// r.SetBumpMap(texPath + bumpName);
	r.SetNormalMap(texPath + normalName);
}

int main()
{
	//SetTriangles(); //输入三角形
	SetModel();
	SetLight();
	SetCamera(); //设置相机

	//初始化光栅化器
	Rasterizer r(width, height);
	SetTexture(r);

	// 渲染循环
	do
	{
		logger.RanderInfo(width, height);
		logger.RanderStart(objectList);
		logger.PrintFrameCount(++frameCount);

		objectList[0].rotation = Vector3f(0, angle, 0);
		objectList[0].scale = Vector3f(scale, scale, scale);

		r.Clear();
		//复制一份出来供光栅化器处理
		std::vector<Object> oList = objectList;
		std::vector<Light> lList = lightList;

		//光栅化
		r.VertexShader(oList, lList, camera);
		r.FragmentShader(oList, lList);

		//OpenCV: 显示
		cv::Mat image(height, width, CV_32FC3, r.GetFrameBuffer().data());
		image.convertTo(image, CV_8UC3, 1.0f);
		cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
		logger.ShowImage();
		cv::imshow("OpenCV", image);
		//cv::imwrite("spotFabric.png", image);
		key = cv::waitKey(10);

		// 一些键盘操作
		if (key == 'a') angle += -10;
		if (key == 'd') angle += 10;
		if (key == 'w') scale += 0.2;
		if (key == 's') scale += -0.2;
		if (scale > 5) scale = 5;
		if (scale < 0.3) scale = 0.3;
		
		// 清屏
		system("cls");
	} while (key != 27);

	return 0;
}