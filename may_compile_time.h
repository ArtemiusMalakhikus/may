/*
* The MIT License (MIT)
*
* Copyright (c) 2024 Malakhov Artyom
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

#ifndef MAY_COMPILE_TIME
#define MAY_COMPILE_TIME

#define MAY_NDEBUG

#ifdef MAY_NDEBUG
#define MAY_CONSTEXPR constexpr
#else
#define MAY_CONSTEXPR
#endif

#include <cstdint>

namespace may
{

MAY_CONSTEXPR double pi = 3.14159265358979323846264338327950288;
MAY_CONSTEXPR double precision = 0.000001;
MAY_CONSTEXPR double ln2 = 0.6931471805599453;

template<typename T>
MAY_CONSTEXPR T abs(T absolute)
{
	return absolute * ((absolute > 0) - (absolute < 0));
}

MAY_CONSTEXPR uint64_t factorial(uint64_t n)
{
	return n == 0 ? 1 : n * may::factorial(n - 1);
}

template<typename T>
MAY_CONSTEXPR T pow(T base, int32_t exp)
{
	return exp == 0 ? 1 : base * may::pow(base, exp - 1);
}

template<typename T>
MAY_CONSTEXPR T sin_series(T rad, int32_t n)
{
	T number = may::pow(-1, n) * (may::pow(rad, 2 * n + 1) / may::factorial(2 * n + 1));
	return may::abs(number) < static_cast<T>(precision) ? number : number + may::sin_series(rad, n + 1);
}

template<typename T>
MAY_CONSTEXPR T cos_series(T rad, int32_t n)
{
	T number = may::pow(-1, n) * (may::pow(rad, 2 * n) / may::factorial(2 * n));
	return may::abs(number) < static_cast<T>(precision) ? number : number + may::cos_series(rad, n + 1);
}

template<typename T>
MAY_CONSTEXPR T sin(T rad)
{
	return may::sin_series(rad > static_cast<T>(pi) ? -((static_cast<T>(pi) * 2) - rad) : rad, 0);
}

template<typename T>
MAY_CONSTEXPR T cos(T rad)
{
	return may::cos_series(rad > static_cast<T>(pi) ? (static_cast<T>(pi) * 2) - rad : rad, 0);
}

template<typename T>
MAY_CONSTEXPR T log2_series(T b, int32_t n)
{
	T number = static_cast<T>(2.0) * may::pow((b - static_cast<T>(1.0)) / (b + static_cast<T>(1.0)), 
		static_cast<T>(2 * n + 1)) / static_cast<T>(2 * n + 1);
	return number < static_cast<T>(precision) ? number : number + may::log2_series(b, n + 1);
}

template<typename T>
MAY_CONSTEXPR T log2(T b)
{
	int32_t result = 0;

	while (b >= static_cast<T>(2.0))
	{
		b /= static_cast<T>(2.0);
		++result;
	}

	while (b < static_cast<T>(1.0))
	{
		b *= static_cast<T>(2.0);
		--result;
	}

	return static_cast<T>(result) + may::log2_series(b, 0) / static_cast<T>(ln2);
}

template<typename T, typename Q>
MAY_CONSTEXPR Q MatrixProduct(const T& mat3, const Q& vec3)
{
	return Q{
		mat3[0][0] * vec3[0] + mat3[0][1] * vec3[1] + mat3[0][2] * vec3[2],
		mat3[1][0] * vec3[0] + mat3[1][1] * vec3[1] + mat3[1][2] * vec3[2],
		mat3[2][0] * vec3[0] + mat3[2][1] * vec3[1] + mat3[2][2] * vec3[2]
	};
}

}

#endif // !MAY_COMPILE_TIME
