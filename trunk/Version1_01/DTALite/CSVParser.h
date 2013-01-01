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
using std::ostringstream;

#include <iostream>
#include <fstream>
using namespace std;

template <typename T>
string NumberToString ( T Number )
{
	ostringstream ss;
	ss << Number;
	return ss.str();
}


template <typename T>
T StringToNumber ( const string &Text )
{
	istringstream ss(Text);
	T result;
	return ss >> result ? result : 0;
}
class CCSVParser
{
public : ifstream inFile;

		 string mFileName;
private:
	char Delimiter;
	bool IsFirstLineHeader;
	vector<string> LineFieldsValue;
	map<string,int> FieldsIndices;

	vector<string> ParseLine(string line);

public:
	CCSVParser(void);
	bool OpenCSVFile(string fileName, bool b_required = true);
	void CloseCSVFile(void);
	bool ReadRecord();

	template <class T> bool GetValueByFieldNameRequired(string field_name, T& value)
	{
		bool required_field = true;
		bool print_out = false;
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			if(required_field)
			{
				cout << "Field " << field_name << " in file " << mFileName << " does not exist. Please check the file."  << endl;

				g_ProgramStop();
			}
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				return false;
			}

			if(FieldsIndices[field_name] >= LineFieldsValue.size())  // no value is read for index FieldsIndices[field_name]
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

			if(print_out)
			{
				cout << "Field " << field_name << " = " << converted_value << endl;
			}

			if (/*!ss.eof() || */ ss.fail())
			{
				return false;
			}

			value = converted_value;
			return true;
		}
	}


	template <class T> bool GetValueByFieldNameWithPrintOut(string field_name, T& value)
	{
		bool required_field = true;
		bool print_out = true;
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			if(required_field)
			{
				cout << "Field " << field_name << " in File " << mFileName << " does not exit."  << endl;

				g_ProgramStop();
			}
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				return false;
			}

			if(FieldsIndices[field_name] >= LineFieldsValue.size())  // no value is read for index FieldsIndices[field_name]
			{
				cout << "Missing value for " << field_name << " in File " << mFileName <<  endl;

				return false;
			}
			string str_value = LineFieldsValue[FieldsIndices[field_name]];

			if (str_value.length() <= 0)
			{
				return false;
			}

			istringstream ss(str_value);

			T converted_value;

			ss >> std::noskipws;
			ss >> converted_value;

			if(print_out)
			{
				cout << "Field " << field_name << " = " << converted_value << endl;
			}

			if (/*!ss.eof() || */ ss.fail())
			{
				return false;
			}

			value = converted_value;
			return true;
		}
	}

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

			if(FieldsIndices[field_name] >= LineFieldsValue.size())  // no value is read for index FieldsIndices[field_name]
			{
				return false;
			}
			string str_value = LineFieldsValue[FieldsIndices[field_name]];

			if (str_value.length() <= 0)
			{
				return false;
			}

			istringstream ss(str_value);

			ss >> std::noskipws;
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

	bool GetValueByFieldNameWithPrintOut(string field_name, string& value)
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

			int index = FieldsIndices[field_name];
			int size = LineFieldsValue.size();

			if(FieldsIndices[field_name]>= LineFieldsValue.size())
			{
				return false;
			}
			if (LineFieldsValue[FieldsIndices[field_name]].length() <= 0)
			{
				return false;
			}
			string str_value = LineFieldsValue[FieldsIndices[field_name]];



			value = str_value;
			return true;
		}
	}

	bool GetValueByFieldName(string field_name, string& value)
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

			int index = FieldsIndices[field_name];
			int size = LineFieldsValue.size();

			if(FieldsIndices[field_name]>= LineFieldsValue.size())
			{
				return false;
			}
			if (LineFieldsValue[FieldsIndices[field_name]].length() <= 0)
			{
				return false;
			}
			string str_value = LineFieldsValue[FieldsIndices[field_name]];



			value = str_value;
			return true;
		}
	}

	~CCSVParser(void);
};

