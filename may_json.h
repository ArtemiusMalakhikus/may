#ifndef MAY_JSON_H
#define MAY_JSON_H

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

/*
* This is a single file library for reading JSON files.
* File encoding support: UTF-8.
*/

#include <fstream>
#include <sstream>
#include <string>
#include <deque>

namespace may
{

enum ValueType
{
    NONE,
    OBJECT,
    ARRAY,
    STRING,
    NUMBER,
    BOOL,
    NULLPTR
};

struct JSONValue
{
	JSONValue()
	{
		previousPtr = 0;
        type = NONE;
	}

    virtual ~JSONValue()
    {

    }

	JSONValue* previousPtr;
    ValueType type;
};

struct JSONObject : public JSONValue
{
    JSONObject()
    {
        type = OBJECT;
    }

    virtual ~JSONObject()
    {
        for (uint32_t i = 0; i < pairs.size(); ++i)
        {
            delete pairs[i].second;
        }
    }

	std::deque<std::pair<std::string, JSONValue*> > pairs;
};

struct JSONArray : public JSONValue
{
    JSONArray()
    {
        type = ARRAY;
    }

    virtual ~JSONArray()
    {
        for (uint32_t i = 0; i < array.size(); ++i)
        {
            delete array[i];
        }
    }

	std::deque<JSONValue*> array;
};

struct JSONText : public JSONValue
{
    JSONText()
        : JSONValue()
    {

    }

    JSONText(const char* str, ValueType _type)
        : JSONValue()
    {
        type = _type;
        string = str;
    }

    virtual ~JSONText()
    {

    }

	std::string string;
};

class JSON
{
public:
	JSON()
	{
		currentPos = 0;
		mainObject = 0;
	}

    ~JSON()
    {
        Clear();
    }

    void Clear()
    {
        delete mainObject;
        mainObject = 0;
    }

    /*!
    * \param [in] key
    * \param [in] jsonObjectPtr Pointer to add a new object or nullptr.
    * \return Pointer to new object.
    */
    JSONObject* AddObjectValue(const char* key, JSONObject* jsonObjectPtr)
    {
        if (!mainObject)
        {
            mainObject = new JSONObject;
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }
        else if (!jsonObjectPtr)
        {
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }

        jsonObjectPtr->pairs.push_back(std::pair<std::string, JSONValue*>(key, new JSONObject));
        return static_cast<JSONObject*>(jsonObjectPtr->pairs.back().second);
    }

    /*!
    * \param [in] jsonArrayPtr Pointer to add a new object.
    * \return Pointer to new object or nullptr.
    */
    JSONObject* AddObjectValue(JSONArray* jsonArrayPtr)
    {
        if (!jsonArrayPtr)
            return 0;

        jsonArrayPtr->array.push_back(new JSONObject);
        return static_cast<JSONObject*>(jsonArrayPtr->array.back());
    }

    /*!
    * \param [in] key
    * \param [in] jsonObjectPtr Pointer to add a new array or nullptr.
    * \return Pointer to new array.
    */
    JSONArray* AddArrayValue(const char* key, JSONObject* jsonObjectPtr)
    {
        if (!mainObject)
        {
            mainObject = new JSONObject;
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }
        else if (!jsonObjectPtr)
        {
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }

        jsonObjectPtr->pairs.push_back(std::pair<std::string, JSONValue*>(key, new JSONArray));
        return static_cast<JSONArray*>(jsonObjectPtr->pairs.back().second);
    }

    /*!
    * \param [in] jsonArrayPtr Pointer to add a new array.
    * \return Pointer to new array or nullptr.
    */
    JSONArray* AddArrayValue(JSONArray* jsonArrayPtr)
    {
        if (!jsonArrayPtr)
            return 0;

        jsonArrayPtr->array.push_back(new JSONArray);
        return static_cast<JSONArray*>(jsonArrayPtr->array.back());
    }

    /*!
    * \param [in] key
    * \param [in] str
    * \param [in] jsonObjectPtr Pointer to add a new string or nullptr.
    * \return Pointer to new string.
    */
    JSONText* AddStringValue(const char* key, const char* str, JSONObject* jsonObjectPtr)
    {
        if (!mainObject)
        {
            mainObject = new JSONObject;
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }
        else if (!jsonObjectPtr)
        {
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }

        jsonObjectPtr->pairs.push_back(std::pair<std::string, JSONValue*>(key, new JSONText(str, STRING)));
        return static_cast<JSONText*>(jsonObjectPtr->pairs.back().second);
    }

