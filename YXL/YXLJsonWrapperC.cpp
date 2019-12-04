#include "YXLJsonWarpper.h"
#include "YXLJsonWarpperC.h"

YXL::JSON::Json* AsJson(void* json)
{
	return (YXL::JSON::Json*)json;
}

#ifdef __cplusplus
extern "C"
{
#endif

void* CJsonLoad(const char* data, const int len)
{
	if (nullptr == data || 0 == len)
		return nullptr;
	auto ret = new YXL::JSON::Json;
	ret->LoadFronJsonContent(std::string(data, len));
	return ret;
}

void* CJsonGetRoot(void* json)
{
	if (nullptr == json)
		return nullptr;
	auto& root = AsJson(json)->GetRoot();
	return &root;
}

void* CJsonGetChild(void* par, const char* child)
{
	if (nullptr == par || nullptr == child)
		return nullptr;
	rapidjson::Value& val = *(rapidjson::Value*)par;
	if (false == val.HasMember(child))
		return nullptr;
	auto& ret = val[child];
	return &ret;
}

int CJsonGetInt(void* node)
{
	if (nullptr == node)
		return 0;
	rapidjson::Value& val = *(rapidjson::Value*)node;
	if (false == val.IsInt())
		return 0;
	return val.GetInt();
}

void CJsonGetInt2(void* node, int* dst)
{
	if (nullptr == node)
		return;
	rapidjson::Value& val = *(rapidjson::Value*)node;
	if (false == JsonValIsIntVec(val))
		return;
	std::vector<int> v = JsonGetIntVec(val);
	if (v.size() >= 2)
	{
		dst[0] = v[0];
		dst[1] = v[1];
	}
}

void CJsonRelease(void* json)
{
	delete AsJson(json);
}

#ifdef __cplusplus
}
#endif