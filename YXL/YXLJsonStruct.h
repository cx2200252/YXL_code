#pragma once
#include "YXLJsonWarpper.h"
#include "YXLHelper.h"
#include <memory>

namespace YXL
{
	namespace JSON
	{
		class ParamBase
		{
		protected:
			enum class Type
			{
				Int,
				Uint,
				Int64,
				Uint64,
				Float,
				Double,
				Bool,
				String,
			};

		public:
			ParamBase(Type type) :_type(type) {}
			virtual ~ParamBase() {}

#define _GET_FUN(type, T)\
			virtual type Get##T(const int idx)\
			{\
				yxlout << "[error]this param is not type of "<< #type << std::endl;\
				return 0;\
			}\
			virtual bool Is##T()\
			{\
				return false;\
			}

			_GET_FUN(int, Int)
			_GET_FUN(unsigned int, Uint)
			_GET_FUN(int64_t, Int64)
			_GET_FUN(uint64_t, Uint64)
			_GET_FUN(float, Float)
			_GET_FUN(double, Double)
			_GET_FUN(bool, Bool)
			_GET_FUN(std::string, String)

#undef _GET_FUN

		private:
			Type _type;
		};

#define _DEF_PARAM_BASE(type, T) \
		class ParamBase##T :public ParamBase\
		{\
		public:\
			ParamBase##T() :ParamBase(Type::T) {}\
			virtual void ToVector(std::vector<type>& vec) = 0;\
			virtual bool Is##T()\
			{\
				return true;\
			}\
		};

		_DEF_PARAM_BASE(int, Int)
		_DEF_PARAM_BASE(unsigned, Uint)
		_DEF_PARAM_BASE(int64_t, Int64)
		_DEF_PARAM_BASE(uint64_t, Uint64)
		_DEF_PARAM_BASE(float, Float)
		_DEF_PARAM_BASE(double, Double)
		_DEF_PARAM_BASE(bool, Bool)
		_DEF_PARAM_BASE(std::string, String)

#undef _DEF_PARAM_BASE

#define _DEF_PARAM_T(type, T)\
		class Param##T : public ParamBase##T\
		{\
		public:\
			Param##T(){}\
			virtual type Get##T(const int idx)\
			{\
				return _value;\
			}\
			virtual void ToVector(std::vector<type>& vec)\
			{\
				vec.clear();\
				vec.push_back(_value);\
			}\
			static bool IsType(const rapidjson::Value& val)\
			{\
				return JsonValIs##T(val);\
			}\
			static std::shared_ptr<ParamBase> Load(const rapidjson::Value& val)\
			{\
				auto ret = new Param##T;\
				ret->_value = JsonGet##T(val);\
				return std::shared_ptr<ParamBase>(ret);\
			}\
		private:\
			type _value;\
		};

		_DEF_PARAM_T(int, Int)
		_DEF_PARAM_T(unsigned, Uint)
		_DEF_PARAM_T(int64_t, Int64)
		_DEF_PARAM_T(uint64_t, Uint64)
		_DEF_PARAM_T(float, Float)
		_DEF_PARAM_T(double, Double)
		_DEF_PARAM_T(bool, Bool)
		_DEF_PARAM_T(std::string, String)

#undef _DEF_PARAM_T

#define _DEF_PARAM_RANGE(type, T) \
		class ParamRange##T : public ParamBase##T\
		{\
		public:\
			virtual type Get##T(const int idx)\
			{\
				return (std::min)(idx*step + from, to);\
			}\
			virtual void ToVector(std::vector<type>& vec)\
			{\
				vec.clear();\
				if (step > 0)\
				{\
					for (type cur(from); cur <= to; cur += step)\
						vec.push_back(cur);\
				}\
				else\
				{\
					for (type cur(from); cur >= to; cur += step)\
						vec.push_back(cur);\
				}\
			}\
			static bool IsType(const rapidjson::Value& val)\
			{\
				return val.IsObject() && JsonValHasMemberAndIs##T(val, "from") && JsonValHasMemberAndIs##T(val, "to") && JsonValHasMemberAndIs##T(val, "step");\
			}\
			static std::shared_ptr<ParamBase> Load(const rapidjson::Value& val)\
			{\
				auto ret = new ParamRange##T;\
				ret->from = JsonGet##T(val["from"]);\
				ret->to = JsonGet##T(val["to"]);\
				ret->step = JsonGet##T(val["step"]);\
				if ((ret->to - ret->from)*ret->step <= 0)\
					yxlout << "[error]range error: [" << ret->from << ":" << ret->step << ":" << ret->to << "]" << std::endl;\
				return std::shared_ptr<ParamBase>(ret);\
			}\
		private:\
			type from, to, step;\
		};