    /*!
    * \param [in] str
    * \param [in] jsonArrayPtr Pointer to add a new string.
    * \return Pointer to new string or nullptr.
    */
    JSONText* AddStringValue(const char* str, JSONArray* jsonArrayPtr)
    {
        if (!jsonArrayPtr)
            return 0;

        jsonArrayPtr->array.push_back(new JSONText(str, STRING));
        return static_cast<JSONText*>(jsonArrayPtr->array.back());
    }

    /*!
    * \param [in] key
    * \param [in] number
    * \param [in] jsonObjectPtr Pointer to add a new number or nullptr.
    * \return Pointer to new number.
    */
    JSONText* AddNumberValue(const char* key, const char* number, JSONObject* jsonObjectPtr)
    {
        if (!mainObject)
        {
            mainObject = new JSONObject;
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }
        else if (!jsonObjectPtr)
        {
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }

        jsonObjectPtr->pairs.push_back(std::pair<std::string, JSONValue*>(key, new JSONText(number, NUMBER)));
        return static_cast<JSONText*>(jsonObjectPtr->pairs.back().second);
    }

    /*!
    * \param [in] number
    * \param [in] jsonArrayPtr Pointer to add a new number.
    * \return Pointer to new number or nullptr.
    */
    JSONText* AddNumberValue(const char* number, JSONArray* jsonArrayPtr)
    {
        if (!jsonArrayPtr)
            return 0;

        jsonArrayPtr->array.push_back(new JSONText(number, NUMBER));
        return static_cast<JSONText*>(jsonArrayPtr->array.back());
    }

    /*!
    * \param [in] key
    * \param [in] boolean
    * \param [in] jsonObjectPtr Pointer to add a new bool or nullptr.
    * \return Pointer to new bool.
    */
    JSONText* AddBoolValue(const char* key, const char* boolean, JSONObject* jsonObjectPtr)
    {
        if (!mainObject)
        {
            mainObject = new JSONObject;
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }
        else if (!jsonObjectPtr)
        {
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }

        jsonObjectPtr->pairs.push_back(std::pair<std::string, JSONValue*>(key, new JSONText(boolean, BOOL)));
        return static_cast<JSONText*>(jsonObjectPtr->pairs.back().second);
    }

    /*!
    * \param [in] boolean
    * \param [in] jsonArrayPtr Pointer to add a new boolean.
    * \return Pointer to new boolean or nullptr.
    */
    JSONText* AddBoolValue(const char* boolean, JSONArray* jsonArrayPtr)
    {
        if (!jsonArrayPtr)
            return 0;

        jsonArrayPtr->array.push_back(new JSONText(boolean, BOOL));
        return static_cast<JSONText*>(jsonArrayPtr->array.back());
    }

    /*!
    * \param [in] key
    * \param [in] null
    * \param [in] jsonObjectPtr Pointer to add a new null or nullptr.
    * \return Pointer to new null.
    */
    JSONText* AddNullValue(const char* key, const char* null, JSONObject* jsonObjectPtr)
    {
        if (!mainObject)
        {
            mainObject = new JSONObject;
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }
        else if (!jsonObjectPtr)
        {
            jsonObjectPtr = static_cast<JSONObject*>(mainObject);
        }

        jsonObjectPtr->pairs.push_back(std::pair<std::string, JSONValue*>(key, new JSONText(null, NULLPTR)));
        return static_cast<JSONText*>(jsonObjectPtr->pairs.back().second);
    }

    /*!
    * \param [in] null
    * \param [in] jsonArrayPtr Pointer to add a new null.
    * \return Pointer to new null or nullptr.
    */
    JSONText* AddNullValue(const char* null, JSONArray* jsonArrayPtr)
    {
        if (!jsonArrayPtr)
            return 0;

        jsonArrayPtr->array.push_back(new JSONText(null, NULLPTR));
        return static_cast<JSONText*>(jsonArrayPtr->array.back());
    }

    /*!
    * \brief Reading a file from the json format.
    * \param [in] fileName File name with extension.
    */
	bool Read(const char* fileName)
    {
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        if (!file)
        {
            return true;
        }

        //file size determination
        file.seekg(0, std::ios::end);
        uint32_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        //copying file to string
        std::string json;
        json.resize(fileSize);
        file.read(const_cast<char*>(json.data()), fileSize);
        file.close();

        currentPos = 0;
        Parsing(json);
        return false;
    }

    /*!
    * \brief Reading a string from the json format.
    * \param [in] json String data.
    * \param [in] pos Begin position is the string.
    */
    bool Read(std::string& json, uint32_t pos)
    {
        currentPos = pos;
        Parsing(json);
        return false;
    }

