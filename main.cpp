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
	std::string objPath = "./Models/spot/";
	std::string objName = "spot_triangulated_good.obj";
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
			o->scale = Vector3f(2, 2, 2);
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
	std::string texPath = "./Models/spot/";
	std::string texName = "denmin_fabric_02_diff_4k.png";
	std::string bumpName = "BumpTest01.png";
	std::string normalName = "denmin_fabric_02_nor_4k.png";
	r.SetTexture(texPath + texName);
	//r.SetBumpMap(texPath + bumpName);
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
	logger.RanderInfo(width, height);
	SetTexture(r);

	//打印渲染信息
	logger.RanderStart(objectList);
	do
	{
		//objectList[0].rotation = Vector3f(0, angle, 0);
		logger.PrintFrameCount(++frameCount);
		r.Clear();
		//复制一份出来供光栅化器处理
		std::vector<Object> oList = objectList;
		std::vector<Light> lList = lightList;

		//光栅化
		r.VertexShader(oList, lList, camera);
		r.FragmentShader(oList, lList);

		//绘制
		cv::Mat image(height, width, CV_32FC3, r.GetFrameBuffer().data());
		image.convertTo(image, CV_8UC3, 1.0f);
		cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
		logger.ShowImage();
		cv::imshow("OpenCV", image);
		//cv::imwrite("spotFabric.png", image);
		key = cv::waitKey(0);
		if (key == 'a') angle += 10;
		if (key == 'd') angle += -10;
	} while (key != 27);

	return 0;
}