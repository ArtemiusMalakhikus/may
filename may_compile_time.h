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

#define MAY_DEBUG

#ifdef MAY_DEBUG
#define MAY_CONSTEXPR constexpr
#else
#define MAY_CONSTEXPR
#endif

namespace may_compile_time
{

MAY_CONSTEXPR auto pi = 3.14159265358979323846264338327950288;
MAY_CONSTEXPR auto precision = 0.000001f;

template<typename mayType>
MAY_CONSTEXPR mayType abs(mayType absolute)
{
	return absolute * ((absolute > 0) - (absolute < 0));
}

MAY_CONSTEXPR uint64_t factorial(uint64_t n)
{
	return n == 0 ? 1 : n * may_compile_time::factorial(n - 1);
}

template<typename mayType>
MAY_CONSTEXPR mayType pow(mayType base, uint32_t exp)
{
	return exp == 0 ? 1 : base * may_compile_time::pow(base, exp - 1);
}

template<typename mayType>
MAY_CONSTEXPR mayType sin_series(mayType rad, uint32_t n)
{
	mayType number = may_compile_time::pow(-1, n) * (may_compile_time::pow(rad, 2 * n + 1) / may_compile_time::factorial(2 * n + 1));
	return may_compile_time::abs(number) < precision ? number : number + may_compile_time::sin_series(rad, n + 1);
}

template<typename mayType>
MAY_CONSTEXPR mayType cos_series(mayType rad, uint32_t n)
{
	mayType number = may_compile_time::pow(-1, n) * (may_compile_time::pow(rad, 2 * n) / may_compile_time::factorial(2 * n));
	return may_compile_time::abs(number) < precision ? number : number + may_compile_time::cos_series(rad, n + 1);
}

template<typename mayType>
MAY_CONSTEXPR mayType sin(mayType rad)
{
	return may_compile_time::sin_series(rad > pi ? -((pi * 2) - rad) : rad, 0);
}

template<typename mayType>
MAY_CONSTEXPR mayType cos(mayType rad)
{
	return may_compile_time::cos_series(rad > pi ? (pi * 2) - rad : rad, 0);
}

}

#endif // !MAY_COMPILE_TIME
