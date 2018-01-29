#pragma once
#include <rapidjson\document.h>
#include <rapidjson\prettywriter.h>
#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <YXLHelper.h>

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

		template<>
		struct ValueGetter<const char*> {
			static std::string Get(const rapidjson::Value & val) {
				return "";
			}
			static bool IsType(const rapidjson::Value & val)
			{
				YXL::yxlout << YXL_LOG_PREFIX << "const char* not supported..." << std::endl;
				return false;
			}
		};

		//
		class Json
		{
		public:
			void Load(std::string path)
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
				if (_doc.Parse(str_in.c_str()).HasParseError())
				{
					std::cout << "the project file has been corrupted: " << path << std::endl;
				}
				fin.close();
			}
			void LoadFronJsonContent(const std::string& content)
			{
				if (_doc.Parse(content.c_str()).HasParseError())
				{
					std::cout << "the json content has been corrupted: " << content << std::endl;
				}
			}
			void Save(const std::string& path)
			{
				using namespace rapidjson;
				StringBuffer sb;
				PrettyWriter<StringBuffer> writer(sb);
				_doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

				std::ofstream fout(path);
				fout << sb.GetString() << std::endl;
				fout.close();
			}

			rapidjson::Value& GetValue(const std::string& name, rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType))
			{
				rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;
				if (false == par.HasMember(name.c_str()))
				{
					return _none_val;
				}
				return par[name.c_str()];
			}

			void AddValue(const std::string& name, rapidjson::Value& val, rapidjson::Value& parent=rapidjson::Value(rapidjson::Type::kNullType))
			{
				rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;

				rapidjson::Value v;
				v.Set(name.c_str(), _doc.GetAllocator());
				par.AddMember(v, val, _doc.GetAllocator());
			}


			//write (overwrite)
			template<typename type> void SetMember(const std::string& name, type val, rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType), ValueParser<type> parser = ValueParser<type>())
			{
				rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;
				if (par.HasMember(name.c_str()))
				{
					par.RemoveMember(name.c_str());
				}
				AddMember(name, val, parent, parser);
			}

			template<typename type> bool SetMemberArray(const std::string& name, type* vals, const int cnt, rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType), ValueParser<type> parser = ValueParser<type>())
			{
				if (0 >= cnt)
				{
					return false;
				}

				rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;

				if (par.HasMember(name.c_str()))
				{
					par.RemoveMember(name.c_str());
				}

				return AddMemberArray(name, vals, cnt, parent, parser);
			}

			template<typename type> bool SetMemberArray(const std::string& name, std::vector<type>& vals, rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType), ValueParser<type> parser = ValueParser<type>())
			{
				return SetMemberArray(name, &vals[0], vals.size(), parent, parser);
			}

			//write (append)

			template<typename type> void AddMember(const std::string& name, type val, rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType), ValueParser<type> parser = ValueParser<type>())
			{
				rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;
				rapidjson::Value v;
				v.Set(name.c_str(), _doc.GetAllocator());
				par.AddMember(v, parser.Parse(val, _doc), _doc.GetAllocator());
			}

			template<typename type> bool AddMemberArray(const std::string& name, type* vals, const int cnt, rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType), ValueParser<type> parser = ValueParser<type>())
			{
				if (0 >= cnt)
				{
					return false;
				}
				rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;

				rapidjson::Value v(rapidjson::Type::kArrayType);
				for (int i(0); i != cnt; ++i)
				{
					v.PushBack(parser.Parse(vals[i], _doc), _doc.GetAllocator());
				}

				{
					rapidjson::Value key;
					key.Set(name.c_str(), _doc.GetAllocator());
					par.AddMember(key, v, _doc.GetAllocator());
				}
				return true;
			}

			template<typename type> bool AddMemberArray(const std::string& name, std::vector<type>& vals, rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType), ValueParser<type> parser = ValueParser<type>())
			{
				return AddMemberArray(name, &vals[0], (int)vals.size(), parent, parser);
			}

			//read
			template<typename type> type ReadValue(const std::string& name, type def_val = type(), const rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType), ValueGetter<type> getter = ValueGetter<type>())
			{
				const rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;
				if (par.HasMember(name.c_str()) && getter.IsType(par[name.c_str()]))
				{
					return getter.Get(par[name.c_str()]);
				}
				else
				{
					return def_val;
				}
			}

			template<typename type> bool ReadValue(type* dest, const int cnt, const std::string& name, type def_val = type(), const rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType), ValueGetter<type> getter = ValueGetter<type>())
			{
				const rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;
				if (par.HasMember(name.c_str()) && par[name.c_str()].IsArray() && par[name.c_str()].Size() == cnt)
				{
					int idx(0);
					for (auto iter = par[name.c_str()].Begin(); iter != par[name.c_str()].End() && idx < cnt; ++iter, ++idx)
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

			template<typename type> bool ReadValue(std::vector<type>& dest, const std::string& name, type def_val = type(), rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType), ValueGetter<type> getter = ValueGetter<type>())
			{
				const rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;
				if (par.HasMember(name.c_str()) && par[name.c_str()].IsArray())
				{
					dest.resize(par[name.c_str()].Size());
					return ReadRapidJsonValue(&dest[0], dest.size(), par, name, def_val, getter);
				}
				else
				{
					return false;
				}
			}

			rapidjson::Value& GetRoot()
			{
				return _doc;
			}

		private:
			rapidjson::Document _doc;
			rapidjson::Value _none_val = rapidjson::Value(rapidjson::Type::kNullType);
		};
	}
}
