#ifndef MAY_JSON_H
#define MAY_JSON_H

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

#include <fstream>
#include <deque>
#include <string>

namespace may_json
{

struct JSON
{
	JSON()
	{
		previousPtr = 0;
	}

    virtual ~JSON()
    {

    }

	virtual void foo() {};

	JSON* previousPtr;
};

struct JSONObject : public JSON
{
    virtual ~JSONObject()
    {
        for (uint32_t i = 0; i < pairs.size(); ++i)
        {
            delete pairs[i].second;
        }
    }

	virtual void foo() {};

	std::deque<std::pair<std::string, JSON*> > pairs;
};

struct JSONArray : public JSON
{
    virtual ~JSONArray()
    {
        for (uint32_t i = 0; i < array.size(); ++i)
        {
            delete array[i];
        }
    }

	virtual void foo() {};

	std::deque<JSON*> array;
};

struct JSONValue : public JSON
{
	virtual void foo() {};
	std::string value;
};

class JSONParsing
{
public:
	JSONParsing()
	{
		currentPos = 0;
		object = 0;
	}

    ~JSONParsing()
    {
        delete object;
    }

	bool Read(const char* fileName)
    {
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        if (!file)
        {
            return false;
        }

        file.seekg(0, std::ios::end);
        uint32_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string json;
        json.resize(fileSize);
        file.read(const_cast<char*>(json.data()), fileSize);
        file.close();

        JSON* currentObjectPtr = 0; //текущий объект, который инициализируетс€
        JSON** newPtr = 0;          //указатель на нулевой указатель, дл€ которого выдел€етс€ пам€ть

        std::string str;
        bool strFlag = false;
        bool valueFlag = false;

        while (currentPos < json.size())
        {
            char symbol = GetNextSymbolWithoutSpace(json);

            if (valueFlag)
            {
                switch (symbol)
                {
                case '{':
                {
                    *newPtr = new JSONObject;
                    (*newPtr)->previousPtr = currentObjectPtr;
                    currentObjectPtr = *newPtr;
                    valueFlag = false;
                    break;
                }
                case '[':
                {
                    *newPtr = new JSONArray;
                    (*newPtr)->previousPtr = currentObjectPtr;
                    currentObjectPtr = (*newPtr);
                    valueFlag = false;
                    break;
                }
                case '\"':
                {
                    ++currentPos;
                    str = GetStr(json);

                    *newPtr = new JSONValue;
                    reinterpret_cast<JSONValue*>(*newPtr)->value = str;

                    (*newPtr)->previousPtr = currentObjectPtr;
                    valueFlag = false;
                    break;
                }
                default:
                {
                    str = GetVal(json);

                    *newPtr = new JSONValue;
                    reinterpret_cast<JSONValue*>(*newPtr)->value = str;

                    (*newPtr)->previousPtr = currentObjectPtr;
                    valueFlag = false;
                    break;
                }
                }

                ++currentPos;
                continue;
            }

            switch (symbol)
            {
            case '{':
            {
                if (!object)
                {
                    object = new JSONObject;
                    currentObjectPtr = object;
                }
                else
                {
                    JSONObject* object = new JSONObject;
                    object->previousPtr = currentObjectPtr;

                    JSONArray* tempPtr = reinterpret_cast<JSONArray*>(currentObjectPtr);
                    tempPtr->array.push_back(object);

                    currentObjectPtr = object;
                }
                break;
            }
            case '\"':
            {
                ++currentPos;
                str = GetStr(json);

                JSONArray* tempPtr = dynamic_cast<JSONArray*>(currentObjectPtr);
                if (tempPtr)
                {
                    JSONValue* value = new JSONValue;
                    value->value = str;
                    value->previousPtr = tempPtr;

                    tempPtr->array.push_back(value);
                }
                break;
            }
            case ':':
            {
                JSONObject* tempPtr = reinterpret_cast<JSONObject*>(currentObjectPtr);
                tempPtr->pairs.push_back(std::pair<std::string, JSON*>(str, 0));

                newPtr = &tempPtr->pairs.back().second;
                valueFlag = true;
                break;
            }
            case '}':
            {
                currentObjectPtr = currentObjectPtr->previousPtr;
                break;
            }
            case ']':
            {
                currentObjectPtr = currentObjectPtr->previousPtr;
                break;
            }
            case 0:
            case ',':
                break;
            default:
            {
                JSONArray* tempPtr = dynamic_cast<JSONArray*>(currentObjectPtr);
                if (tempPtr)
                {
                    str = GetVal(json);

                    JSONValue* value = new JSONValue;
                    value->value = str;
                    value->previousPtr = tempPtr;

                    tempPtr->array.push_back(value);
                }
                break;
            }
            }

            ++currentPos;
        }

        return true;
    }

