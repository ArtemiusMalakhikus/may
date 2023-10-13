#ifndef MAY_TIMER_H
#define MAY_TIMER_H

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
		\param [in] s Время, которое нужно засечь в секундах.
	*/
	may_timer(double s)
	{
#ifndef CHRONO
		//получение частоты счётчика производительности
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		//определяю время за такт
		timePerFrec = 1.0 / frequency;

		//определяю время в тактах, которое надо засечь
		timeCount = s * frequency;

		//инициализация значений счётчика
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
		//получение частоты счётчика производительности
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		//определяю время за такт
		timePerFrec = 1.0 / frequency;

		timeCount = 0;

		//инициализация значений счётчика
		baseCounter = 0;
		currentCounter = 0;
		previousCounter = 0;
#else
		ns = static_cast<std::chrono::nanoseconds>(0);
#endif // !CHRONO
	}

	/*!
		Функция устанавливает время, которое нужно засечь.
		\param [in] s Время, которое нужно засечь в секундах.
	*/
	void SetTime(double s)
	{
#ifndef CHRONO
		//получение частоты счётчика производительности
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		//определяю время в тактах, которое надо засечь
		timeCount = s * frequency;
#else
		sec = s;
		ns = static_cast<std::chrono::nanoseconds>(static_cast<long long>(sec * 1000000000));
#endif // !CHRONO
	}

	/*!
		Функция запускает таймер.
	*/
	void Start()
	{
#ifndef CHRONO
		//получение текущего значения счётчика производительности
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
		Функция проверяет готовность таймера c обновлением.
		\return истину, если таймер готов, иначе ложь.
	*/
	bool CheckWithUpdate()
	{
#ifndef CHRONO
		//получение текущего значения счётчика производительности
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
		Функция проверяет готовность таймера c обновлением.
		Записывает текущее время таймера в переданный параметр.
		\param [out] time Ссылка на данные, для записи времени.
		\return истину, если таймер готов, иначе ложь.
	*/
	bool CheckWithUpdate(float& time)
	{
#ifndef CHRONO
		//получение текущего значения счётчика производительности
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
		Функция проверяет готовность таймера без обновления.
		\return истину, если таймер готов, иначе ложь.
	*/
	bool CheckWithoutUpdate()
	{
#ifndef CHRONO
		//получение текущего значения счётчика производительности
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
		Функция проверяет готовность таймера без обновления.
		Записывает текущее время таймера в переданный параметр.
		\param [out] time Ссылка на данные, для записи времени.
		\return истину, если таймер готов, иначе ложь.
	*/
	bool CheckWithoutUpdate(float& time)
	{
#ifndef CHRONO
		//получение текущего значения счётчика производительности
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
		Функция возвращает время с начала старта таймера.
		\param [out] time Ссылка на данные, для записи времени.
		\return время, которое прошло с момента старта таймера в секудах.
	*/
	void GetTime(float& time)
	{
#ifndef CHRONO
		//получение текущего значения счётчика производительности
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCounter);

		//вычисление времени в секундах, которое прошло с момента старта таймера
		time = timePerFrec * (currentCounter - baseCounter);
#else
		currentNS = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::time_point(currentNS - previousNS);
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count() * 0.000000001;
#endif // !CHRONO
	}

	///*!
	//	Функция производит рассчёт количества кадров и времени
	//	за секунду.
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
	float timeCount;                    //время, которое надо засечь в тактах
	float timePerFrec;                  //время за такт в секундах
	unsigned long long frequency;       //частота счётчика
	unsigned long long baseCounter;     //начальное значение счётчика
	unsigned long long currentCounter;  //текущее значение счётчика
	unsigned long long previousCounter; //предыдущее значение счётчика
#else
	double sec;                                       //время, которое надо засечь в секундах
	std::chrono::nanoseconds ns;                      //время, которое надо засечь в наносекундах
	std::chrono::steady_clock::time_point baseNS;     //начальное значение времени
	std::chrono::steady_clock::time_point currentNS;  //текущее значение времени
	std::chrono::steady_clock::time_point previousNS; //предыдущее значение времени
#endif // !CHRONO
};

#endif // !MAY_TIMER_H