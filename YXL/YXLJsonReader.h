#pragma once
#undef min
#undef max
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
//#include "YXLHelper.h"

#ifndef _NO_WINDOWS_
#include <Windows.h>
#else
#include <QString>
#include <QTextCodec>
#endif

namespace YXL
{
	namespace JSON
	{

#ifndef _NO_WINDOWS_
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
#else
		inline std::string UTF8ToGBK(std::string utf8)
		{
			QTextCodec *gbk_codec = QTextCodec::codecForName("gbk");
			QTextCodec *utf8_codec = QTextCodec::codecForName("UTF-8");
			auto tmp = utf8_codec->toUnicode(utf8.c_str());
			auto ret = gbk_codec->fromUnicode(tmp);
			return ret.data();
		}

		inline std::string GBKToUTF8(std::string gbk)
		{
			QTextCodec *gbk_codec = QTextCodec::codecForName("gbk");
			QTextCodec *utf8_codec = QTextCodec::codecForName("UTF-8");
			auto tmp = gbk_codec->toUnicode(gbk.c_str());
			auto ret = utf8_codec->fromUnicode(tmp);
			return ret.data();
		}
#endif

		template<typename type>
		struct ValueParser {
			static const type& Parse(const type& val, rapidjson::Document& doc) 
			{
				return val;
			}
		};

		template<typename type>
		struct ValueParser<std::vector<type> > {
			static rapidjson::Value Parse(const std::vector<type>& vals, rapidjson::Document& doc)
			{
				rapidjson::Value ret(rapidjson::Type::kArrayType);
				for (auto val : vals)
					ret.PushBack(ValueParser<type>::Parse(val, doc), doc.GetAllocator());
				return ret;
			}
		};

		template<>
		struct ValueParser<std::string> {
			static rapidjson::Value Parse(const std::string& val, rapidjson::Document& doc) 
			{
				auto str = GBKToUTF8(val);
				rapidjson::Value v;
				v.Set(str.c_str(), doc.GetAllocator());
				return v;
			}
		};

		template<>
		struct ValueParser<const char*> {
			static rapidjson::Value Parse(const char* val, rapidjson::Document& doc) 
			{
				auto str = GBKToUTF8(std::string(val));
				rapidjson::Value v;
				v.Set(str.c_str(), doc.GetAllocator());
				return v;
			}
		};

		template<>
		struct ValueParser<rapidjson::Value> {
			static rapidjson::Value Parse(const rapidjson::Value& val, rapidjson::Document& doc)
			{
				rapidjson::Value ret;
				ret.CopyFrom(val, doc.GetAllocator(), true);
				return ret;
			}
		};

#define ToJsonValue(type, val, doc) YXL::JSON::ValueParser<type>::Parse(val, doc)
#define ToJsonValueVec(type, vals, doc) YXL::JSON::ValueParser<std::vector<type> >::Parse(vals, doc)

#define JsonParseStr(val, doc) ToJsonValue(std::string, val, doc)
#define JsonParseCStr(val, doc) ToJsonValue(const char*, val, doc)
#define JsonParseVecStr(vals, doc) ToJsonValueVec(std::string, vals, doc)
#define JsonParseVecCStr(vals, doc) ToJsonValueVec(const char*, vals, doc)

#define JsonParseBoolVec(vals, doc) ToJsonValueVec(bool, vals, doc)
#define JsonParseIntVec(vals, doc) ToJsonValueVec(int, vals, doc)
#define JsonParseFloatVec(vals, doc) ToJsonValueVec(float, vals, doc)


		//read

		template<typename type>
		struct ValueGetter {
			static type Get(const rapidjson::Value & val) 
			{
				return type();
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return false;
			}
		};

		template<typename type>
		struct ValueGetter<std::vector<type> > {
			static std::vector<type> Get(const rapidjson::Value & val)
			{
				std::vector<type> ret;
				ret.reserve(val.Size());
				for (auto iter = val.Begin(); iter != val.End(); ++iter)
					ret.push_back(ValueGetter<type>::Get(*iter));
				return ret;
			}
			static bool IsType(const rapidjson::Value & val)
			{
				if (false == val.IsArray())
					return false;
				for (auto iter = val.Begin(); iter != val.End(); ++iter)
					if (false == ValueGetter<type>::IsType(*iter))
						return false;
				return true;
			}
		};

		template <>
		struct ValueGetter<int> {
			static int Get(const rapidjson::Value & val) 
			{
				return val.GetInt();
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsInt();
			}
		};

		template<>
		struct ValueGetter<bool> {
			static bool Get(const rapidjson::Value & val) 
			{
				return val.GetBool();
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsBool();
			}
		};

