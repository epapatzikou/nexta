#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
using namespace std;
using std::string;
using std::ifstream;
using std::vector;
using std::map;
using std::istringstream;
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
private:
	char Delimiter;
	bool IsFirstLineHeader;
	ifstream inFile;
	vector<string> LineFieldsValue;
	vector<string> Headers;
	map<string,int> FieldsIndices;

	vector<string> ParseLine(string line);

public:
	int m_EmptyLineCount ;
	bool m_bSynchroSingleCSVFile;
	string m_SynchroSectionName;
	bool m_bLastSectionRead;

	bool ReadSectionHeader();

	bool m_bSkipFirstLine;  // for synchro CSV files
	CCSVParser(void);
	bool OpenCSVFile(string fileName, bool bIsFirstLineHeader = true);
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

			int size = (int)(LineFieldsValue.size());
			if(FieldsIndices[field_name]>= size)
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


	void WriteHeader()
	{
		if (!outFile.is_open()) 
			return;


		//for(unsigned int i = 0; i< FieldsIndices.size(); i++)
		//{
		//outFile << LineFieldsCategoryName[i] << ",";
		//}
		//outFile << endl;

		//if(row_title == true)
		//	outFile << ",";

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

	bool CCSVWriter::Open(string fileName)
	{
		outFile.open(fileName.c_str());

		if (outFile.is_open()==false)
		{

			CString str;
			str.Format("File %s cannot be opened.", fileName.c_str());
			AfxMessageBox(str);
			return false;
		}

		return true;
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


