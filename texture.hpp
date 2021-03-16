#ifndef TEXTURE_H
#define TEXTURE_H
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include "Log.hpp"
using namespace Eigen;

class Texture
{
public:
	Texture(std::string name)
	{
		texture = cv::imread(name);
		cv::cvtColor(texture, texture, cv::COLOR_RGB2BGR);
		width = texture.cols;
		height = texture.rows;
		if (width <= 0 || height <= 0) logger.TextureLoadFail();
	}

	Vector3f GetColor(float u, float v)
	{
		u = u - floor(u);
		v = v - floor(v);
		//因为从0开始，所以长宽需要－1
		float uT = u * (width - 1);
		float vT = (1 - v) * (height - 1);
		cv::Vec3b color;
		if (BLERP)//双线性插值
		{
			//最邻近的四个像素与他们的颜色值
			int uMin = floor(uT);
			int uMax = std::min((float)(width - 1), ceil(uT));
			int vMin = floor(vT);
			int vMax = std::min((float)(height - 1), ceil(vT));
			auto u00 = texture.at<cv::Vec3b>(vMax, uMin);
			auto u10 = texture.at<cv::Vec3b>(vMax, uMax);
			auto u01 = texture.at<cv::Vec3b>(vMin, uMin);
			auto u11 = texture.at<cv::Vec3b>(vMin, uMax);

			//线性插值权重
			float s = uT - uMin;
			float t = vT - vMin;

			//进行线性插值
			auto u0 = u00 + s * (u10 - u00);
			auto u1 = u01 + s * (u11 - u01);
			color = u0 + t * (u1 - u0);
		}
		else
		{
			color = texture.at<cv::Vec3b>(vT, uT);
		}
		return Vector3f(color[0], color[1], color[2]);
	}

	int GetWidth() { return width; }
	int GetHeight() { return height; }
private:
	cv::Mat texture;
	int width, height;
	Log logger;
};


#endif // !TEXTURE_H
