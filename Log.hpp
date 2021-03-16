#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include "global.h"
using namespace std;

class Log
{
public:
	//渲染器信息
	void RanderInfo(const int& width, const int& height)
	{
		cout << "渲染窗口大小：" << width << "×" << height << endl;
		if(MSAA4X) cout << "MSAA4X：开启" << endl;
		else cout << "MSAA4X：关闭" << endl;
		if(BLERP) cout << "贴图双线性插值：开启" << endl;
		else cout << "贴图双线性插值：关闭" << endl;
	}
	//渲染开始
	void RanderStart(const vector<Object>& objectList)
	{
		cout << endl;
		cout << "渲染开始了哦" << endl;
		cout << "此次要渲染" << objectList.size() << "个物体" << endl;
		int objIndex = 0;
		for (auto& o : objectList)
		{
			cout << "其中第" << ++objIndex << "个物体有" << o.triangles.size() << "个三角形" << endl;
		}
	}
	//打印帧信息
	void PrintFrameCount(const int& frameCount)
	{
		cout << endl;
		cout << "现在开始渲染第" << frameCount << "帧" << endl;
	}
	//出图
	void ShowImage()
	{
		cout << "登登等登，渲完辣，出图嘞" << endl;
	}
	//模型加载成功
	void ModelLoadSuccess(const string name)
	{
		cout << "模型" << name << "加载成功" << endl;
	}
	//模型加载失败
	void ModelLoadFail()
	{
		cout << "WARNING: 模型加载失败" << endl;
		system("pause");
	}

	//贴图加载失败
	void TextureLoadFail()
	{
		cout << "WARNING: 贴图加载失败" << endl;
	}
	//无贴图
	void NullTexture()
	{
		cout << "WARNING: 贴图无效" << endl;
		//system("pause");
	}

	//顶点着色器完事r
	void VertexShaderFinish()
	{
		cout << "顶点着色器，工作完成咯！" << endl;
	}

	//三角形计数
	void FragmentCount(const int& size, const int& objIndex, const int& triIndex)
	{
		if (size <= 10)
		{
			std::cout << "第" << objIndex << "个物体的第" << triIndex << "个三角形片元渲染开始啦" << std::endl;
		}
		else if (size <= 100)
		{
			if (triIndex % 10 == 0) cout << "第" << objIndex << "个物体的第" << triIndex << "个三角形片元渲染开始啦" << endl;
		}
		else if (size <= 1000)
		{
			if (triIndex % 100 == 0) cout << "第" << objIndex << "个物体的第" << triIndex << "个三角形片元渲染开始啦" << endl;
		}
		else if (size <= 5000)
		{
			if (triIndex % 500 == 0) cout << "第" << objIndex << "个物体的第" << triIndex << "个三角形片元渲染开始啦" << endl;
		}
		else if (size <= 10000)
		{
			if (triIndex % 1000 == 0) cout << "第" << objIndex << "个物体的第" << triIndex << "个三角形片元渲染开始啦" << endl;
		}
		else if (triIndex % 5000 == 0) cout << "第" << objIndex << "个物体的第" << triIndex << "个三角形片元渲染开始啦" << endl;
	}
};

#endif // !LOG_HPP