    /*!
    * \brief Writing json to file.
    * \param [in] fileName File name with extension.
    */
    void Write(const char* fileName)
    {
        std::ofstream outFile(fileName, std::ios::out);
        std::ostringstream oss;
        std::string tab;
        BuildJSON(mainObject, oss, tab);
        outFile << oss.str();
        outFile.close();
    }

    /*!
    * \brief Writing json to string.
    * \param [in] json Reference to string data.
    */
    void Write(std::string& json)
    {
        std::ostringstream oss;
        std::string tab;
        BuildJSON(mainObject, oss, tab);
        json = oss.str();
    }

    /*!
    * \return Value or nullptr.
    */
    JSONValue* FindValueByKey(const char* key, JSONObject* jsonPtr)
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

    /*!
    * \return Object value or nullptr.
    */
    JSONObject* GetObjectValue(JSONValue* value)
    {
        return (value && value->type == OBJECT) ? static_cast<JSONObject*>(value) : 0;
    }

    /*!
    * \return Array value or nullptr.
    */
    JSONArray* GetArrayValue(JSONValue* value)
    {
        return (value && value->type == ARRAY) ? static_cast<JSONArray*>(value) : 0;
    }

    /*!
    * \return String value or nullptr.
    */
    JSONText* GetStringValue(JSONValue* value)
    {
        return (value && (value->type == STRING || value->type == NUMBER || value->type == BOOL || value->type == NULLPTR)) ? static_cast<JSONText*>(value) : 0;
    }

    JSONObject* GetMainObject()
    {
        return static_cast<JSONObject*>(mainObject);
    }

private:

    /*!
    * \brief Function parses JSON.
    */
    bool Parsing(std::string& json)
    {
        JSONValue* currentObjectPtr = 0; //current object, that is being initialized
        JSONValue** newPtr = 0;          //pointer to pointer for memory allocation

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
        bool endObjectFlag = false;

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
                    str = GetString(json);

                    *newPtr = new JSONText;
                    reinterpret_cast<JSONText*>(*newPtr)->string = str;
                    (*newPtr)->type = STRING;

                    (*newPtr)->previousPtr = currentObjectPtr;
                    valueFlag = false;
                    break;
                }
                default:
                {
                    std::pair<std::string, ValueType> p = GetNumber(json);

                    *newPtr = new JSONText;
                    reinterpret_cast<JSONText*>(*newPtr)->string = p.first;
                    (*newPtr)->type = p.second;

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
                if (!mainObject)
                {
                    mainObject = new JSONObject;
                    currentObjectPtr = mainObject;
                }
                else
                {
                    JSONObject* newObject = new JSONObject;
                    newObject->previousPtr = currentObjectPtr;

                    JSONArray* tempPtr = reinterpret_cast<JSONArray*>(currentObjectPtr);
                    tempPtr->array.push_back(newObject);

                    currentObjectPtr = newObject;
                }
                break;
            }
            case '\"':
            {
                ++currentPos;
                str = GetString(json);

                JSONArray* tempPtr = dynamic_cast<JSONArray*>(currentObjectPtr);
                if (tempPtr)
                {
                    JSONText* value = new JSONText;
                    value->string = str;
                    value->previousPtr = tempPtr;

                    tempPtr->array.push_back(value);
                }
                break;
            }
            case ':':
            {
                JSONObject* tempPtr = reinterpret_cast<JSONObject*>(currentObjectPtr);
                tempPtr->pairs.push_back(std::pair<std::string, JSONValue*>(str, 0));

                newPtr = &tempPtr->pairs.back().second;
                valueFlag = true;
                break;
            }
            case '}':
            {
                currentObjectPtr = currentObjectPtr->previousPtr;
                endObjectFlag = true;
                break;
            }
            case ']':
            {
                currentObjectPtr = currentObjectPtr->previousPtr;
                break;
            }
            case ',':
            {
                /*if (endObjectFlag)
                {
                    endObjectFlag = false;
                    JSONArray* tempPtr = dynamic_cast<JSONArray*>(currentObjectPtr->previousPtr);
                    if (tempPtr)
                        currentObjectPtr = currentObjectPtr->previousPtr;
                }*/
                break;
            }
            case 0:
                break;
            default:
            {
                JSONArray* tempPtr = dynamic_cast<JSONArray*>(currentObjectPtr);
                if (tempPtr)
                {
                    std::pair<std::string, ValueType> p = GetNumber(json);

                    JSONText* value = new JSONText;
                    value->string = p.first;
                    value->type = p.second;
                    value->previousPtr = tempPtr;

                    tempPtr->array.push_back(value);
                }
                break;
            }
            }