class CCSVWriter
{
public : 
	ofstream outFile;
	char Delimiter;
	int FieldIndex;
	bool IsFirstLineHeader;
	map<int,string> LineFieldsValue;
	vector<string> LineFieldsName;
	vector<string> LineFieldsCategoryName;
	map<string,int> FieldsIndices;  

	bool row_title;

public:
	void SetRowTitle(bool flag)
	{
		row_title = flag;
	}

	bool OpenCSVFile(string fileName, bool b_required=true);
	void CloseCSVFile(void);
	template <class T> bool SetValueByFieldName(string field_name, T& value)  // by doing so, we do not need to exactly follow the sequence of field names
	{
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			return false;
		}
		else
		{

			LineFieldsValue[FieldsIndices[field_name]] = NumberToString(value);

			return true;
		}
	}

	void Reset()
	{

		LineFieldsValue.clear();
		LineFieldsName.clear();
		LineFieldsCategoryName.clear();
		FieldsIndices.clear();

	}
	void SetFieldName(string field_name)
	{ 
		FieldsIndices[field_name] = LineFieldsName.size();
		LineFieldsName.push_back (field_name);
		LineFieldsCategoryName.push_back(" ");

	}

	void SetFieldNameWithCategoryName(string field_name,string category_name)
	{ 
		FieldsIndices[field_name] = LineFieldsName.size();
		LineFieldsName.push_back (field_name);
		LineFieldsCategoryName.push_back(category_name);

	}


	void WriteTextString(CString textString)
	{
		if (!outFile.is_open()) 
			return;
		outFile << textString << endl;

	}

	void WriteTextLabel(CString textString)
	{
		if (!outFile.is_open()) 
			return;
		outFile << textString;

	}

	template <class T>  void WriteNumber(T value)
	{
		if (!outFile.is_open()) 
			return;
		outFile << NumberToString(value) << endl;
	}

	template <class T>  void WriteParameterValue(CString textString, T value)
	{
		if (!outFile.is_open()) 
			return;

		outFile << textString <<"=,"<< NumberToString(value) << endl;
	}

	void WriteNewEndofLine()
	{
		if (!outFile.is_open()) 
			return;
		outFile << endl;
	}


	void WriteHeader(bool bCategoryNameLine = true, bool bRowTitle = true)
	{
		if (!outFile.is_open()) 
			return;


		if(bCategoryNameLine == true)
		{
			for(unsigned int i = 0; i< FieldsIndices.size(); i++)
			{
				outFile << LineFieldsCategoryName[i] << ",";
			}
			outFile << endl;

		}

		if(bRowTitle == true)
			outFile << ",";

		for(unsigned int i = 0; i< FieldsIndices.size(); i++)
		{
			outFile << LineFieldsName[i] << ",";
		}

		outFile << endl;
	}
	void WriteRecord()
	{
		if (!outFile.is_open()) 
			return;

		for(unsigned int i = 0; i< FieldsIndices.size(); i++)
		{
			string str ;
			if(LineFieldsValue.find(i) != LineFieldsValue.end()) // has been initialized
				outFile << LineFieldsValue[i].c_str () << ",";
			else
				outFile << ' ' << ",";
		}

		LineFieldsValue.clear();

		outFile << endl;
	}

	CCSVWriter::CCSVWriter()
	{
		row_title = false;
		FieldIndex = 0;
		Delimiter = ',';
		IsFirstLineHeader = true;
	}

	CCSVWriter::~CCSVWriter(void)
	{
		if (outFile.is_open()) outFile.close();
	}


	CCSVWriter::CCSVWriter(string fileName)
	{
		Open(fileName);

	};

	void CCSVWriter::Open(string fileName)
	{
		outFile.open(fileName.c_str());

		if (outFile.is_open()==false)
		{
			cout << "File " << fileName.c_str() << " cannot be opened." << endl;
			getchar();
			exit(0);
		}

	};

	void CCSVWriter::OpenAppend(string fileName)
	{
		outFile.open(fileName.c_str(), fstream::app);

		if (outFile.is_open()==false)
		{
			cout << "File " << fileName.c_str() << " cannot be opened." << endl;
			getchar();
			exit(0);
		}

	};
};



