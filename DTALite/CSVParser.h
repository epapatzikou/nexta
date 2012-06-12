#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

using std::string;
using std::ifstream;
using std::vector;
using std::map;
using std::istringstream;

class CCSVParser
{
public : ifstream inFile;

private:
	char Delimiter;
	bool IsFirstLineHeader;
	vector<string> LineFieldsValue;
	map<string,int> FieldsIndices;

	vector<string> ParseLine(string line);
	
public:
	CCSVParser(void);
	bool OpenCSVFile(string fileName);
	void CloseCSVFile(void);
	bool ReadRecord();

	template <class T> bool GetValueByFieldName(string field_name, T& value)
	{
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				return false;
			}

			string str_value = LineFieldsValue[FieldsIndices[field_name]];

			if (str_value.length() <= 0)
			{
				return false;
			}

			istringstream ss(str_value);

			T converted_value;
			ss >> converted_value;

			if (/*!ss.eof() || */ ss.fail())
			{
				return false;
			}

			value = converted_value;
			return true;
		}
	}

	template <> bool GetValueByFieldName<string>(string field_name, string& value)
	{
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				return false;
			}

			string str_value = LineFieldsValue[FieldsIndices[field_name]];

			if (str_value.length() <= 0)
			{
				return false;
			}

			value = str_value;
			return true;
		}
	}

	~CCSVParser(void);
};