            ++currentPos;
        }

        return false;
    }

    /*!
    * \brief Function recursively build data in json format.
    * \param [in] jsonPtr Pointer to json value.
    * \param [in] oss Reference to string stream.
    * \param [in] tab Reference to tabulation string.
    */
    void BuildJSON(JSONValue* value, std::ostringstream& oss, std::string& tab)
    {
        switch (value->type)
        {
        case OBJECT:
            oss << "{\n";
            tab += '\t';

            for (uint32_t i = 0; i < static_cast<JSONObject*>(value)->pairs.size(); ++i)
            {
                oss << tab << '"' << static_cast<JSONObject*>(value)->pairs[i].first.c_str() << '"' << ':';
                BuildJSON(static_cast<JSONObject*>(value)->pairs[i].second, oss, tab);

                if (i < static_cast<JSONObject*>(value)->pairs.size() - 1)
                    oss << ',';
                oss << '\n';
            }

            tab.erase(tab.size() - 1, 1);
            oss << tab << "}";
            break;
        case ARRAY:
            oss << "[\n";
            tab += '\t';

            for (uint32_t i = 0; i < static_cast<JSONArray*>(value)->array.size(); ++i)
            {
                oss << tab;
                BuildJSON((static_cast<JSONArray*>(value)->array[i]), oss, tab);

                if (i < static_cast<JSONArray*>(value)->array.size() - 1)
                    oss << ',';
                oss << '\n';
            }

            tab.erase(tab.size() - 1, 1);
            oss << tab << "]";
            break;
        case STRING:
            oss << '"' << static_cast<JSONText*>(value)->string << '"';
            break;
        case NUMBER:
            oss << static_cast<JSONText*>(value)->string;
            break;
        case BOOL:
            oss << std::boolalpha << static_cast<bool>(std::atoi(static_cast<JSONText*>(value)->string.c_str())) << std::noboolalpha;
            break;
        case NULLPTR:
            oss << "null";
            break;
        default:
            break;
        }
    }

	char GetNextSymbolWithoutSpace(const std::string& json)
    {
        for (currentPos; currentPos < json.size(); ++currentPos)
        {
            if (json[currentPos] != ' ' && json[currentPos] != '\r' && json[currentPos] != '\n' && json[currentPos] != '\t')
                return json[currentPos];
        }
        return 0;
    }

	std::string GetString(const std::string& json)
    {
        std::string str;
        bool escapeSymbol = false;
        for (currentPos; currentPos < json.size(); ++currentPos)
        {
            if (json[currentPos] == '\\')
            {
                escapeSymbol = true;
                continue;
            }
            else if (json[currentPos] == '\"' && escapeSymbol)
                escapeSymbol = false;
            else if (json[currentPos] == '\"' && !escapeSymbol)
                return str;

            str += json[currentPos];
        }
    }

	std::pair<std::string, ValueType> GetNumber(const std::string& json)
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
		        if (boolStr == "null")
                    return std::pair<std::string, ValueType>("0", NULLPTR);
		    
		        boolStr[0] = json[currentPos];
		        boolStr[1] = json[currentPos + 1];
		        boolStr[2] = json[currentPos + 2];
                boolStr[3] = json[currentPos + 3];
		        if (boolStr == "true")
     			    return std::pair<std::string, ValueType>("1", BOOL);
		    
		        boolStr.resize(5);
		        boolStr[0] = json[currentPos];
		        boolStr[1] = json[currentPos + 1];
		        boolStr[2] = json[currentPos + 2];
		        boolStr[3] = json[currentPos + 3];
		        boolStr[4] = json[currentPos + 4];
		        if (boolStr == "false")
     			    return std::pair<std::string, ValueType>("0", BOOL);
            }

            if ((0x30 <= static_cast<uint8_t>(json[currentPos]) && static_cast<uint8_t>(json[currentPos]) <= 0x39) ||
                static_cast<uint8_t>(json[currentPos]) == 0x2E || static_cast<uint8_t>(json[currentPos]) == 0x2D ||
                static_cast<uint8_t>(json[currentPos]) == 0x45 || static_cast<uint8_t>(json[currentPos]) == 0x65)
                str += json[currentPos];
            else if (!str.empty())
                return std::pair<std::string, ValueType>(str, NUMBER);
            else
                return std::pair<std::string, ValueType>(str, NONE);
        }
    }

	uint32_t currentPos; //current positon symbol in JSON data
	JSONValue* mainObject;   //main object JSON
};

}

#endif // !MAY_JSON_H

