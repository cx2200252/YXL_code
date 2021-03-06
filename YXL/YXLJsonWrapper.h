#ifndef _YXL_JSON_READER_H_
#define _YXL_JSON_READER_H_

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <sstream>
//#include "YXLHelper.h"

#ifdef _WITH_WINDOWS_
#include <Windows.h>
#else
#ifdef _WITH_QT_
#include <QString>
#include <QTextCodec>
#endif
#endif

namespace YXL
{
	namespace JSON
	{

#ifdef _WITH_WINDOWS_
		inline std::string UTF8ToGBK(std::string utf8)
		{
			wchar_t * lpUnicodeStr = NULL;
			int nRetLen = 0;
			nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], -1, NULL, NULL);
			lpUnicodeStr = new WCHAR[nRetLen + 1];
			nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], -1, lpUnicodeStr, nRetLen);
			if (!nRetLen)
			{
				delete[] lpUnicodeStr;
				return 0;
			}
			nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, NULL, NULL, NULL, NULL);
			char* tmp = new char[nRetLen];
			nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, tmp, nRetLen, NULL, NULL);
			delete[] lpUnicodeStr;
			std::string ret = tmp;
			delete[] tmp;
			return ret;
		}

		inline std::string GBKToUTF8(std::string gbk)
		{
			wchar_t * lpUnicodeStr = NULL;
			int nRetLen = 0;
			nRetLen = ::MultiByteToWideChar(CP_ACP, 0, &gbk[0], -1, NULL, NULL);
			lpUnicodeStr = new WCHAR[nRetLen + 1];
			nRetLen = ::MultiByteToWideChar(CP_ACP, 0, &gbk[0], -1, lpUnicodeStr, nRetLen);
			if (!nRetLen)
			{
				delete[] lpUnicodeStr;
				return 0;
			}
			nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, NULL, 0, NULL, NULL);
			char* tmp = new char[nRetLen];
			nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, (char *)tmp, nRetLen, NULL, NULL);
			std::string ret = tmp;
			delete[] tmp;
			return ret;
		}
#else
#ifdef _WITH_QT_
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
#else
		inline std::string UTF8ToGBK(std::string utf8)
		{
			return utf8;
		}

		inline std::string GBKToUTF8(std::string gbk)
		{
			return gbk;
		}
