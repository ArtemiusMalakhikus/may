/*
* The MIT License (MIT)
*
* Copyright (c) 2023 Malakhov Artyom
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand
* associated documentation files(the “Software”), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS
* OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MAY_TIMER_H
#define MAY_TIMER_H

#ifndef CHRONO
#define NOMINMAX
#include <Windows.h>
#else
#include <chrono>
#endif

namespace may
{

class Timer
{
public:

	/*!
	* \param [in] second Time in second to measure.
	*/
	Timer(double second)
	{
#ifndef CHRONO
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		period = 1.0 / frequency;

		timerCounter = second * frequency;
		baseCounter = 0;
		currentCounter = 0;
		previousCounter = 0;
#else
		ns = static_cast<std::chrono::nanoseconds>(static_cast<long long>(second * 1000000000));
#endif // !CHRONO
	}

	Timer()
	{
#ifndef CHRONO
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		period = 1.0 / frequency;

		timerCounter = 0;
		baseCounter = 0;
		currentCounter = 0;
		previousCounter = 0;
#else
		ns = static_cast<std::chrono::nanoseconds>(0);
#endif // !CHRONO
	}

	/*!
	* \brief Set time for measurement.
	* \param [in] second Time in second to measure.
	*/
	void SetTime(double second)
	{
#ifndef CHRONO
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		timerCounter = second * frequency;
#else
		ns = static_cast<std::chrono::nanoseconds>(static_cast<long long>(second * 1000000000));
#endif // !CHRONO
	}

	/*!
	* \brief Timer start.
	*/
	void Start()
	{
#ifndef CHRONO
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
	* \return true - timer ready, else - false.
	*/
	bool CheckWithUpdate()
	{
#ifndef CHRONO
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		if (currentCounter - previousCounter >= timerCounter)
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
	* \param [out] time Reference to get the time.
	* \return true - timer ready, else - false.
	*/
	bool CheckWithUpdate(float& time)
	{
#ifndef CHRONO
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		unsigned long long count = currentCounter - previousCounter;
		time = period * count;

		if (count >= timerCounter)
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
	* \return true - timer ready, else - false.
	*/
	bool CheckWithoutUpdate()
	{
#ifndef CHRONO
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		if (currentCounter - previousCounter >= timerCounter)
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
	* \param [out] time Reference to get the time.
	* \return true - timer ready, else - false.
	*/
	bool CheckWithoutUpdate(float& time)
	{
#ifndef CHRONO
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		unsigned long long count = currentCounter - previousCounter;
		time = period * count;

		if (count >= timerCounter)
			return true;

		return false;
#else
		currentNS = std::chrono::steady_clock::now();

		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::time_point(currentNS - previousNS);
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count() * 0.000000001;

		if (t.time_since_epoch() >= ns)
			return true;

		return false;
#endif // !CHRONO
	}

	/*!
	* \return time.
	*/
	float GetTime()
	{
#ifndef CHRONO
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		//вычисление времени в секундах, которое прошло с момента старта таймера
		return period * (currentCounter - baseCounter);
#else
		currentNS = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::time_point(currentNS - previousNS);
		return std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count() * 0.000000001;
#endif // !CHRONO
	}

private:
#ifndef CHRONO
	float period;
	unsigned long long frequency;

	float timerCounter;
	unsigned long long baseCounter;
	unsigned long long currentCounter;
	unsigned long long previousCounter;
#else
	std::chrono::nanoseconds ns;                      //время, которое надо засечь в наносекундах
	std::chrono::steady_clock::time_point baseNS;     //начальное значение времени
	std::chrono::steady_clock::time_point currentNS;  //текущее значение времени
	std::chrono::steady_clock::time_point previousNS; //предыдущее значение времени
#endif // !CHRONO
	};

}

#endif // !MAY_TIMER_H