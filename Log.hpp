#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include "global.h"
using namespace std;

class Log
{
public:
	//��Ⱦ����Ϣ
	void RanderInfo(const int& width, const int& height)
	{
		cout << "��Ⱦ���ڴ�С��" << width << "��" << height << endl;
		if(MSAA4X) cout << "MSAA4X������" << endl;
		else cout << "MSAA4X���ر�" << endl;
		if(BLERP) cout << "��ͼ˫���Բ�ֵ������" << endl;
		else cout << "��ͼ˫���Բ�ֵ���ر�" << endl;
	}
	//��Ⱦ��ʼ
	void RanderStart(const vector<Object>& objectList)
	{
		cout << endl;
		cout << "��Ⱦ��ʼ��Ŷ" << endl;
		cout << "�˴�Ҫ��Ⱦ" << objectList.size() << "������" << endl;
		int objIndex = 0;
		for (auto& o : objectList)
		{
			cout << "���е�" << ++objIndex << "��������" << o.triangles.size() << "��������" << endl;
		}
	}
	//��ӡ֡��Ϣ
	void PrintFrameCount(const int& frameCount)
	{
		cout << endl;
		cout << "���ڿ�ʼ��Ⱦ��" << frameCount << "֡" << endl;
	}
	//��ͼ
	void ShowImage()
	{
		cout << "�ǵǵȵǣ�����������ͼ��" << endl;
	}
	//ģ�ͼ��سɹ�
	void ModelLoadSuccess(const string name)
	{
		cout << "ģ��" << name << "���سɹ�" << endl;
	}
	//ģ�ͼ���ʧ��
	void ModelLoadFail()
	{
		cout << "WARNING: ģ�ͼ���ʧ��" << endl;
		system("pause");
	}

	//��ͼ����ʧ��
	void TextureLoadFail()
	{
		cout << "WARNING: ��ͼ����ʧ��" << endl;
	}
	//����ͼ
	void NullTexture()
	{
		cout << "WARNING: ��ͼ��Ч" << endl;
		//system("pause");
	}

	//������ɫ������r
	void VertexShaderFinish()
	{
		cout << "������ɫ����������ɿ���" << endl;
	}

	//�����μ���
	void FragmentCount(const int& size, const int& objIndex, const int& triIndex)
	{
		if (size <= 10)
		{
			std::cout << "��" << objIndex << "������ĵ�" << triIndex << "��������ƬԪ��Ⱦ��ʼ��" << std::endl;
		}
		else if (size <= 100)
		{
			if (triIndex % 10 == 0) cout << "��" << objIndex << "������ĵ�" << triIndex << "��������ƬԪ��Ⱦ��ʼ��" << endl;
		}
		else if (size <= 1000)
		{
			if (triIndex % 100 == 0) cout << "��" << objIndex << "������ĵ�" << triIndex << "��������ƬԪ��Ⱦ��ʼ��" << endl;
		}
		else if (size <= 5000)
		{
			if (triIndex % 500 == 0) cout << "��" << objIndex << "������ĵ�" << triIndex << "��������ƬԪ��Ⱦ��ʼ��" << endl;
		}
		else if (size <= 10000)
		{
			if (triIndex % 1000 == 0) cout << "��" << objIndex << "������ĵ�" << triIndex << "��������ƬԪ��Ⱦ��ʼ��" << endl;
		}
		else if (triIndex % 5000 == 0) cout << "��" << objIndex << "������ĵ�" << triIndex << "��������ƬԪ��Ⱦ��ʼ��" << endl;
	}
};

#endif // !LOG_HPP