		template<>
		struct ValueGetter<float> {
			static float Get(const rapidjson::Value & val) 
			{
				return val.GetFloat();
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsFloat();
			}
		};

		template<>
		struct ValueGetter<std::string> {
			static std::string Get(const rapidjson::Value & val) 
			{
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
				std::cout << "const char* not supported..." << std::endl;
				return false;
			}
		};

#define JsonValIsType(type, json_val) YXL::JSON::ValueGetter<type>::IsType(json_val)
#define JsonValIsTypeVec(type, json_val) YXL::JSON::ValueGetter<std::vector<type> >::IsType(json_val)
#define FromJsonValue(type, json_val) YXL::JSON::ValueGetter<type>::Get(json_val)
#define FromJsonValueVec(type, json_val) YXL::JSON::ValueGetter<std::vector<type> >::Get(json_val)

#define JsonValIsBool(json_val) JsonValIsType(bool, json_val)
#define JsonValIsInt(json_val) JsonValIsType(int, json_val)
#define JsonValIsFloat(json_val) JsonValIsType(float, json_val)
#define JsonValIsStr(json_val) JsonValIsType(std::string, json_val)

#define JsonValIsBoolVec(json_val) JsonValIsTypeVec(bool, json_val)
#define JsonValIsIntVec(json_val) JsonValIsTypeVec(int, json_val)
#define JsonValIsFloatVec(json_val) JsonValIsTypeVec(float, json_val)
#define JsonValIsStrVec(json_val) JsonValIsTypeVec(std::string, json_val)

#define JsonValHasMemberAndIsType(json_val, member, type) (json_val.HasMember(member)&&JsonValIsType(type, json_val[member]))
#define JsonValHasMemberAndIsTypeVec(json_val, member, type) (json_val.HasMember(member)&&JsonValIsTypeVec(type, json_val[member]))

#define JsonValHasMemberAndIsBool(json_val, member) JsonValHasMemberAndIsType(json_val, member, bool)
#define JsonValHasMemberAndIsInt(json_val, member) JsonValHasMemberAndIsType(json_val, member, int)
#define JsonValHasMemberAndIsFloat(json_val, member) JsonValHasMemberAndIsType(json_val, member, float)
#define JsonValHasMemberAndIsStr(json_val, member) JsonValHasMemberAndIsType(json_val, member, std::string)

#define JsonValHasMemberAndIsBoolVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, bool)
#define JsonValHasMemberAndIsIntVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, int)
#define JsonValHasMemberAndIsFloatVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, float)
#define JsonValHasMemberAndIsStrVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, std::string)

#define JsonGetBool(json_val) FromJsonValue(bool, json_val)
#define JsonGetInt(json_val) FromJsonValue(int, json_val)
#define JsonGetFloat(json_val) FromJsonValue(float, json_val)
#define JsonGetStr(json_val) FromJsonValue(std::string, json_val)

#define JsonGetBoolVec(json_val) FromJsonValueVec(bool, json_val)
#define JsonGetIntVec(json_val) FromJsonValueVec(int, json_val)
#define JsonGetFloatVec(json_val) FromJsonValueVec(float, json_val)
#define JsonGetStrVec(json_val) FromJsonValueVec(std::string, json_val)

		//
		class Json
		{
		public:
			static std::shared_ptr<Json> New(const std::string& path)
			{
				auto ret = std::shared_ptr<Json>(new Json);
				ret->Load(path);
				return ret;
			}
			static std::shared_ptr<Json> NewFromJSONContent(const std::string& content)
			{
				auto ret = std::shared_ptr<Json>(new Json);
				ret->LoadFronJsonContent(content);
				return ret;
			}

