#pragma once
#include <rapidjson\document.h>
#include <rapidjson\prettywriter.h>
#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

namespace YXL
{
	namespace JSON
	{
		inline rapidjson::Document LoadJson(std::string path)
		{
			rapidjson::Document doc;
			{
				std::ifstream fin(path);
				std::string str_in;
				if (fin.is_open())
				{
					std::string str;
					while (std::getline(fin, str))
					{
						str_in += str + "\n";
					}
				}
				else
				{
					str_in = "{\"author\":\"yixuan lu\"}";
				}
				if (doc.Parse(str_in.c_str()).HasParseError())
				{
					std::cout << "the project file has been corrupted: " << path << std::endl;
				}
				fin.close();
			}
			return doc;
		}

		inline void SaveJson(rapidjson::Document& doc, const std::string& path)
		{
			using namespace rapidjson;
			StringBuffer sb;
			PrettyWriter<StringBuffer> writer(sb);
			doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

			std::ofstream fout(path);
			fout << sb.GetString() << std::endl;
			fout.close();
		}

		inline std::string UTF8ToGBK(std::string utf8)
		{
			wchar_t * lpUnicodeStr = NULL;
			int nRetLen = 0;

			nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], -1, NULL, NULL);  //��ȡת����Unicode���������Ҫ���ַ��ռ䳤��
			lpUnicodeStr = new WCHAR[nRetLen + 1];  //ΪUnicode�ַ����ռ�
			nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], -1, lpUnicodeStr, nRetLen);  //ת����Unicode����
			if (!nRetLen)  //ת��ʧ��������˳�
			{
				delete[] lpUnicodeStr;
				return 0;
			}

			nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, NULL, NULL, NULL, NULL);  //��ȡת����GBK���������Ҫ���ַ��ռ䳤��

			char* tmp = new char[nRetLen];


			nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, tmp, nRetLen, NULL, NULL);  //ת����GBK����

			delete[] lpUnicodeStr;

			std::string ret = tmp;
			delete[] tmp;

			return ret;
		}

		template<typename type>
		struct ValueGetter {
			static type Get(const rapidjson::Value & val) {
				return type();
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsBool();
			}
		};

		template <>
		struct ValueGetter<int> {
			static int Get(const rapidjson::Value & val) {
				return val.GetInt();
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsInt();
			}
		};

		template<>
		struct ValueGetter<bool> {
			static bool Get(const rapidjson::Value & val) {
				return val.GetBool();
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsBool();
			}
		};

		template<>
		struct ValueGetter<float> {
			static float Get(const rapidjson::Value & val) {
				return val.GetFloat();
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsFloat();
			}
		};

		template<>
		struct ValueGetter<std::string> {
			static std::string Get(const rapidjson::Value & val) {
				return UTF8ToGBK(val.GetString());
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsString();
			}
		};

		template<typename type> type ReadRapidJsonValue(const rapidjson::Value& val, const std::string& name, type def_val = type(), ValueGetter<type> getter = ValueGetter<type>())
		{
			if (val.HasMember(name.c_str()) && getter.IsType(val[name.c_str()]))
			{
				return getter.Get(val[name.c_str()]);
			}
			else
			{
				return def_val;
			}
		}
	}
}
