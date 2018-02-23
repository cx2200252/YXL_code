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

		inline std::string GBKToUTF8(std::string gbk)
		{
			wchar_t * lpUnicodeStr = NULL;
			int nRetLen = 0;
			nRetLen = ::MultiByteToWideChar(CP_ACP, 0, &gbk[0], -1, NULL, NULL);  //��ȡת����Unicode���������Ҫ���ַ��ռ䳤��
			lpUnicodeStr = new WCHAR[nRetLen + 1];  //ΪUnicode�ַ����ռ�
			nRetLen = ::MultiByteToWideChar(CP_ACP, 0, &gbk[0], -1, lpUnicodeStr, nRetLen);  //ת����Unicode����
			if (!nRetLen)  //ת��ʧ��������˳�
			{
				delete[] lpUnicodeStr;
				return 0;
			}
			nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, NULL, 0, NULL, NULL);  //��ȡת����UTF8���������Ҫ���ַ��ռ䳤��
			char* tmp = new char[nRetLen];
			nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, (char *)tmp, nRetLen, NULL, NULL);  //ת����UTF8����
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
			const type& Parse(const type& val, rapidjson::Document& doc) 
			{
				return val;
			}
		};

		template<typename type>
		struct ValueParser<std::vector<type> > {
			rapidjson::Value Parse(const std::vector<type>& vals, rapidjson::Document& doc)
			{
				rapidjson::Value ret(rapidjson::Type::kArrayType);
				ValueParser<type> parser;
				for (auto val : vals)
					ret.PushBack(parser.Parse(val, doc), doc.GetAllocator());
				return ret;
			}
		};

		template<>
		struct ValueParser<std::string> {
			std::string str;
			rapidjson::Value Parse(const std::string& val, rapidjson::Document& doc) 
			{
				str = GBKToUTF8(val);
				rapidjson::Value v;
				v.Set(str.c_str(), doc.GetAllocator());
				return v;
			}
		};

		template<>
		struct ValueParser<const char*> {
			std::string str;
			rapidjson::Value Parse(const char* val, rapidjson::Document& doc) 
			{
				str = GBKToUTF8(std::string(val));
				rapidjson::Value v;
				v.Set(str.c_str(), doc.GetAllocator());
				return v;
			}
		};

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
				ValueParser<type> parser;
				parent.AddMember(v, parser.Parse(val, _doc), _doc.GetAllocator());
			}
			template<typename type> void AddMember(const std::string& name, const type& val)
			{
				AddMember(name, val, _doc);
			}
			template<typename type> bool AddMember(const std::string& name, const type* vals, const int cnt, rapidjson::Value& parent)
			{
				if (0 >= cnt)
					return false;
				ValueParser<type> parser = ValueParser<type>();
				rapidjson::Value v(rapidjson::Type::kArrayType);
				for (int i(0); i != cnt; ++i)
					v.PushBack(parser.Parse(vals[i], _doc), _doc.GetAllocator());
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
				ValueGetter<type> getter = ValueGetter<type>();
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
			template<typename type> bool ReadValue(type* dest, const int cnt, const std::string& name, const rapidjson::Value& parent = rapidjson::Value(rapidjson::Type::kNullType))
			{
				ValueGetter<type> getter = ValueGetter<type>();
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
