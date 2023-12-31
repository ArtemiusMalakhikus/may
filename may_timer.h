#ifndef MAY_TIMER_H
#define MAY_TIMER_H

/*
* The MIT License (MIT)
*
* Copyright (c) 2023 Malakhov Artyom
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand
* associated documentation files(the �Software�), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS
* OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CHRONO
#define NOMINMAX
#include <Windows.h>
#else
#include <chrono>
#endif


class may_timer
{
public:

	/*!
		\param [in] s �����, ������� ����� ������ � ��������.
	*/
	may_timer(double s)
	{
#ifndef CHRONO
		//��������� ������� �������� ������������������
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		//��������� ����� �� ����
		timePerFrec = 1.0 / frequency;

		//��������� ����� � ������, ������� ���� ������
		timeCount = s * frequency;

		//������������� �������� ��������
		baseCounter = 0;
		currentCounter = 0;
		previousCounter = 0;
#else
		sec = s;
		ns = static_cast<std::chrono::nanoseconds>(static_cast<long long>(sec * 1000000000));
#endif // !CHRONO
	}

	may_timer()
	{
#ifndef CHRONO
		//��������� ������� �������� ������������������
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		//��������� ����� �� ����
		timePerFrec = 1.0 / frequency;

		timeCount = 0;

		//������������� �������� ��������
		baseCounter = 0;
		currentCounter = 0;
		previousCounter = 0;
#else
		ns = static_cast<std::chrono::nanoseconds>(0);
#endif // !CHRONO
	}

	/*!
		������� ������������� �����, ������� ����� ������.
		\param [in] s �����, ������� ����� ������ � ��������.
	*/
	void SetTime(double s)
	{
#ifndef CHRONO
		//��������� ������� �������� ������������������
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		//��������� ����� � ������, ������� ���� ������
		timeCount = s * frequency;
#else
		sec = s;
		ns = static_cast<std::chrono::nanoseconds>(static_cast<long long>(sec * 1000000000));
#endif // !CHRONO
	}

	/*!
		������� ��������� ������.
	*/
	void Start()
	{
#ifndef CHRONO
		//��������� �������� �������� �������� ������������������
		QueryPerformanceCounter((LARGE_INTEGER*)&baseCounter);

		currentCounter = baseCounter;
		previousCounter = baseCounter;
#else
		baseNS = std::chrono::steady_clock::now();
		currentNS = baseNS;
		previousNS = baseNS;
#endif // !CHRONO
	}

	/*!
		������� ��������� ���������� ������� c �����������.
		\return ������, ���� ������ �����, ����� ����.
	*/
	bool CheckWithUpdate()
	{
#ifndef CHRONO
		//��������� �������� �������� �������� ������������������
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		if (currentCounter - previousCounter >= timeCount)
		{
			previousCounter = currentCounter;
			return true;
		}
		return false;
#else
		currentNS = std::chrono::steady_clock::now();

		if (currentNS - previousNS >= ns)
		{
			previousNS = currentNS;
			return true;
		}
		return false;
#endif // !CHRONO
	}

	/*!
		������� ��������� ���������� ������� c �����������.
		���������� ������� ����� ������� � ���������� ��������.
		\param [out] time ������ �� ������, ��� ������ �������.
		\return ������, ���� ������ �����, ����� ����.
	*/
	bool CheckWithUpdate(float& time)
	{
#ifndef CHRONO
		//��������� �������� �������� �������� ������������������
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		unsigned long long count = currentCounter - previousCounter;
		time = timePerFrec * count;

		if (count >= timeCount)
		{
			previousCounter = currentCounter;
			return true;
		}
		return false;
#else
		currentNS = std::chrono::steady_clock::now();

		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::time_point(currentNS - previousNS);
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count() * 0.000000001;

		if (t.time_since_epoch() >= ns)
		{
			previousNS = currentNS;
			return true;
		}
		return false;
#endif // !CHRONO
	}

	/*!
		������� ��������� ���������� ������� ��� ����������.
		\return ������, ���� ������ �����, ����� ����.
	*/
	bool CheckWithoutUpdate()
	{
#ifndef CHRONO
		//��������� �������� �������� �������� ������������������
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		if (currentCounter - previousCounter >= timeCount)
		{
			return true;
		}
		return false;
#else
		currentNS = std::chrono::steady_clock::now();

		if (currentNS - previousNS >= ns)
		{
			return true;
		}
		return false;
#endif // !CHRONO
	}

	/*!
		������� ��������� ���������� ������� ��� ����������.
		���������� ������� ����� ������� � ���������� ��������.
		\param [out] time ������ �� ������, ��� ������ �������.
		\return ������, ���� ������ �����, ����� ����.
	*/
	bool CheckWithoutUpdate(float& time)
	{
#ifndef CHRONO
		//��������� �������� �������� �������� ������������������
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		unsigned long long count = currentCounter - previousCounter;
		time = timePerFrec * count;

		if (count >= timeCount)
		{
			return true;
		}
		return false;
#else
		currentNS = std::chrono::steady_clock::now();

		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::time_point(currentNS - previousNS);
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count() * 0.000000001;

		if (t.time_since_epoch() >= ns)
		{
			return true;
		}
		return false;
#endif // !CHRONO
	}

	/*!
		������� ���������� ����� � ������ ������ �������.
		\param [out] time ������ �� ������, ��� ������ �������.
		\return �����, ������� ������ � ������� ������ ������� � �������.
	*/
	void GetTime(float& time)
	{
#ifndef CHRONO
		//��������� �������� �������� �������� ������������������
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		//���������� ������� � ��������, ������� ������ � ������� ������ �������
		time = timePerFrec * (currentCounter - baseCounter);
#else
		currentNS = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::time_point(currentNS - previousNS);
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count() * 0.000000001;
#endif // !CHRONO
	}

	///*!
	//	������� ���������� ������� ���������� ������ � �������
	//	�� �������.
	//*/
	//void CalculateFrameStats()
	//{
	//	static int countFrame = 0;

	//	++countFrame;

	//	if (CheckWithUpdate())
	//	{
	//		std::cout << "FPS: " << countFrame << "; Frame time: " << (float)1000 / countFrame << " ms" << std::endl;

	//		countFrame = 0;
	//	}
	//}
private:
#ifndef CHRONO
	float timeCount;                    //�����, ������� ���� ������ � ������
	float timePerFrec;                  //����� �� ���� � ��������
	unsigned long long frequency;       //������� ��������
	unsigned long long baseCounter;     //��������� �������� ��������
	unsigned long long currentCounter;  //������� �������� ��������
	unsigned long long previousCounter; //���������� �������� ��������
#else
	double sec;                                       //�����, ������� ���� ������ � ��������
	std::chrono::nanoseconds ns;                      //�����, ������� ���� ������ � ������������
	std::chrono::steady_clock::time_point baseNS;     //��������� �������� �������
	std::chrono::steady_clock::time_point currentNS;  //������� �������� �������
	std::chrono::steady_clock::time_point previousNS; //���������� �������� �������
#endif // !CHRONO
};

#endif // !MAY_TIMER_H