		_DEF_PARAM_RANGE(int, Int)
		_DEF_PARAM_RANGE(unsigned, Uint)
		_DEF_PARAM_RANGE(int64_t, Int64)
		_DEF_PARAM_RANGE(uint64_t, Uint64)
		_DEF_PARAM_RANGE(float, Float)
		_DEF_PARAM_RANGE(double, Double)

#undef _DEF_PARAM_RANGE


#define _DEF_PARAM_REPEAT(type, T)\
		class ParamRepeat##T : public ParamBase##T\
		{\
		public:\
			virtual type Get##T(const int idx)\
			{\
				return idx < 0 ? _values[0] : (idx >= _values.size() ? *_values.rbegin() : _values[idx]);\
			}\
			virtual void ToVector(std::vector<type>& vec)\
			{\
				vec.assign(_values.begin(), _values.end());\
			}\
			static bool IsType(const rapidjson::Value& val);\
			static std::shared_ptr<ParamBase> Load(const rapidjson::Value& val);\
		private:\
			std::vector<type> _values;\
		};

		_DEF_PARAM_REPEAT(int, Int)
		_DEF_PARAM_REPEAT(unsigned, Uint)
		_DEF_PARAM_REPEAT(int64_t, Int64)
		_DEF_PARAM_REPEAT(uint64_t, Uint64)
		_DEF_PARAM_REPEAT(float, Float)
		_DEF_PARAM_REPEAT(double, Double)
		_DEF_PARAM_REPEAT(bool, Bool)
		_DEF_PARAM_REPEAT(std::string, String)

#undef _DEF_PARAM_REPEAT

#define _DEF_PARAM_ARRAY(type, T)\
		class ParamArray##T : public ParamBase##T\
		{\
		public:\
			virtual type Get##T(const int idx)\
			{\
				return idx < 0 ? _values[0] : (idx >= _values.size() ? *_values.rbegin() : _values[idx]);\
			}\
			virtual void ToVector(std::vector<type>& vec)\
			{\
				vec.assign(_values.begin(), _values.end());\
			}\
			static bool IsType(const rapidjson::Value& val);\
			static std::shared_ptr<ParamBase> Load(const rapidjson::Value& val);\
		private:\
			std::vector<type> _values;\
		};

		_DEF_PARAM_ARRAY(int, Int)
		_DEF_PARAM_ARRAY(unsigned, Uint)
		_DEF_PARAM_ARRAY(int64_t, Int64)
		_DEF_PARAM_ARRAY(uint64_t, Uint64)
		_DEF_PARAM_ARRAY(float, Float)
		_DEF_PARAM_ARRAY(double, Double)
		_DEF_PARAM_ARRAY(bool, Bool)
		_DEF_PARAM_ARRAY(std::string, String)

#undef _DEF_PARAM_ARRAY


		//getter
		template <>
		struct ValueGetter<std::shared_ptr<ParamBase>> {
			static std::shared_ptr<ParamBase> Get(const rapidjson::Value & val)
			{
				std::shared_ptr<ParamBase> ret = nullptr;

#define _Load(T) if(T::IsType(val)) \
	{\
		ret=T::Load(val);\
	}
#define _Load2(T) \
				_Load(Param##T)\
				else _Load(ParamRange##T)\
				else _Load(ParamRepeat##T)\
				else _Load(ParamArray##T)
#define _Load3(T) \
				_Load(Param##T)\
				else _Load(ParamRepeat##T)\
				else _Load(ParamArray##T)

				_Load2(Int)
				else _Load2(Uint)
				else _Load2(Int64)
				else _Load2(Uint64)
				else _Load2(Float)
				else _Load2(Double)
				else _Load3(Bool)
				else _Load3(String)
#undef _Load
#undef _Load2
#undef _Load3

				return ret;
			}
			static bool IsType(const rapidjson::Value & val)
			{
				bool ret = true;

#define _IsType(T)\
	ret |= Param##T::IsType(val) || ParamRange##T::IsType(val) || ParamRepeat##T::IsType(val) || ParamArray##T::IsType(val);
#define _IsType2(T)\
	ret |= Param##T::IsType(val) || ParamRepeat##T::IsType(val) || ParamArray##T::IsType(val);

				_IsType(Int);
				_IsType(Uint);
				_IsType(Int64);
				_IsType(Uint64);
				_IsType(Float);
				_IsType(Double);
				_IsType2(Bool);
				_IsType2(String);
#undef _IsType
#undef _IsType2

				return ret;
			}
		};

		//reading sample
		/*
		{
			"a": 1.0,
			"b":{
				"from": 2.0,
				"to": 3.0,
				"step": 0.1
			},
			"c":{
				"value": {
					"from": 4.0,
					"to": 5.0,
					"step": 0.1
				},
				"repeat": 3
			},
			"d":[
				5.0,
				{
					"value": 6.0,
					"repeat": 3
				},
				{
					"from": 7.0,
					"to": 8.0,
					"step": 0.1
				}
			]
		}

		auto json = YXL::JSON::Json::New(fn_json);
		auto& root = json->GetRoot();
		std::map<std::string, std::shared_ptr<YXL::JSON::ParamBase>> items;
		for (auto iter = root.MemberBegin(); iter != root.MemberEnd(); ++iter)
		{
			auto name = JsonGetStr(iter->name);
			auto tmp = json->ReadValue(name, std::shared_ptr<YXL::JSON::ParamBase>(nullptr));
			if (tmp)
				items[name] = tmp;
		}
		*/
	}
}

