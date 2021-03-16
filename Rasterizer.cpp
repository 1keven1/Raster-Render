#include "Rasterizer.h"

Rasterizer::Rasterizer(int w, int h) :width(w), height(h)
{
	//分配Buffer空间
	frameBuffer.resize(w * h);
	if (!MSAA4X) depthBuffer.resize(w * h);
	else depthBuffer.resize(w * h * 4);
	//生成视口变换矩阵
	viewport << width / 2, 0, 0, width / 2,
		0, height / 2, 0, height / 2,
		0, 0, 1, 0,
		0, 0, 0, 1;

	texture = std::nullopt;
	bumpMap = std::nullopt;
	normalMap = std::nullopt;
}

void Rasterizer::VertexShader(std::vector<Object>& objectList, std::vector<Light>& lightList, Camera c)
{
	//对于每个物体
	for (auto& object : objectList)
	{
		//计算MVP矩阵
		SetModelMatrix(object);
		SetViewMatrix(c);
		SetProjectionMatrix(c);
		mvp = projection * view * model;
		//对于物体中的每个三角形
		for (Triangle& t : object.triangles)
		{
			//对于三角形的每个顶点
			for (auto& vec : t.v)
			{
				//变换
				vec = mvp * vec;
				vec = viewport * vec;
				//齐次坐标归一化
				vec.x() /= vec.w();
				vec.y() /= vec.w();
				vec.z() /= vec.w();
				vec.w() /= vec.w();
			}
			//对于三角形的每个法线
			//Eigen::Matrix4f inv_trans = (view * model).inverse().transpose();
			for (auto& nor : t.normal)
			{
				nor = view * model * nor;
				nor = nor.normalized();
			}
		}
	}
	//对于每个光源
	for (auto& l : lightList)
	{
		//变换
		l.position = view * l.position;

		//齐次坐标归一化
		l.position.x() /= l.position.w();
		l.position.y() /= l.position.w();
		l.position.z() /= l.position.w();
		l.position.w() /= l.position.w();
	}
	logger.VertexShaderFinish();
	return;
}

