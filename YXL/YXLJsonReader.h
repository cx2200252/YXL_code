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
		inline std::string UTF8ToGBK(std::string utf8)
		{
			wchar_t * lpUnicodeStr = NULL;
			int nRetLen = 0;

			nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], -1, NULL, NULL);  //获取转换到Unicode编码后所需要的字符空间长度
			lpUnicodeStr = new WCHAR[nRetLen + 1];  //为Unicode字符串空间
			nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], -1, lpUnicodeStr, nRetLen);  //转换到Unicode编码
			if (!nRetLen)  //转换失败则出错退出
			{
				delete[] lpUnicodeStr;
				return 0;
			}

			nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, NULL, NULL, NULL, NULL);  //获取转换到GBK编码后所需要的字符空间长度

			char* tmp = new char[nRetLen];


			nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, tmp, nRetLen, NULL, NULL);  //转换到GBK编码

			delete[] lpUnicodeStr;

			std::string ret = tmp;
			delete[] tmp;

			return ret;
		}

		inline std::string GBKToUTF8(std::string gbk)
		{
			wchar_t * lpUnicodeStr = NULL;
			int nRetLen = 0;

			nRetLen = ::MultiByteToWideChar(CP_ACP, 0, &gbk[0], -1, NULL, NULL);  //获取转换到Unicode编码后所需要的字符空间长度
			lpUnicodeStr = new WCHAR[nRetLen + 1];  //为Unicode字符串空间
			nRetLen = ::MultiByteToWideChar(CP_ACP, 0, &gbk[0], -1, lpUnicodeStr, nRetLen);  //转换到Unicode编码
			if (!nRetLen)  //转换失败则出错退出
			{
				delete[] lpUnicodeStr;
				return 0;
			}

			nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, NULL, 0, NULL, NULL);  //获取转换到UTF8编码后所需要的字符空间长度

			char* tmp = new char[nRetLen];

			nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, (char *)tmp, nRetLen, NULL, NULL);  //转换到UTF8编码

			std::string ret = tmp;
			delete[] tmp;

			return ret;
		}

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

		//write

		template<typename type>
		struct ValueParser {
			type Parse(const type& val, rapidjson::Document& doc) {
				return val;
			}
		};

		template<>
		struct ValueParser<std::string> {
			std::string str;
			rapidjson::Value Parse(const std::string& val, rapidjson::Document& doc) {
				str = GBKToUTF8(val);
				rapidjson::Value v;
				v.Set(str.c_str(), doc.GetAllocator());
				return v;
			}
		};

		template<>
		struct ValueParser<const char*> {
			std::string str;
			rapidjson::Value Parse(const char* val, rapidjson::Document& doc) {
				str = GBKToUTF8(std::string(val));
				rapidjson::Value v;
				v.Set(str.c_str(), doc.GetAllocator());
				return v;
			}
		};

		template<typename type> void SetMember(rapidjson::Document& doc, rapidjson::Value & jsonVal, const std::string& name, type val, ValueParser<type> parser=ValueParser<type>())
		{
			//jsonVal[name.c_str()].Set(val, doc.GetAllocator());
			if (jsonVal.HasMember(name.c_str()))
			{
				jsonVal.RemoveMember(name.c_str());
			}
			{
				rapidjson::Value v;
				v.Set(name.c_str(), doc.GetAllocator());
				jsonVal.AddMember(v, parser.Parse(val, doc), doc.GetAllocator());
			}
		}

		template<typename type> bool SetMemberArray(rapidjson::Document& doc, rapidjson::Value & jsonVal, const std::string& name, type* vals, const int cnt, ValueParser<type> parser = ValueParser<type>())
		{
			if (0 >= cnt)
			{
				return false;
			}

			rapidjson::Value v(rapidjson::Type::kArrayType);
			for (int i(0); i != cnt; ++i)
			{
				v.PushBack(parser.Parse(vals[i], doc), doc.GetAllocator());
			}

			if (jsonVal.HasMember(name.c_str()))
			{
				jsonVal.RemoveMember(name.c_str());
			}
			{
				rapidjson::Value key;
				key.Set(name.c_str(), doc.GetAllocator());
				jsonVal.AddMember(key, v, doc.GetAllocator());
			}
			return true;
		}

		template<typename type> bool SetMemberArray(rapidjson::Document& doc, rapidjson::Value & jsonVal, const std::string& name, std::vector<type>& vals, ValueParser<type> parser = ValueParser<type>())
		{
			return SetMemberArray(doc, jsonVal, name, &vals[0], vals.size(), parser);
		}


		//read

		template<typename type>
		struct ValueGetter {
			static type Get(const rapidjson::Value & val) {
				return type();
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return false;
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

		template<typename type> bool ReadRapidJsonValue(type* dest, const int cnt, const rapidjson::Value& val, const std::string& name, type def_val = type(), ValueGetter<type> getter = ValueGetter<type>())
		{
			if (val.HasMember(name.c_str()) && val[name.c_str()].IsArray() && val[name.c_str()].Size() == cnt)
			{
				int idx(0);
				for (auto iter = val[name.c_str()].Begin(); iter != val[name.c_str()].End() && idx < cnt; ++iter, ++idx)
				{
					if (false == getter.IsType(*iter))
					{
						return false;
					}
					dest[idx] = getter.Get(*iter);
				}
				return idx == cnt;
			}
			else
			{
				return false;
			}
		}

		template<typename type> bool ReadRapidJsonValue(std::vector<type>& dest, const rapidjson::Value& val, const std::string& name, type def_val = type(), ValueGetter<type> getter = ValueGetter<type>())
		{
			if (val.HasMember(name.c_str()) && val[name.c_str()].IsArray())
			{
				dest.resize(val[name.c_str()].Size());
				return ReadRapidJsonValue(&dest[0], dest.size(), val, name, def_val, getter);
			}
			else
			{
				return false;
			}
		}

		//
	}
}