		public:
			bool Load(const std::string path)
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
					std::cout << "create empty json file because file not exist: " << path << std::endl;
					str_in = "{\"author\":\"yixuan lu\"}";
				}
				fin.close();
				if (_doc.Parse(str_in.c_str()).HasParseError())
				{
					std::cout << "the project file has been corrupted: " << path << std::endl;
					return false;
				}
				return true;
			}
			bool LoadFronJsonContent(const std::string& content)
			{
				if (_doc.Parse(content.c_str()).HasParseError())
				{
					std::cout << "the json content has been corrupted: " << content << std::endl;
					return false;
				}
				return true;
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

		public:
			rapidjson::Value& GetJSONValue(const std::string& name, rapidjson::Value& parent)
			{
				if (false == parent.HasMember(name.c_str()))
				{
					return _none_val;
				}
				return parent[name.c_str()];
			}
			rapidjson::Value& GetJSONValue(const std::string& name)
			{
				return GetJSONValue(name, _doc);
			}
			void AddJSONValue(const std::string& name, rapidjson::Value& val, rapidjson::Value& parent)
			{
				rapidjson::Value v;
				v.Set(name.c_str(), _doc.GetAllocator());
				parent.AddMember(v, val, _doc.GetAllocator());
			}
			void AddJSONValue(const std::string& name, rapidjson::Value& val)
			{
				AddJSONValue(name, val, _doc);
			}

		public:
			//write (overwrite)
			template<typename type> void SetMember(const std::string& name, const type& val, rapidjson::Value& parent)
			{
				while (parent.HasMember(name.c_str()))
					parent.RemoveMember(name.c_str());
				AddMember(name, val, parent);
			}
			template<typename type> void SetMember(const std::string& name, const type& val)
			{
				SetMember(name, val, _doc);
			}
			//write (overwrite)
			template<typename type> bool SetMember(const std::string& name, const type* vals, const int cnt, rapidjson::Value& parent)
			{
				if (0 >= cnt)
					return false;
				while (parent.HasMember(name.c_str()))
					parent.RemoveMember(name.c_str());
				return AddMember(name, vals, cnt, parent);
			}
			template<typename type> bool SetMember(const std::string& name, const type* vals, const int cnt)
			{
				return AddMember(name, vals, cnt, _doc);
			}
			template<typename type> bool SetMember(const std::string& name, std::vector<type>& vals, rapidjson::Value& parent)
			{
				return SetMember(name, &vals[0], vals.size(), parent);
			}
			template<typename type> bool SetMember(const std::string& name, std::vector<type>& vals)
			{
				return SetMember(name, vals, _doc);
			}

			//write (append)
			template<typename type> void AddMember(const std::string& name, const type& val, rapidjson::Value& parent)
			{
				rapidjson::Value v;
				v.Set(name.c_str(), _doc.GetAllocator());
				parent.AddMember(v, ToJsonValue(type, val, _doc), _doc.GetAllocator());
			}
			template<typename type> void AddMember(const std::string& name, const type& val)
			{
				AddMember(name, val, _doc);
			}
			template<typename type> bool AddMember(const std::string& name, const type* vals, const int cnt, rapidjson::Value& parent)
			{
				if (0 >= cnt)
					return false;
				rapidjson::Value v(rapidjson::Type::kArrayType);
				for (int i(0); i != cnt; ++i)
					v.PushBack(ToJsonValue(type, vals[i], _doc), _doc.GetAllocator());
				rapidjson::Value key;
				key.Set(name.c_str(), _doc.GetAllocator());
				parent.AddMember(key, v, _doc.GetAllocator());
				return true;
			}
			template<typename type> bool AddMember(const std::string& name, const type* vals, const int cnt)
			{
				return AddMember(name, vals, cnt, _doc);
			}
			template<typename type> bool AddMember(const std::string& name, std::vector<type>& vals, rapidjson::Value& parent)
			{
				return AddMember(name, &vals[0], (int)vals.size(), parent);
			}
			template<typename type> bool AddMember(const std::string& name, std::vector<type>& vals)
			{
				return AddMember(name, vals, _doc);
			}

			//read
			template<typename type> type ReadValue(const std::string& name, type def_val = type(), const rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType))
			{
				const rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;
				if (par.HasMember(name.c_str()) && JsonValIsType(type, par[name.c_str()]))
					return FromJsonValue(type, par[name.c_str()]);
				else
					return def_val;
			}
			template<typename type> bool ReadValue(type* dest, const int cnt, const std::string& name, const rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType))
			{
				const rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;
				if (par.HasMember(name.c_str()) && par[name.c_str()].IsArray() && par[name.c_str()].Size() == cnt)
				{
					int idx(0);
					for (auto iter = par[name.c_str()].Begin(); iter != par[name.c_str()].End() && idx < cnt; ++iter, ++idx)
					{
						if (false == JsonValIsType(type, *iter))
							return false;
						dest[idx] = FromJsonValue(type, *iter);
					}
					return idx == cnt;
				}
				else
					return false;
			}
			template<typename type> bool ReadValue(std::vector<type>& dest, const std::string& name, const rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType))
			{
				const rapidjson::Value& par = (parent.GetType() != rapidjson::Type::kNullType) ? parent : _doc;
				if (par.HasMember(name.c_str()) && par[name.c_str()].IsArray())
				{
					dest.resize(par[name.c_str()].Size());
					return ReadValue(&dest[0], dest.size(), name, par);
				}
				else
					return false;
			}

		public:
			rapidjson::Value& GetRoot()
			{
				return _doc;
			}
			rapidjson::Document& GetDoc()
			{
				return _doc;
			}

		private:
			rapidjson::Document _doc;
			rapidjson::Value _none_val = rapidjson::Value(rapidjson::Type::kNullType);
		};
	}
}
