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
private:
	char Delimiter;
	bool IsFirstLineHeader;
	ifstream inFile;
	vector<string> LineFieldsValue;
	vector<string> Headers;
	map<string,int> FieldsIndices;

	vector<string> ParseLine(string line);
	
public:
	CCSVParser(void);
	bool OpenCSVFile(string fileName);
	void CloseCSVFile(void);
	bool ReadRecord();
	vector<string> GetLineRecord(void);
	vector<string> GetHeaderList();

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

			int index  = FieldsIndices[field_name];
			if(index >=LineFieldsValue.size())
			{
			return false;
			}
			string str_value = LineFieldsValue[index];

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
