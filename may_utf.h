#ifndef MAY_ENCODING_UFT_H
#define MAY_ENCODING_UFT_H

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

#include <stdint.h>
#include <iterator>
#include <vector>

namespace may_utf
{

union Symbol
{
	uint32_t all;
	uint8_t perByte[4];
};

inline void EncodingUft8(const uint32_t& symbolForEncoding, std::vector<uint8_t>& encodedData)
{
	Symbol* symbol = (Symbol*)&symbolForEncoding;

	if (0 <= symbol->all && symbol->all <= 0x7F)
	{
		encodedData.resize(1);
		encodedData[0] = symbol->perByte[0];
		return;
	}
	else if (0x80 <= symbol->all && symbol->all <= 0x7FF)
	{
		encodedData.resize(2);
		encodedData[0] = 0xC0;
		encodedData[1] = 0x80;
	}
	else if (0x800 <= symbol->all && symbol->all <= 0xFFFF)
	{
		encodedData.resize(3);
		encodedData[0] = 0xE0;
		encodedData[1] = 0x80;
		encodedData[2] = 0x80;
	}
	else if (0x10000 <= symbol->all && symbol->all <= 0x10FFFF)
	{
		encodedData.resize(4);
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
	for (int8_t j = encodedData.size() - 1; j >= 1; --j)
	{
		for (uint8_t k = 0; k < 6; ++k)
		{
			encodedData[j] |= (symbol->all & (1 << bit)) >> offset;
			++bit;
		}

		offset += 6;
	}

	for (uint8_t k = 0; k < 8 - encodedData.size() + 1; ++k)
	{
		encodedData[0] |= (symbol->all & (1 << bit)) >> offset;
		++bit;

		if (bit == maxBit)
			break;
	}
}

inline void DecodingUtf8(const std::vector<uint8_t>& dataForDecoding, uint32_t& decodedSymbol)
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
	for (uint8_t j = dataForDecoding.size() - 1; j >= 1; --j)
	{
		symbol->all |= static_cast<uint32_t>(dataForDecoding[j] ^ 0x80) << offset;
		offset += 6;
	}

	symbol->all |= static_cast<uint32_t>(dataForDecoding[0] ^ mask) << offset;
}

inline void Utf16ToUft8Array(std::vector<uint16_t>& input, std::vector<uint8_t>& output)
{
	uint32_t symbol = 0;
	std::vector<uint8_t> data;

	for (uint32_t i = 0; i < input.size(); ++i)
	{
		EncodingUft8(input[i], data);
		std::copy(data.begin(), data.end(), std::back_inserter(output));
	}
}

inline void Utf8ToUft16Array(std::vector<uint8_t>& input, std::vector<uint16_t>& output)
{
	uint32_t symbol = 0;
	std::vector<uint8_t> data;

	for (uint32_t i = 0; i < input.size(); ++i)
	{
		if ((input[i] & 0xE0) == 0xE0)
		{
			data.resize(3);
			data[0] = input[i];
			data[1] = input[++i];
			data[2] = input[++i];
		}
		else if ((input[i] & 0xC0) == 0xC0)
		{
			data.resize(2);
			data[0] = input[i];
			data[1] = input[++i];
		}
		else
		{
			data.resize(1);
			data[0] = input[i];
		}

		DecodingUtf8(data, symbol);

		output.push_back(symbol);
	}
}

inline void Utf16ToUft8String(const std::wstring& input, std::string& output)
{
	std::vector<uint8_t> data;

	for (uint32_t i = 0; i < input.size(); ++i)
	{
		EncodingUft8(input[i], data);
		std::copy(data.begin(), data.end(), std::back_inserter(output));
	}
}

inline void Utf8ToUft16String(std::string& input, std::wstring& output)
{
	uint32_t symbol = 0;
	std::vector<uint8_t> data;

	for (uint32_t i = 0; i < input.size(); ++i)
	{
		if ((input[i] & 0xE0) == 0xE0)
		{
			data.resize(3);
			data[0] = input[i];
			data[1] = input[++i];
			data[2] = input[++i];
		}
		else if ((input[i] & 0xC0) == 0xC0)
		{
			data.resize(2);
			data[0] = input[i];
			data[1] = input[++i];
		}
		else
		{
			data.resize(1);
			data[0] = input[i];
		}

		DecodingUtf8(data, symbol);

		output.push_back(symbol);
	}
}

}

#endif // !MAY_ENCODING_UFT_H