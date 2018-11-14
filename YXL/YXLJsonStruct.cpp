#include "YXLJsonStruct.h"


namespace YXL
{
	namespace JSON
	{

#define _Load_Repeat(T) if(T::IsType(val["value"])) \
	{\
		auto v=T::Load(val["value"]);\
		((T*)v.get())->ToVector(tmp);\
		for(int i(0); i < repeat; ++i)\
			ret->_values.insert(ret->_values.end(), tmp.begin(), tmp.end());\
		return std::shared_ptr<ParamBase>(ret);\
	}
#define _PARAM_REPEAT_IMPL(type, T)\
		bool ParamRepeat##T::IsType(const rapidjson::Value & val)\
		{\
			bool ret = val.IsObject() && JsonValHasMemberAndIsInt(val, "repeat") && val.HasMember("value");\
			if (ret)\
				ret &= Param##T::IsType(val["value"]) || ParamRange##T::IsType(val["value"]) || ParamArray##T::IsType(val["value"]) || ParamRepeat##T::IsType(val["value"]);\
			return ret;\
		}\
		std::shared_ptr<ParamBase> ParamRepeat##T::Load(const rapidjson::Value & val)\
		{\
			auto ret = new ParamRepeat##T;\
			int repeat = (std::max)(1, JsonGetInt(val["repeat"]));\
			std::vector<type> tmp;\
			_Load_Repeat(Param##T);\
			_Load_Repeat(ParamRange##T);\
			_Load_Repeat(ParamArray##T);\
			_Load_Repeat(ParamRepeat##T);\
			return std::shared_ptr<ParamBase>(ret);\
		}
#define _PARAM_REPEAT_IMPL2(type, T)\
		bool ParamRepeat##T::IsType(const rapidjson::Value & val)\
		{\
			bool ret = val.IsObject() && JsonValHasMemberAndIsInt(val, "repeat") && val.HasMember("value");\
			if (ret)\
				ret &= Param##T::IsType(val["value"]) || ParamArray##T::IsType(val["value"]) || ParamRepeat##T::IsType(val["value"]);\
			return ret;\
		}\
		std::shared_ptr<ParamBase> ParamRepeat##T::Load(const rapidjson::Value & val)\
		{\
			auto ret = new ParamRepeat##T;\
			int repeat = (std::max)(1, JsonGetInt(val["repeat"]));\
			std::vector<type> tmp;\
			_Load_Repeat(Param##T);\
			_Load_Repeat(ParamArray##T);\
			_Load_Repeat(ParamRepeat##T);\
			return std::shared_ptr<ParamBase>(ret);\
		}

		_PARAM_REPEAT_IMPL(int, Int)
		_PARAM_REPEAT_IMPL(unsigned, Uint)
		_PARAM_REPEAT_IMPL(int64_t, Int64)
		_PARAM_REPEAT_IMPL(uint64_t, Uint64)
		_PARAM_REPEAT_IMPL(float, Float)
		_PARAM_REPEAT_IMPL(double, Double)
		_PARAM_REPEAT_IMPL2(bool, Bool)
		_PARAM_REPEAT_IMPL2(std::string, String)

#undef _PARAM_REPEAT_IMPL
#undef _PARAM_REPEAT_IMPL2
#undef _Load_Repeat

#define _Load_Array(T) if(T::IsType(*iter)) \
	{\
		auto v=T::Load(*iter);\
		((T*)v.get())->ToVector(tmp);\
		ret->_values.insert(ret->_values.end(), tmp.begin(), tmp.end());\
	}
#define _PARAM_ARRAY_IMPL(type, T)\
		bool ParamArray##T::IsType(const rapidjson::Value & val)\
		{\
			bool ret = val.IsArray();\
			if (ret)\
			{\
				for (auto iter = val.Begin(); iter != val.End(); ++iter)\
					ret &= (Param##T::IsType(*iter) || ParamRange##T::IsType(*iter) || ParamRepeat##T::IsType(*iter) || (ParamArray##T::IsType(*iter)));\
			}\
			return ret;\
		}\
		std::shared_ptr<ParamBase> ParamArray##T::Load(const rapidjson::Value & val)\
		{\
			auto ret = new ParamArray##T;\
			std::vector<type> tmp;\
			for (auto iter = val.Begin(); iter != val.End(); ++iter)\
			{\
				_Load_Array(Param##T)\
				else _Load_Array(ParamRange##T)\
				else _Load_Array(ParamArray##T)\
				else _Load_Array(ParamRepeat##T);\
			}\
			return std::shared_ptr<ParamBase>(ret);\
		}
#define _PARAM_ARRAY_IMPL2(type, T)\
		bool ParamArray##T::IsType(const rapidjson::Value & val)\
		{\
			bool ret = val.IsArray();\
			if (ret)\
			{\
				for (auto iter = val.Begin(); iter != val.End(); ++iter)\
					ret &= (Param##T::IsType(*iter) || ParamRepeat##T::IsType(*iter) || (ParamArray##T::IsType(*iter)));\
			}\
			return ret;\
		}\
		std::shared_ptr<ParamBase> ParamArray##T::Load(const rapidjson::Value & val)\
		{\
			auto ret = new ParamArray##T;\
			std::vector<type> tmp;\
			for (auto iter = val.Begin(); iter != val.End(); ++iter)\
			{\
				_Load_Array(Param##T)\
				else _Load_Array(ParamArray##T)\
				else _Load_Array(ParamRepeat##T);\
			}\
			return std::shared_ptr<ParamBase>(ret);\
		}

		_PARAM_ARRAY_IMPL(int, Int)
		_PARAM_ARRAY_IMPL(unsigned, Uint)
		_PARAM_ARRAY_IMPL(int64_t, Int64)
		_PARAM_ARRAY_IMPL(uint64_t, Uint64)
		_PARAM_ARRAY_IMPL(float, Float)
		_PARAM_ARRAY_IMPL(double, Double)
		_PARAM_ARRAY_IMPL2(bool, Bool)
		_PARAM_ARRAY_IMPL2(std::string, String)

#undef _PARAM_ARRAY_IMPL
#undef _PARAM_ARRAY_IMPL2
#undef _Load_Array



		
	}
}