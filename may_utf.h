#ifndef MAY_ENCODING_UFT_H
#define MAY_ENCODING_UFT_H

/*
* The MIT License (MIT)
*
* Copyright (c) 2023 Malakhov Artyom
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand
* associated documentation files(the УSoftwareФ), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED УAS ISФ, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
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

/*!
* \param [in] s —имвол дл€ кодировани€ в UTF-8.
* \param [out] data —сылка на указатель, куда будут помещены данные
*/
inline void EncodingUft8(const uint32_t& s, std::vector<uint8_t>& data)
{
	Symbol* symbol = (Symbol*)&s;

	if (0 <= symbol->all && symbol->all <= 0x7F)
	{
		data.resize(1);
		data[0] = symbol->perByte[0];
		return;
	}
	else if (0x80 <= symbol->all && symbol->all <= 0x7FF)
	{
		data.resize(2);
		data[0] = 0xC0;
		data[1] = 0x80;
	}
	else if (0x800 <= symbol->all && symbol->all <= 0xFFFF)
	{
		data.resize(3);
		data[0] = 0xE0;
		data[1] = 0x80;
		data[2] = 0x80;
	}
	else if (0x10000 <= symbol->all && symbol->all <= 0x10FFFF)
	{
		data.resize(4);
		data[0] = 0xF0;
		data[1] = 0x80;
		data[2] = 0x80;
		data[3] = 0x80;
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
	for (int8_t j = data.size() - 1; j >= 1; --j)
	{
		for (uint8_t k = 0; k < 6; ++k)
		{
			data[j] |= (symbol->all & (1 << bit)) >> offset;
			++bit;
		}

		offset += 6;
	}

	for (uint8_t k = 0; k < 8 - data.size() + 1; ++k)
	{
		data[0] |= (symbol->all & (1 << bit)) >> offset;
		++bit;

		if (bit == maxBit)
			break;
	}
}

/*!
* \param [in] data ƒанные дл€ декодировани€ из UTF-8.
* \param [out] s —сылка, куда будут помещены данные
*/
inline void DecodingUtf8(const std::vector<uint8_t>& data, uint32_t& s)
{
	Symbol* symbol = (Symbol*)&s;

	uint8_t mask = 0;
	if ((data[0] & 0xF0) == 0xF0)
	{
		mask = 0xF0;
	}
	else if ((data[0] & 0xE0) == 0xE0)
	{
		mask = 0xE0;
	}
	else if ((data[0] & 0xC0) == 0xC0)
	{
		mask = 0xC0;
	}
	else
	{
		symbol->all = data[0];
		return;
	}

	symbol->all = 0;
	uint8_t offset = 0;
	for (uint8_t j = data.size() - 1; j >= 1; --j)
	{
		symbol->all |= static_cast<uint32_t>(data[j] ^ 0x80) << offset;
		offset += 6;
	}

	symbol->all |= static_cast<uint32_t>(data[0] ^ mask) << offset;
}

/*!
* \param [in] inData —сылка на данные символов UTF-16.
* \param [out] outData —сылка на указатель, куда будут помещены данные UTF-8.
*/
inline void Utf16ToUft8Array(std::vector<uint16_t>& inData, std::vector<uint8_t>& outData)
{
	uint32_t symbol = 0;
	std::vector<uint8_t> data;

	for (uint32_t i = 0; i < inData.size(); ++i)
	{
		EncodingUft8(inData[i], data);
		std::copy(data.begin(), data.end(), std::back_inserter(outData));
	}
}

/*!
* \param [in] inData —сылка на данные символов UTF-8.
* \param [out] outData —сылка на указатель, куда будут помещены данные UTF-16
*/
inline void Utf8ToUft16Array(std::vector<uint8_t>& inData, std::vector<uint16_t>& outData)
{
	uint32_t symbol = 0;
	std::vector<uint8_t> data;

	for (uint32_t i = 0; i < inData.size(); ++i)
	{
		if ((inData[i] & 0xE0) == 0xE0)
		{
			data.resize(3);
			data[0] = inData[i];
			data[1] = inData[++i];
			data[2] = inData[++i];
		}
		else if ((inData[i] & 0xC0) == 0xC0)
		{
			data.resize(2);
			data[0] = inData[i];
			data[1] = inData[++i];
		}
		else
		{
			data.resize(1);
			data[0] = inData[i];
		}

		DecodingUtf8(data, symbol);

		outData.push_back(symbol);
	}
}

/*!
* \param [in] inData —сылка на исходную строку.
* \param [out] outData —сылка на готовую строку.
*/
inline void Utf16ToUft8String(const std::wstring& inData, std::string& outData)
{
	std::vector<uint8_t> data;

	for (uint32_t i = 0; i < inData.size(); ++i)
	{
		EncodingUft8(inData[i], data);
		std::copy(data.begin(), data.end(), std::back_inserter(outData));
	}
}

/*!
* \param [in] inData —сылка на исходную строку.
* \param [out] outData —сылка на готовую строку.
*/
inline void Utf8ToUft16String(std::string& inData, std::wstring& outData)
{
	uint32_t symbol = 0;
	std::vector<uint8_t> data;

	for (uint32_t i = 0; i < inData.size(); ++i)
	{
		if ((inData[i] & 0xE0) == 0xE0)
		{
			data.resize(3);
			data[0] = inData[i];
			data[1] = inData[++i];
			data[2] = inData[++i];
		}
		else if ((inData[i] & 0xC0) == 0xC0)
		{
			data.resize(2);
			data[0] = inData[i];
			data[1] = inData[++i];
		}
		else
		{
			data.resize(1);
			data[0] = inData[i];
		}

		DecodingUtf8(data, symbol);

		outData.push_back(symbol);
	}
}

}

#endif // !MAY_ENCODING_UFT_H