#endif
#endif

		template<typename type>
		struct ValueParser {
			static rapidjson::Value Parse(const type& val, rapidjson::Document& doc)
			{
				//static_assert(false, "no parser");
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

#define _PARSER_IMPLEMENT(type, name) \
		template<>\
		struct ValueParser<type> {\
			static rapidjson::Value Parse(const type val, rapidjson::Document& doc)\
			{\
				rapidjson::Value v;\
				v.Set##name(val);\
				return v;\
			}\
		};

		_PARSER_IMPLEMENT(bool, Bool)
		_PARSER_IMPLEMENT(int, Int)
		_PARSER_IMPLEMENT(unsigned int, Uint)
		_PARSER_IMPLEMENT(int64_t, Int64)
		_PARSER_IMPLEMENT(uint64_t, Uint64)
		_PARSER_IMPLEMENT(float, Float)
		_PARSER_IMPLEMENT(double, Double)

#undef _PARSER_IMPLEMENT

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

#define JsonParseBool(val, doc) ToJsonValue(bool, val, doc)
#define JsonParseInt(val, doc) ToJsonValue(int, val, doc)
#define JsonParseUint(val, doc) ToJsonValue(unsigned int, val, doc)
#define JsonParseInt64(val, doc) ToJsonValue(int64_t, val, doc)
#define JsonParseUint64(val, doc) ToJsonValue(uint64_t, val, doc)
#define JsonParseFloat(val, doc) ToJsonValue(float, val, doc)
#define JsonParseDouble(val, doc) ToJsonValue(double, val, doc)
#define JsonParseString(val, doc) ToJsonValue(std::string, val, doc)
#define JsonParseCString(val, doc) ToJsonValue(const char*, val, doc)

#define JsonParseBoolVec(vals, doc) ToJsonValueVec(bool, vals, doc)
#define JsonParseIntVec(vals, doc) ToJsonValueVec(int, vals, doc)
#define JsonParseUintVec(vals, doc) ToJsonValueVec(unsigned int, vals, doc)
#define JsonParseInt64Vec(vals, doc) ToJsonValueVec(int64_t, vals, doc)
#define JsonParseUint64Vec(vals, doc) ToJsonValueVec(uint64_t, vals, doc)
#define JsonParseFloatVec(vals, doc) ToJsonValueVec(float, vals, doc)
#define JsonParseDoubleVec(vals, doc) ToJsonValueVec(double, vals, doc)
#define JsonParseStringVec(vals, doc) ToJsonValueVec(std::string, vals, doc)
#define JsonParseCStringVec(vals, doc) ToJsonValueVec(const char*, vals, doc)

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

#define _GETTER_IMPLEMENT(type, name)\
		template<>\
		struct ValueGetter<type> {\
			static type Get(const rapidjson::Value & val)\
			{\
				return val.Get##name();\
			}\
			static bool IsType(const rapidjson::Value & val)\
			{\
				return val.Is##name();\
			}\
		};

#define _GETTER_IMPLEMENT_V2(type, name, name2)\
		template<>\
		struct ValueGetter<type> {\
			static type Get(const rapidjson::Value & val)\
			{\
				return val.Is##name() ? val.Get##name() : (type)val.Get##name2();\
			}\
			static bool IsType(const rapidjson::Value & val)\
			{\
				return val.Is##name() || val.Is##name2();\
			}\
		};

		_GETTER_IMPLEMENT(bool, Bool)
		_GETTER_IMPLEMENT(int, Int)
		_GETTER_IMPLEMENT(unsigned int, Uint)
		_GETTER_IMPLEMENT(int64_t, Int64)
		_GETTER_IMPLEMENT(uint64_t, Uint64)
		_GETTER_IMPLEMENT_V2(float, Float, Int)
		_GETTER_IMPLEMENT_V2(double, Double, Int)

#undef _GETTER_IMPLEMENT
#undef _GETTER_IMPLEMENT_V2

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
#define JsonValIsUint(json_val) JsonValIsType(unsigned int, json_val)
#define JsonValIsInt64(json_val) JsonValIsType(int64_t, json_val)
#define JsonValIsUint64(json_val) JsonValIsType(uint64_t, json_val)
#define JsonValIsFloat(json_val) JsonValIsType(float, json_val)
#define JsonValIsDouble(json_val) JsonValIsType(double, json_val)
#define JsonValIsString(json_val) JsonValIsType(std::string, json_val)

#define JsonValIsBoolVec(json_val) JsonValIsTypeVec(bool, json_val)
#define JsonValIsIntVec(json_val) JsonValIsTypeVec(int, json_val)
#define JsonValIsUintVec(json_val) JsonValIsTypeVec(unsigned int, json_val)
#define JsonValIsInt64Vec(json_val) JsonValIsTypeVec(int64_t, json_val)
#define JsonValIsUint64Vec(json_val) JsonValIsTypeVec(uint64_t, json_val)
#define JsonValIsFloatVec(json_val) JsonValIsTypeVec(float, json_val)
#define JsonValIsDoubleVec(json_val) JsonValIsTypeVec(double, json_val)
#define JsonValIsStringVec(json_val) JsonValIsTypeVec(std::string, json_val)

#define JsonValHasMemberAndIsType(json_val, member, type) (json_val.HasMember(member)&&JsonValIsType(type, json_val[member]))
#define JsonValHasMemberAndIsTypeVec(json_val, member, type) (json_val.HasMember(member)&&JsonValIsTypeVec(type, json_val[member]))

#define JsonValHasMemberAndIsBool(json_val, member) JsonValHasMemberAndIsType(json_val, member, bool)
#define JsonValHasMemberAndIsInt(json_val, member) JsonValHasMemberAndIsType(json_val, member, int)
#define JsonValHasMemberAndIsUint(json_val, member) JsonValHasMemberAndIsType(json_val, member, unsigned int)
#define JsonValHasMemberAndIsInt64(json_val, member) JsonValHasMemberAndIsType(json_val, member, int64_t)
#define JsonValHasMemberAndIsUint64(json_val, member) JsonValHasMemberAndIsType(json_val, member, uint64_t)
#define JsonValHasMemberAndIsFloat(json_val, member) JsonValHasMemberAndIsType(json_val, member, float)
#define JsonValHasMemberAndIsDouble(json_val, member) JsonValHasMemberAndIsType(json_val, member, double)
#define JsonValHasMemberAndIsString(json_val, member) JsonValHasMemberAndIsType(json_val, member, std::string)

#define JsonValHasMemberAndIsBoolVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, bool)
#define JsonValHasMemberAndIsIntVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, int)
#define JsonValHasMemberAndIsUintVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, unsigned int)
#define JsonValHasMemberAndIsInt64Vec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, int64_t)
#define JsonValHasMemberAndIsUint64Vec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, uint64_t)
#define JsonValHasMemberAndIsFloatVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, float)
#define JsonValHasMemberAndIsDoubleVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, double)
#define JsonValHasMemberAndIsStringVec(json_val, member) JsonValHasMemberAndIsTypeVec(json_val, member, std::string)

#define JsonGetBool(json_val) FromJsonValue(bool, json_val)
#define JsonGetInt(json_val) FromJsonValue(int, json_val)
#define JsonGetUint(json_val) FromJsonValue(unsigned int, json_val)
#define JsonGetInt64(json_val) FromJsonValue(int64_t, json_val)
#define JsonGetUint64(json_val) FromJsonValue(uint64_t, json_val)
#define JsonGetFloat(json_val) FromJsonValue(float, json_val)
#define JsonGetDouble(json_val) FromJsonValue(double, json_val)
#define JsonGetString(json_val) FromJsonValue(std::string, json_val)

#define JsonGetBoolVec(json_val) FromJsonValueVec(bool, json_val)
#define JsonGetIntVec(json_val) FromJsonValueVec(int, json_val)
#define JsonGetUintVec(json_val) FromJsonValueVec(unsigned int, json_val)
#define JsonGetInt64Vec(json_val) FromJsonValueVec(int64_t, json_val)
#define JsonGetUint64Vec(json_val) FromJsonValueVec(uint64_t, json_val)
#define JsonGetFloatVec(json_val) FromJsonValueVec(float, json_val)
#define JsonGetDoubleVec(json_val) FromJsonValueVec(double, json_val)
#define JsonGetStringVec(json_val) FromJsonValueVec(std::string, json_val)

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
			static std::shared_ptr<Json> NewFromJSONContent(const std::string& content, bool with_comment = false)
			{
				auto ret = std::shared_ptr<Json>(new Json);
				ret->LoadFronJsonContent(content, with_comment);
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
						if(Esacpe(str).substr(0, 2) != "//")
							str_in += str + "\n";
				}
				else
				{
					std::cout << "create empty json file because file not exist: " << path << std::endl;
					str_in = "{\"author\":\"yixuan lu\"}";
				}
				fin.close();
				if (_doc.Parse(str_in.c_str()).HasParseError())
				{
					std::cout << "the file has been corrupted: " << path << std::endl;
					return false;
				}
				return true;
			}
			bool LoadFronJsonContent(const std::string& content, bool with_comment=false)
			{
				if (content.length() < 2)
				{
					std::cout << "no content" << std::endl;
					return false;
				}
				if (false == with_comment)
				{
					if (_doc.Parse(content.c_str()).HasParseError())
					{
						std::cout << "the json content has been corrupted: " << content << std::endl;
						return false;
					}
				}
				else
				{
					std::string tmp;
					tmp.reserve(content.length());
					bool escape = false;
					size_t pre_pos = 0;
					size_t cur_pos;
					while (std::string::npos != (cur_pos = content.find('\n', pre_pos)))
					{
						std::string line = Esacpe(content.substr(pre_pos, cur_pos - pre_pos+1));
						pre_pos = cur_pos+1;
						if (line.length()<2 || line.substr(0, 2) != "//")
							tmp += line;
					}
					std::string line = Esacpe(content.substr(pre_pos, content.length() - pre_pos));
					if (line.length()<2 || line.substr(0, 2) != "//")
						tmp += line;
					if (_doc.Parse(tmp.c_str()).HasParseError())
					{
						std::cout << "the json content has been corrupted: " << content << std::endl;
						return false;
					}
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
			void SaveBinary(std::ostream& out)
			{
				using namespace rapidjson;
				StringBuffer sb;
				PrettyWriter<StringBuffer> writer(sb);
				_doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

				std::string content = sb.GetString();
				int len = content.length();
				out.write((char*)&len, sizeof(len));
				out.write(content.c_str(), content.length());
			}
			void GetJsonContent(std::string& content) const 
			{
				using namespace rapidjson;
				StringBuffer sb;
				PrettyWriter<StringBuffer> writer(sb);
				_doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
				content = sb.GetString();
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

			void SetJSONValue(const std::string& path, rapidjson::Value& v)
			{
				auto Spilt = [](std::vector<std::string>& res, const std::string& src, const std::string& splitChars)
				{
					std::size_t prePos = src.find_first_not_of(splitChars);
					std::size_t pos = src.find_first_of(splitChars, prePos);
					while (std::string::npos != pos)
					{
						res.push_back(src.substr(prePos, pos - prePos));
						prePos = src.find_first_not_of(splitChars, pos + 1);
						pos = src.find_first_of(splitChars, prePos);
					}
					if (std::string::npos != prePos)
						res.push_back(src.substr(prePos, src.length() - prePos));
				};
				auto func = [&](const std::string& name, rapidjson::Value* par) {
					auto iter = par->FindMember(JsonParseString(name, _doc));
					if (iter != par->MemberEnd() && iter->value.IsObject() == false)
						par->EraseMember(iter);
					iter = par->FindMember(JsonParseString(name, _doc));
					if (iter == par->MemberEnd())
					{
						par->AddMember(JsonParseString(name, _doc), rapidjson::Value(rapidjson::kObjectType), _doc.GetAllocator());
						iter = par->FindMember(JsonParseString(name, _doc));
					}
					return &iter->value;
				};

				std::vector<std::string> node_path;
				Spilt(node_path, path, "/");

				auto dst = node_path.back();
				node_path.pop_back();

				rapidjson::Value* par = &_doc;
				for (auto v : node_path)
					par = func(v, par);

				auto name = JsonParseString(dst, _doc);
				if (par->HasMember(name))
					par->EraseMember(dst.c_str());
				par->AddMember(name, v, _doc.GetAllocator());
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
				while (parent.HasMember(name.c_str()))
					parent.RemoveMember(name.c_str());
				return AddMember(name, vals, cnt, parent);
			}
			template<typename type> bool SetMember(const std::string& name, const type* vals, const int cnt)
			{
				return SetMember(name, vals, cnt, _doc);
			}
			template<typename type> bool SetMember(const std::string& name, std::vector<type>& vals, rapidjson::Value& parent)
			{
				if(vals.empty())
					return SetMember(name, (type*)nullptr, vals.size(), parent);
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
					if (dest.empty())
						return true;
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
			std::string Esacpe(const std::string& str, const std::string escape_ch = "\t ")
			{
				auto beg = str.find_first_not_of(escape_ch);
				auto end = str.find_last_not_of(escape_ch);
				if (std::string::npos == beg || std::string::npos == end)
					return "";
				return str.substr(beg, end - beg + 1);
			}

		private:
			rapidjson::Document _doc;
			rapidjson::Value _none_val = rapidjson::Value(rapidjson::Type::kNullType);
		};
	}
}
#endif // !_YXL_JSON_READER_H_
