#ifndef SHADER
#define SHADER
#include <Eigen/Eigen>
#include "global.h"
#include "texture.hpp"
#include "Log.hpp"
using namespace Eigen;

class Shader
{
public:
	Shader()
	{
		shadingFunction = ETextureWithNormal;

		texture = nullptr;
		bumpMap = nullptr;
		normalMap = nullptr;
	}
	Vector3f Shading()
	{
		switch (shadingFunction)
		{
		case EBaseVertexColor:
			return BaseVertexColor();
			break;
		case ENormalColor:
			return NormalShader();
			break;
		case EBlinnPhongShading:
			return BlinnPhongShader();
			break;
		case ETextureShading:
			return TextureShader();
			break;
		case ETextureWithBump:
			return TextureWithBump();
			break;
		case ETextureWithNormal:
			return TextureWithNormal();
			break;
		}
	}
public:
	Vector3f BaseVertexColor()
	{
		return color;
	}

	Vector3f NormalShader()
	{
		Vector3f returnColor = (normal.normalized() + Vector3f(1.0f, 1.0f, 1.0f)) / 2.0f;
		return returnColor * 225.0f;
	}

	Vector3f BlinnPhongShader()
	{
		//�����⡢������͸߹�ϵ��
		Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
		Vector3f kd = color / 255;
		Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);
		float p = 100;

		Vector3f resultColor = { 0, 0, 0 };
		for (const auto& light : *lightList)
		{
			Vector3f diffuse, specular, ambient;
			//��Ⱦ����Դ�ľ���
			float r = (light.position.head<3>() - position).norm();
			//���շ���
			Vector3f l = (light.position.head<3>() - position).normalized();
			//�۲췽��
			Vector3f v = -position.normalized();
			//��Դ����͹۲췽��İ������
			Vector3f h = (v + l).normalized();

			//���㻷���⡢������͸߹���ɫ
			ambient = ka.cwiseProduct(light.ambientIntensity);
			diffuse = kd.cwiseProduct(light.intensity / (r * r)) * std::max(0.f, normal.dot(l));
			specular = ks.cwiseProduct(light.intensity / (r * r)) * pow(std::max(0.f, normal.dot(h)), p);

			resultColor += ambient + diffuse + specular;
		}
		return resultColor * 255.f;
	}

	Vector3f TextureShader()
	{
		Vector3f diffColor = color;
		if (texture != nullptr)
		{
			diffColor = texture->GetColor(texCoord.x(), texCoord.y());
		}
		else logger.NullTexture();

		//�����⡢������͸߹�ϵ��
		Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
		Vector3f kd = diffColor / 255;
		Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);
		float p = 100;

		Vector3f resultColor = { 0, 0, 0 };
		for (const auto& light : *lightList)
		{
			Vector3f diffuse, specular, ambient;
			//��Ⱦ����Դ�ľ���
			float r = (light.position.head<3>() - position).norm();
			//���շ���
			Vector3f l = (light.position.head<3>() - position).normalized();
			//�۲췽��
			Vector3f v = -position.normalized();
			//��Դ����͹۲췽��İ������
			Vector3f h = (v + l).normalized();

			//���㻷���⡢������͸߹���ɫ
			ambient = ka.cwiseProduct(light.ambientIntensity);
			diffuse = kd.cwiseProduct(light.intensity / (r * r)) * std::max(0.f, normal.dot(l));
			specular = ks.cwiseProduct(light.intensity / (r * r)) * pow(std::max(0.f, normal.dot(h)), p);

			resultColor += ambient + diffuse + specular;
		}
		return resultColor * 255.f;
	}

	Vector3f TextureWithBump()
	{
		//������
		if (bumpMap != nullptr)
		{
			float kh = 0.2, kn = 0.1;
			Vector3f n = normal;
			Vector3f t = Vector3f(n.x() * n.y() / sqrt(n.x() * n.x() + n.z() * n.z()), sqrt(n.x() * n.x() + n.z() * n.z()), n.z() * n.y() / sqrt(n.x() * n.x() + n.z() * n.z()));
			Vector3f b = n.cross(t);
			Matrix3f TBN;
			TBN << t.x(), b.x(), n.x(),
				t.y(), b.y(), n.y(),
				t.z(), b.z(), n.z();
			float u = texCoord.x();
			float v = texCoord.y();
			float h = bumpMap->GetHeight();
			float w = bumpMap->GetWidth();
			float dU = kh * kn * (bumpMap->GetColor(u + 1 / w, v).norm() - bumpMap->GetColor(u, v).norm());
			float dV = kh * kn * (bumpMap->GetColor(u, v + 1 / h).norm() - bumpMap->GetColor(u, v).norm());
			Vector3f ln{ -dU,-dV,1 };
			normal = (TBN * ln).normalized();
		}
		return TextureShader();
	}

	Vector3f TextureWithNormal()
	{
		//������
		if (normalMap != nullptr)
		{
			float kh = 0.2, kn = 0.1;
			Vector3f n = normal;
			Vector3f t = Vector3f(n.x() * n.y() / sqrt(n.x() * n.x() + n.z() * n.z()), sqrt(n.x() * n.x() + n.z() * n.z()), n.z() * n.y() / sqrt(n.x() * n.x() + n.z() * n.z()));
			Vector3f b = n.cross(t);
			Matrix3f TBN;
			TBN << t.x(), b.x(), n.x(),
				t.y(), b.y(), n.y(),
				t.z(), b.z(), n.z();
			float u = texCoord.x();
			float v = texCoord.y();
			Vector3f ln = ((normalMap->GetColor(u, v)) / 255.f) * 2.f - Vector3f(1, 1, 1);
			normal = (TBN * ln).normalized();
		}
		return TextureShader();
	}
public:
	void SetColor(const Vector3f& c)
	{
		color = c;
	}
	void SetNormal(const Vector4f& n)
	{
		normal = Vector3f(n.x(), n.y(), n.z());
	}
	void SetLight(const std::vector<Light>& lL)
	{
		lightList = &lL;
	}
	void SetPosition(const Vector4f& vP)
	{
		position = Vector3f(vP.x(), vP.y(), vP.z());
	}
	void SetTexCoord(const Vector2f& tC)
	{
		texCoord = tC;
	}
	void SetTexture(Texture* t)
	{
		texture = t;
	}
	void SetBumpMap(Texture* bM)
	{
		bumpMap = bM;
	}
	void SetNormalMap(Texture* nM)
	{
		normalMap = nM;
	}
private:
	Vector3f color;
	Vector3f normal;
	const std::vector<Light>* lightList;
	Vector3f position;
	Vector2f texCoord;
	Texture* texture;
	Texture* bumpMap;
	Texture* normalMap;

	ShadingFunction shadingFunction;

	Log logger;
};

#endif // !SHADER