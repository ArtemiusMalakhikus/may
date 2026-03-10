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

#ifndef MAY_ENCODING_UFT_H
#define MAY_ENCODING_UFT_H

#include <stdint.h>
#include <iterator>
#include <vector>
#include <string>
#include <algorithm>

namespace may
{

union Symbol
{
	uint32_t all;
	uint8_t perByte[4];
};

inline void EncodingUft8(const uint32_t& symbolForEncoding, uint8_t* encodedData, uint8_t& size)
{
	Symbol* symbol = (Symbol*)&symbolForEncoding;

	if (0 <= symbol->all && symbol->all <= 0x7F)
	{
		size = 1;
		encodedData[0] = symbol->perByte[0];
		return;
	}
	else if (0x80 <= symbol->all && symbol->all <= 0x7FF)
	{
		size = 2;
		encodedData[0] = 0xC0;
		encodedData[1] = 0x80;
	}
	else if (0x800 <= symbol->all && symbol->all <= 0xFFFF)
	{
		size = 3;
		encodedData[0] = 0xE0;
		encodedData[1] = 0x80;
		encodedData[2] = 0x80;
	}
	else if (0x10000 <= symbol->all && symbol->all <= 0x10FFFF)
	{
		size = 4;
		encodedData[0] = 0xF0;
		encodedData[1] = 0x80;
		encodedData[2] = 0x80;
		encodedData[3] = 0x80;
	}
	else
		return;

	uint8_t maxBit = 0;
	for (; maxBit < 4; ++maxBit)
	{
		if (symbol->perByte[maxBit] == 0)
		{
			maxBit *= 8;
			break;
		}
	}

	uint8_t bit = 0;
	uint8_t offset = 0;
	for (int8_t j = size - 1; j >= 1; --j)
	{
		for (uint8_t k = 0; k < 6; ++k)
		{
			encodedData[j] |= (symbol->all & (1 << bit)) >> offset;
			++bit;
		}

		offset += 6;
	}

	for (uint8_t k = 0; k < 8 - size + 1; ++k)
	{
		encodedData[0] |= (symbol->all & (1 << bit)) >> offset;
		++bit;

		if (bit == maxBit)
			break;
	}
}

inline size_t GetSizeUft8(const uint32_t& symbolForEncoding)
{
	Symbol* symbol = (Symbol*)&symbolForEncoding;

	if (0 <= symbol->all && symbol->all <= 0x7F)
		return 1;
	else if (0x80 <= symbol->all && symbol->all <= 0x7FF)
		return 2;
	else if (0x800 <= symbol->all && symbol->all <= 0xFFFF)
		return 3;
	else if (0x10000 <= symbol->all && symbol->all <= 0x10FFFF)
		return 4;
	else
		return 0;
}

inline size_t GetSizeUft16(const uint8_t& symbolForDecoding)
{
	if ((symbolForDecoding & 0xE0) == 0xE0)
		return 3;
	else if ((symbolForDecoding & 0xC0) == 0xC0)
		return 2;
	else
		return 1;
}

inline void DecodingUtf8(uint8_t* dataForDecoding, const uint8_t& size, uint32_t& decodedSymbol)
{
	Symbol* symbol = (Symbol*)&decodedSymbol;

	uint8_t mask = 0;
	if ((dataForDecoding[0] & 0xF0) == 0xF0)
	{
		mask = 0xF0;
	}
	else if ((dataForDecoding[0] & 0xE0) == 0xE0)
	{
		mask = 0xE0;
	}
	else if ((dataForDecoding[0] & 0xC0) == 0xC0)
	{
		mask = 0xC0;
	}
	else
	{
		symbol->all = dataForDecoding[0];
		return;
	}

	symbol->all = 0;
	uint8_t offset = 0;
	for (uint8_t j = size - 1; j >= 1; --j)
	{
		symbol->all |= static_cast<uint32_t>(dataForDecoding[j] ^ 0x80) << offset;
		offset += 6;
	}

	symbol->all |= static_cast<uint32_t>(dataForDecoding[0] ^ mask) << offset;
}

template<typename V, typename T>
V Utf16ToUft8(const T& containerUtf16)
{
	const uint16_t* utf16 = reinterpret_cast<const uint16_t*>(containerUtf16.data());
	size_t size = sizeof(*containerUtf16.data()) * containerUtf16.size() / sizeof(uint16_t);

	size_t utf8Size = 0;
	for (size_t i = 0; i < size; ++i)
		utf8Size += GetSizeUft8(utf16[i]);

	V containerUtf8;
	containerUtf8.resize(std::roundf(static_cast<float>(utf8Size) / sizeof(*containerUtf8.data())));
	uint8_t* utf8 = reinterpret_cast<uint8_t*>(containerUtf8.data());
	size_t utf8Index = 0;

	for (size_t i = 0; i < size; ++i)
	{
		uint8_t data[4];
		uint8_t size = 0;
		EncodingUft8(utf16[i], data, size);
		for (uint8_t i = 0; i < size; ++i)
			utf8[utf8Index++] = data[i];
	}

	return std::move(containerUtf8);
}

template<typename V, typename T>
V Utf8ToUft16(const T& containerUtf8)
{
	const uint8_t* utf8 = reinterpret_cast<const uint8_t*>(containerUtf8.data());
	size_t size = sizeof(*containerUtf8.data()) * containerUtf8.size();

	size_t utf16Size = 0;
	size_t symbolSize = 0;
	for (size_t i = 0; i < size; i += symbolSize)
	{
		symbolSize = GetSizeUft16(utf8[i]);
		++utf16Size;
	}

	V containerUtf16;
	containerUtf16.resize(std::round(utf16Size * static_cast<float>(sizeof(uint16_t)) / sizeof(*containerUtf16.data())));
	uint16_t* utf16 = reinterpret_cast<uint16_t*>(containerUtf16.data());
	size_t utf16Index = 0;

	for (size_t i = 0; i < size; ++i)
	{
		uint8_t data[4];
		uint8_t size = 0;
		uint32_t symbol = 0;

		if ((utf8[i] & 0xE0) == 0xE0)
		{
			size = 3;
			data[0] = utf8[i];
			data[1] = utf8[++i];
			data[2] = utf8[++i];
		}
		else if ((utf8[i] & 0xC0) == 0xC0)
		{
			size = 2;
			data[0] = utf8[i];
			data[1] = utf8[++i];
		}
		else
		{
			size = 1;
			data[0] = utf8[i];
		}

		DecodingUtf8(data, size, symbol);
		utf16[utf16Index++] = symbol;
	}

	return std::move(containerUtf16);
}

}

#endif // !MAY_ENCODING_UFT_H