    bool Read(std::string& json, uint32_t pos)
    {
        currentPos = pos;

        JSON* currentObjectPtr = 0; //текущий объект, который инициализируетс€
        JSON** newPtr = 0;          //указатель на нулевой указатель, дл€ которого выдел€етс€ пам€ть

        if (json.size() > 3)
        {
            for (uint32_t i = 0; i < 3; ++i)
            {
                if (static_cast<uint8_t>(json[i]) == 0xEF || static_cast<uint8_t>(json[i]) == 0xBB || static_cast<uint8_t>(json[i]) == 0xBF)
                {
                    ++currentPos;
                }
            }
        }

        std::string str;
        bool strFlag = false;
        bool valueFlag = false;

        while (currentPos < json.size())
        {
            char symbol = GetNextSymbolWithoutSpace(json);

            if (valueFlag)
            {
                switch (symbol)
                {
                case '{':
                {
                    *newPtr = new JSONObject;
                    (*newPtr)->previousPtr = currentObjectPtr;
                    currentObjectPtr = *newPtr;
                    valueFlag = false;
                    break;
                }
                case '[':
                {
                    *newPtr = new JSONArray;
                    (*newPtr)->previousPtr = currentObjectPtr;
                    currentObjectPtr = (*newPtr);
                    valueFlag = false;
                    break;
                }
                case '\"':
                {
                    ++currentPos;
                    str = GetStr(json);

                    *newPtr = new JSONValue;
                    reinterpret_cast<JSONValue*>(*newPtr)->value = str;

                    (*newPtr)->previousPtr = currentObjectPtr;
                    valueFlag = false;
                    break;
                }
                default:
                {
                    str = GetVal(json);

                    *newPtr = new JSONValue;
                    reinterpret_cast<JSONValue*>(*newPtr)->value = str;

                    (*newPtr)->previousPtr = currentObjectPtr;
                    valueFlag = false;
                    break;
                }
                }

                ++currentPos;
                continue;
            }

            switch (symbol)
            {
            case '{':
            {
                if (!object)
                {
                    object = new JSONObject;
                    currentObjectPtr = object;
                }
                else
                {
                    JSONObject* object = new JSONObject;
                    object->previousPtr = currentObjectPtr;

                    JSONArray* tempPtr = reinterpret_cast<JSONArray*>(currentObjectPtr);
                    tempPtr->array.push_back(object);

                    currentObjectPtr = object;
                }
                break;
            }
            case '\"':
            {
                ++currentPos;
                str = GetStr(json);

                JSONArray* tempPtr = dynamic_cast<JSONArray*>(currentObjectPtr);
                if (tempPtr)
                {
                    JSONValue* value = new JSONValue;
                    value->value = str;
                    value->previousPtr = tempPtr;

                    tempPtr->array.push_back(value);
                }
                break;
            }
            case ':':
            {
                JSONObject* tempPtr = reinterpret_cast<JSONObject*>(currentObjectPtr);
                tempPtr->pairs.push_back(std::pair<std::string, JSON*>(str, 0));

                newPtr = &tempPtr->pairs.back().second;
                valueFlag = true;
                break;
            }
            case '}':
            {
                currentObjectPtr = currentObjectPtr->previousPtr;
                break;
            }
            case ']':
            {
                currentObjectPtr = currentObjectPtr->previousPtr;
                break;
            }
            case 0:
            case ',':
                break;
            default:
            {
                JSONArray* tempPtr = dynamic_cast<JSONArray*>(currentObjectPtr);
                if (tempPtr)
                {
                    str = GetVal(json);

                    JSONValue* value = new JSONValue;
                    value->value = str;
                    value->previousPtr = tempPtr;

                    tempPtr->array.push_back(value);
                }
                break;
            }
            }

            ++currentPos;
        }

        return true;
    }

	JSON* GetValueByKey(const char* key)
    {
        JSONObject* tempPtr = reinterpret_cast<JSONObject*>(object);
        for (uint32_t i = 0; i < tempPtr->pairs.size(); ++i)
        {
            if (std::strcmp(tempPtr->pairs[i].first.c_str(), key) == 0)
            {
                return tempPtr->pairs[i].second;
            }
        }
        return 0;
    }

    JSON* GetValueByKey(const char* key, JSONObject* jsonPtr)
    {
        for (uint32_t i = 0; i < jsonPtr->pairs.size(); ++i)
        {
            if (std::strcmp(jsonPtr->pairs[i].first.c_str(), key) == 0)
            {
                return jsonPtr->pairs[i].second;
            }
        }
        return 0;
    }

    JSON* GetMainObject()
    {
        return object;
    }

private:
	char GetNextSymbolWithoutSpace(const std::string& json)
    {
        for (currentPos; currentPos < json.size(); ++currentPos)
        {
            if (json[currentPos] != ' ' && json[currentPos] != '\r' && json[currentPos] != '\n' && json[currentPos] != '\t')
                return json[currentPos];
        }
        return 0;
    }

	std::string GetStr(const std::string& json)
    {
        std::string str;
        for (currentPos; currentPos < json.size(); ++currentPos)
        {
            if (json[currentPos] != '\"')
                str += json[currentPos];
            else
                return str;
        }
    }

	std::string GetVal(const std::string& json)
    {
        std::string str;
        for (currentPos; currentPos < json.size(); ++currentPos)
        {
            if (currentPos + 3 < json.size())
            {
                std::string boolStr;
                boolStr.resize(4);
                boolStr[0] = json[currentPos];
                boolStr[1] = json[currentPos + 1];
                boolStr[2] = json[currentPos + 2];
                boolStr[3] = json[currentPos + 3];

                if (boolStr == "true")
                    return "1";
                else if (boolStr == "false")
                    return "0";
                else if (boolStr == "null")
                    return "0";
            }

            if ((0x30 <= static_cast<uint8_t>(json[currentPos]) && static_cast<uint8_t>(json[currentPos]) <= 0x39) ||
                static_cast<uint8_t>(json[currentPos]) == 0x2E || static_cast<uint8_t>(json[currentPos]) == 0x2D ||
                static_cast<uint8_t>(json[currentPos]) == 0x45 || static_cast<uint8_t>(json[currentPos]) == 0x65)
                str += json[currentPos];
            else
                return str;
        }
    }

	uint32_t currentPos; //текущий индекс символа файла
	JSON* object;        //объект json
};

}

#endif // !MAY_JSON_H