void Rasterizer::FragmentShader(const std::vector<Object>& objectList, const std::vector<Light>& lightList)
{
	shader.SetTexture(texture ? &*texture : nullptr);
	shader.SetBumpMap(bumpMap ? &*bumpMap : nullptr);
	shader.SetNormalMap(normalMap ? &*normalMap : nullptr);
	int objIndex = 0;
	for (const auto& object : objectList)
	{
		int triIndex = 0;
		objIndex++;
		for (const auto& t : object.triangles)
		{
			triIndex++;
			logger.FragmentCount(object.triangles.size(), objIndex, triIndex);
			//绘制bounding box
			float minXf, maxXf, minYf, maxYf;
			minXf = width;
			maxXf = 0;
			minYf = height;
			maxYf = 0;
			for (const auto& ver : t.v)
			{
				if (ver.x() <= minXf) minXf = ver.x();
				if (ver.x() >= maxXf) maxXf = ver.x();
				if (ver.y() <= minYf) minYf = ver.y();
				if (ver.y() >= maxYf) maxYf = ver.y();
			}
			if (minXf >= width || maxXf <= 0 || minYf >= height || maxYf <= 0) continue;
			if (minXf < 0) minXf = 0;
			if (maxXf > width) maxXf = width;
			if (minYf < 0) minYf = 0;
			if (maxYf > height) maxYf = height;
			//取整
			int minX, maxX, minY, maxY;
			minX = floor(minXf);
			maxX = ceil(maxXf);
			minY = floor(minYf);
			maxY = ceil(maxYf);
			//对bounding box中的每一个像素
			for (int y = minY; y < maxY; y++)
			{
				for (int x = minX; x < maxX; x++)
				{
					//SetPixelColor(Vector2i(x, y), Vector3f(255,0,0));
					//判断像素中心是否在三角形内
					if (!MSAA4X)
					{
						if (InsideTriangle((float)x + 0.5f, (float)y + 0.5f, t))
						{
							//在的话计算2D重心坐标
							float alpha2D, beta2D, gamma2D;
							std::tie(alpha2D, beta2D, gamma2D) = Barycentric2D((float)x + 0.5f, (float)y + 0.5f, t.v);
							float w2D = alpha2D + beta2D + gamma2D;
							float interpolateZ2D = Interpolate(alpha2D, beta2D, gamma2D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ2D /= w2D;

							//像素中点的坐标
							Vector4f p{ (float)x + 0.5f,(float)y + 0.5f,interpolateZ2D,1.f };
							//将三角形和监测点坐标变换回原来的形态
							p = mvp.inverse() * viewport.inverse() * p;
							Vector4f v[3] =
							{
								mvp.inverse() * viewport.inverse() * t.v[0],
								mvp.inverse() * viewport.inverse() * t.v[1],
								mvp.inverse() * viewport.inverse() * t.v[2]
							};
							//计算3D重心坐标
							float alpha3D, beta3D, gamma3D;
							std::tie(alpha3D, beta3D, gamma3D) = Barycentric3D(p, v);
							float interpolateZ3D = Interpolate(alpha3D, beta3D, gamma3D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ3D *= -1;

							//位置
							Vector4f position[3] =
							{
								projection.inverse() * viewport.inverse() * t.v[0],
								projection.inverse() * viewport.inverse() * t.v[1],
								projection.inverse() * viewport.inverse() * t.v[2],
							};

							//判断深度值
							if (depthBuffer[GetPixelIndex(x, y)] > interpolateZ3D)
							{
								//深度更近的话插值出各种值，然后更新深度信息
								auto interpolateColor = Interpolate(alpha3D, beta3D, gamma3D, t.color[0], t.color[1], t.color[2]);
								auto interpolateNormal = Interpolate(alpha3D, beta3D, gamma3D, t.normal[0], t.normal[1], t.normal[2]).normalized();
								if (interpolateNormal.head<3>().dot(Vector3f(0,0,1)) <= 0) continue;
								auto interpolatePosition = Interpolate(alpha3D, beta3D, gamma3D, position[0], position[1], position[2]);
								auto interpolateTexCoord = Interpolate(alpha3D, beta3D, gamma3D, t.texCoord[0], t.texCoord[1], t.texCoord[2]);
								//传入Shader需要的信息
								shader.SetColor(interpolateColor);
								shader.SetNormal(interpolateNormal);
								shader.SetLight(lightList);
								shader.SetPosition(interpolatePosition);
								shader.SetTexCoord(interpolateTexCoord);

								Vector3f pixelColor = shader.Shading();
								SetPixelColor(Vector2i(x, y), pixelColor);
								depthBuffer[GetPixelIndex(x, y)] = interpolateZ3D;
							}
						}
					}
					//MSAA4X
					else //MSAA4X
					{
						//对像素里的格子进行编号：	1 2
						//						3 4
						int depthIndex = GetPixelIndex(x, y) * 4; //Depth Buffer索引值
						float alpha2D, beta2D, gamma2D;
						float alpha3D, beta3D, gamma3D;
						//初始化储存颜色的数组为Frame Buffer原先的值
						Vector3f color[4] =
						{
							frameBuffer[GetPixelIndex(x,y)],
							frameBuffer[GetPixelIndex(x,y)],
							frameBuffer[GetPixelIndex(x,y)],
							frameBuffer[GetPixelIndex(x,y)],
						};
						//计算此三角形变换后的坐标然后储存起来
						Vector4f v[3] =
						{
							mvp.inverse() * viewport.inverse() * t.v[0],
							mvp.inverse() * viewport.inverse() * t.v[1],
							mvp.inverse() * viewport.inverse() * t.v[2]
						};
						//位置
						Vector4f position[3] =
						{
							projection.inverse() * viewport.inverse() * t.v[0],
							projection.inverse() * viewport.inverse() * t.v[1],
							projection.inverse() * viewport.inverse() * t.v[2],
						};
						//像素1
						if (InsideTriangle((float)x + 0.25f, (float)y + 0.75f, t))
						{
							//求2d重心坐标，并差值出采样点坐标
							std::tie(alpha2D, beta2D, gamma2D) = Barycentric2D((float)x + 0.5f, (float)y + 0.5f, t.v);
							float w2D = alpha2D + beta2D + gamma2D;
							float interpolateZ2D = Interpolate(alpha2D, beta2D, gamma2D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ2D /= w2D;

							Vector4f p{ (float)x + 0.25f,(float)y + 0.75f,interpolateZ2D,1.f };
							p = mvp.inverse() * viewport.inverse() * p;
							//计算3D重心坐标
							std::tie(alpha3D, beta3D, gamma3D) = Barycentric3D(p, v);
							//计算正确的深度值
							float interpolateZ3D = Interpolate(alpha3D, beta3D, gamma3D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ3D *= -1;
							//如果深度值更小的话
							if (interpolateZ3D < depthBuffer[depthIndex])
							{
								//深度更近的话插值出各种值，然后更新深度信息
								auto interpolateColor = Interpolate(alpha3D, beta3D, gamma3D, t.color[0], t.color[1], t.color[2]);
								auto interpolateNormal = Interpolate(alpha3D, beta3D, gamma3D, t.normal[0], t.normal[1], t.normal[2]).normalized();
								if (interpolateNormal.head<3>().dot(Vector3f(0, 0, 1)) <= 0) continue;
								auto interpolatePosition = Interpolate(alpha3D, beta3D, gamma3D, position[0], position[1], position[2]);
								auto interpolateTexCoord = Interpolate(alpha3D, beta3D, gamma3D, t.texCoord[0], t.texCoord[1], t.texCoord[2]);

								//传入Shader需要的信息
								shader.SetColor(interpolateColor);
								shader.SetNormal(interpolateNormal);
								shader.SetLight(lightList);
								shader.SetPosition(interpolatePosition);
								shader.SetTexCoord(interpolateTexCoord);
								//储存颜色到数组
								color[0] = shader.Shading();
								//更新深度值
								depthBuffer[depthIndex] = interpolateZ3D;
							}
							//下面的每个像素均类似
						}
						//像素2
						if (InsideTriangle((float)x + 0.75f, (float)y + 0.75f, t))
						{
							std::tie(alpha2D, beta2D, gamma2D) = Barycentric2D((float)x + 0.5f, (float)y + 0.5f, t.v);
							float w2D = alpha2D + beta2D + gamma2D;
							float interpolateZ2D = Interpolate(alpha2D, beta2D, gamma2D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ2D /= w2D;

							Vector4f p{ (float)x + 0.75f,(float)y + 0.75f,interpolateZ2D,1.f };
							p = mvp.inverse() * viewport.inverse() * p;
							std::tie(alpha3D, beta3D, gamma3D) = Barycentric3D(p, v);
							float interpolateZ3D = Interpolate(alpha3D, beta3D, gamma3D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ3D *= -1;

							if (interpolateZ3D < depthBuffer[depthIndex + 1])
							{
								//深度更近的话插值出各种值，然后更新深度信息
								auto interpolateColor = Interpolate(alpha3D, beta3D, gamma3D, t.color[0], t.color[1], t.color[2]);
								auto interpolateNormal = Interpolate(alpha3D, beta3D, gamma3D, t.normal[0], t.normal[1], t.normal[2]).normalized();
								if (interpolateNormal.head<3>().dot(Vector3f(0, 0, 1)) <= 0) continue;
								auto interpolatePosition = Interpolate(alpha3D, beta3D, gamma3D, position[0], position[1], position[2]);
								auto interpolateTexCoord = Interpolate(alpha3D, beta3D, gamma3D, t.texCoord[0], t.texCoord[1], t.texCoord[2]);

								//传入Shader需要的信息
								shader.SetColor(interpolateColor);
								shader.SetNormal(interpolateNormal);
								shader.SetLight(lightList);
								shader.SetPosition(interpolatePosition);
								shader.SetTexCoord(interpolateTexCoord);
								color[1] = shader.Shading();
								depthBuffer[depthIndex + 1] = interpolateZ3D;
							}
						}
						//像素3
						if (InsideTriangle((float)x + 0.25f, (float)y + 0.25f, t))
						{
							std::tie(alpha2D, beta2D, gamma2D) = Barycentric2D((float)x + 0.5f, (float)y + 0.5f, t.v);
							float w2D = alpha2D + beta2D + gamma2D;
							float interpolateZ2D = Interpolate(alpha2D, beta2D, gamma2D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ2D /= w2D;

							Vector4f p{ (float)x + 0.25f,(float)y + 0.25f,interpolateZ2D,1.f };
							p = mvp.inverse() * viewport.inverse() * p;
							std::tie(alpha3D, beta3D, gamma3D) = Barycentric3D(p, v);
							float interpolateZ3D = Interpolate(alpha3D, beta3D, gamma3D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ3D *= -1;

							if (interpolateZ3D < depthBuffer[depthIndex + 2])
							{
								//深度更近的话插值出各种值，然后更新深度信息
								auto interpolateColor = Interpolate(alpha3D, beta3D, gamma3D, t.color[0], t.color[1], t.color[2]);
								auto interpolateNormal = Interpolate(alpha3D, beta3D, gamma3D, t.normal[0], t.normal[1], t.normal[2]).normalized();
								auto interpolatePosition = Interpolate(alpha3D, beta3D, gamma3D, position[0], position[1], position[2]);
								if (interpolateNormal.head<3>().dot(Vector3f(0, 0, 1)) <= 0) continue;
								auto interpolateTexCoord = Interpolate(alpha3D, beta3D, gamma3D, t.texCoord[0], t.texCoord[1], t.texCoord[2]);

								//传入Shader需要的信息
								shader.SetColor(interpolateColor);
								shader.SetNormal(interpolateNormal);
								shader.SetLight(lightList);
								shader.SetPosition(interpolatePosition);
								shader.SetTexCoord(interpolateTexCoord);
								color[2] = shader.Shading();
								depthBuffer[depthIndex + 2] = interpolateZ3D;
							}
						}
						//像素4
						if (InsideTriangle((float)x + 0.75f, (float)y + 0.25f, t))
						{
							std::tie(alpha2D, beta2D, gamma2D) = Barycentric2D((float)x + 0.5f, (float)y + 0.5f, t.v);
							float w2D = alpha2D + beta2D + gamma2D;
							float interpolateZ2D = Interpolate(alpha2D, beta2D, gamma2D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ2D /= w2D;

							Vector4f p{ (float)x + 0.75f,(float)y + 0.25f,interpolateZ2D,1.f };
							p = mvp.inverse() * viewport.inverse() * p;
							std::tie(alpha3D, beta3D, gamma3D) = Barycentric3D(p, v);
							float interpolateZ3D = Interpolate(alpha3D, beta3D, gamma3D, t.v[0].z(), t.v[1].z(), t.v[2].z());
							interpolateZ3D *= -1;

							if (interpolateZ3D < depthBuffer[depthIndex + 3])
							{
								//深度更近的话插值出各种值，然后更新深度信息
								auto interpolateColor = Interpolate(alpha3D, beta3D, gamma3D, t.color[0], t.color[1], t.color[2]);
								auto interpolateNormal = Interpolate(alpha3D, beta3D, gamma3D, t.normal[0], t.normal[1], t.normal[2]).normalized();
								if (interpolateNormal.head<3>().dot(Vector3f(0, 0, 1)) <= 0) continue;
								auto interpolatePosition = Interpolate(alpha3D, beta3D, gamma3D, position[0], position[1], position[2]);
								auto interpolateTexCoord = Interpolate(alpha3D, beta3D, gamma3D, t.texCoord[0], t.texCoord[1], t.texCoord[2]);

								//传入Shader需要的信息
								shader.SetColor(interpolateColor);
								shader.SetNormal(interpolateNormal);
								shader.SetLight(lightList);
								shader.SetPosition(interpolatePosition);
								shader.SetTexCoord(interpolateTexCoord);
								color[3] = shader.Shading();
								depthBuffer[depthIndex + 3] = interpolateZ3D;
							}
						}
						//求平均
						Vector3f finalColor = 0.25f * color[0] + 0.25f * color[1] + 0.25f * color[2] + 0.25f * color[3];
						SetPixelColor(Vector2i(x, y), finalColor);
					} //
				}
			}
		}
	}
}

void Rasterizer::Clear()
{
	std::fill(frameBuffer.begin(), frameBuffer.end(), Vector3f(0, 0, 0));
	std::fill(depthBuffer.begin(), depthBuffer.end(), std::numeric_limits<float>::infinity());
}

void Rasterizer::SetTexture(Texture tex)
{
	texture = tex;
}

void Rasterizer::SetBumpMap(Texture bTex)
{
	bumpMap = bTex;
}

void Rasterizer::SetNormalMap(Texture nTex)
{
	normalMap = nTex;
}

std::vector<Vector3f>& Rasterizer::GetFrameBuffer()
{
	return frameBuffer;
}

void Rasterizer::SetModelMatrix(const Object& o)
{
	Matrix4f rX, rY, rZ; //XYZ轴的旋转矩阵
	float radX, radY, radZ; //xyz轴的旋转弧度
	Matrix4f scale; //缩放矩阵
	Matrix4f move; //位移矩阵

	radX = ToRadian(o.rotation.x());
	radY = ToRadian(o.rotation.y());
	radZ = ToRadian(o.rotation.z());
	rX << 1, 0, 0, 0,
		0, cos(radX), -sin(radX), 0,
		0, sin(radX), cos(radX), 0,
		0, 0, 0, 1;
	rY << cos(radY), 0, sin(radY), 0,
		0, 1, 0, 0,
		-sin(radY), 0, cos(radY), 0,
		0, 0, 0, 1;
	rZ << cos(radZ), -sin(radZ), 0, 0,
		sin(radZ), cos(radZ), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1;
	scale << o.scale.x(), 0, 0, 0,
		0, o.scale.y(), 0, 0,
		0, 0, o.scale.z(), 0,
		0, 0, 0, 1;
	move << 1, 0, 0, o.position.x(),
		0, 1, 0, o.position.y(),
		0, 0, 1, o.position.z(),
		0, 0, 0, 1;
	//矩阵左乘计算出模型矩阵
	model = move * scale * rZ * rY * rX;
}

void Rasterizer::SetViewMatrix(const Camera& c)
{
	//将摄像机移动到原点，然后使用旋转矩阵的正交性让摄像机摆正
	Matrix4f t; //移动矩阵
	Vector3f cX; //摄像机的x轴
	Matrix4f r; //旋转矩阵的旋转矩阵

	t << 1, 0, 0, -c.position.x(),
		0, 1, 0, -c.position.y(),
		0, 0, 1, -c.position.z(),
		0, 0, 0, 1;
	cX = c.g.cross(c.t);
	r << cX.x(), cX.y(), cX.z(), 0,
		c.t.x(), c.t.y(), c.t.z(), 0,
		-c.g.x(), -c.g.y(), -c.g.z(), 0,
		0, 0, 0, 1;
	//矩阵左乘计算出视图矩阵
	view = r * t;
}

void Rasterizer::SetProjectionMatrix(const Camera& c)
{
	//透视投影矩阵
	Matrix4f p2o; //将梯台状透视视锥挤成长方体正交投影
	Matrix4f orthoTrans, orthoScale, ortho; //正交投影矩阵的平移和缩放分解
	float t, r; //近平面的上边界和右边界
	float radFov; //视野的弧度制

	radFov = ToRadian(c.fov);
	t = tan(radFov / 2) * c.nNear;
	r = c.aspectRatio * t;

	p2o << c.nNear, 0, 0, 0,
		0, c.nNear, 0, 0,
		0, 0, c.nFar + c.nNear, c.nNear* c.nFar,
		0, 0, -1, 0;
	orthoTrans << 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, (c.nNear + c.nFar) / 2,
		0, 0, 0, 1;
	orthoScale << 1 / r, 0, 0, 0,
		0, 1 / t, 0, 0,
		0, 0, 2 / (c.nFar - c.nNear), 0,
		0, 0, 0, 1;
	ortho = orthoScale * orthoTrans;
	//矩阵左乘计算出透视投影矩阵
	projection = ortho * p2o;
}

std::tuple<float, float, float> Rasterizer::Barycentric2D(float x, float y, const Vector4f* v)
{
	float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) / (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
	float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) / (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
	float c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) / (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() - v[1].x() * v[0].y());
	return { c1,c2,c3 };
}

std::tuple<float, float, float> Rasterizer::Barycentric3D(const Vector4f& point, const Vector4f* v)
{
	//计算法线方向
	Vector3f a, b, c, p;
	a = Vector3f(v[0].x(), v[0].y(), v[0].z());
	b = Vector3f(v[1].x(), v[1].y(), v[1].z());
	c = Vector3f(v[2].x(), v[2].y(), v[2].z());
	p = Vector3f(point.x(), point.y(), point.z());

	Vector3f n = (b - a).cross(c - a);
	Vector3f na = (c - b).cross(p - b);
	Vector3f nb = (a - c).cross(p - c);
	Vector3f nc = (b - a).cross(p - a);
	float c1 = n.dot(na) / (n.norm() * n.norm());
	float c2 = n.dot(nb) / (n.norm() * n.norm());
	float c3 = n.dot(nc) / (n.norm() * n.norm());
	return { c1,c2,c3 };
}

void Rasterizer::SetPixelColor(const Vector2i point, const Vector3f color)
{
	int ind = (height - point.y() - 1) * width + point.x();
	frameBuffer[ind] = color;
}

int Rasterizer::GetPixelIndex(int x, int y)
{
	return (height - y - 1) * width + x;
}

Vector3f Rasterizer::Interpolate(float alpha, float beta, float gamma, const Vector3f& vert1, const Vector3f& vert2, const Vector3f& vert3)
{
	return (alpha * vert1 + beta * vert2 + gamma * vert3);
}

Vector4f Rasterizer::Interpolate(float alpha, float beta, float gamma, const Vector4f& vert1, const Vector4f& vert2, const Vector4f& vert3)
{
	return (alpha * vert1 + beta * vert2 + gamma * vert3);
}

float Rasterizer::Interpolate(float alpha, float beta, float gamma, const float& vert1, const float& vert2, const float& vert3)
{
	return (alpha * vert1 + beta * vert2 + gamma * vert3);
}

Vector2f Rasterizer::Interpolate(float alpha, float beta, float gamma, const Vector2f& vert1, const Vector2f& vert2, const Vector2f& vert3)
{
	return (alpha * vert1 + beta * vert2 + gamma * vert3);
}

bool Rasterizer::InsideTriangle(const float x, const float y, const Triangle& t)
{
	Vector3f v[3] =
	{
		Vector3f(t.v[0].x(),t.v[0].y(),1.f),
		Vector3f(t.v[1].x(),t.v[1].y(),1.f),
		Vector3f(t.v[2].x(),t.v[2].y(),1.f)
	};
	Vector3f p(x, y, 1);
	//三条边的向量
	Vector3f side1, side2, side3;
	side1 = v[1] - v[0];
	side2 = v[2] - v[1];
	side3 = v[0] - v[2];
	//顶点到点的向量
	Vector3f p1, p2, p3;
	p1 = p - v[0];
	p2 = p - v[1];
	p3 = p - v[2];
	//叉乘
	Vector3f cross1, cross2, cross3;
	cross1 = p1.cross(side1);
	cross2 = p2.cross(side2);
	cross3 = p3.cross(side3);
	//判断是否同号
	if ((cross1.z() > 0 && cross2.z() > 0 && cross3.z() > 0) || (cross1.z() < 0 && cross2.z() < 0 && cross3.z() < 0))
	{
		return true;
	}
	else return false;
}

float Rasterizer::ToRadian(float angle)
{
	return (angle / 180) * PI;
}
