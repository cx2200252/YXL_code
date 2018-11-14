#include "YXLReportGenerator.h"

namespace YXL
{
	namespace JSON
	{
		template<>
		struct ValueParser<Report::ReportTable> {
			static rapidjson::Value Parse(const Report::ReportTable& val, rapidjson::Document& doc)
			{
				auto& allocator = doc.GetAllocator();
				rapidjson::Value v(rapidjson::kObjectType);
				v.AddMember(JsonParseString("height", doc), JsonParseFloat(val._height, doc), allocator);
				v.AddMember(JsonParseString("ratio", doc), JsonParseFloatVec(val._ratio, doc), allocator);
				v.AddMember(JsonParseString("type", doc), JsonParseStringVec(val._type, doc), allocator);
				v.AddMember(JsonParseString("rows", doc), ToJsonValueVec(std::vector<std::string>, val._rows, doc), allocator);

				rapidjson::Value setting(rapidjson::kArrayType);

				std::vector<rapidjson::Value> tmp;
				for (int i(0); i != val._setting_float.size(); ++i)
					tmp.push_back(rapidjson::Value(rapidjson::kObjectType));

				for (int i(0); i != val._setting_float.size(); ++i)
				{
					auto& t = tmp[i];
					for (auto pair : val._setting_float[i])
						t.AddMember(JsonParseString(pair.first, doc), JsonParseFloat(pair.second, doc), allocator);
				}
				for (int i(0); i != val._setting_str.size(); ++i)
				{
					auto& t = tmp[i];
					for (auto pair : val._setting_str[i])
						t.AddMember(JsonParseString(pair.first, doc), JsonParseString(pair.second, doc), allocator);
				}
				for (auto& t : tmp)
					setting.PushBack(t, allocator);

				v.AddMember(JsonParseString("setting", doc), setting, allocator);
				return v;
			}
		};
	}

	namespace Report
	{
		void ReportTable::SaveJson(const std::string& fn)
		{
			auto json = YXL::JSON::Json::New("");
			auto& doc = json->GetDoc();
			auto& root = json->GetRoot();
			auto v = ToJsonValue(ReportTable, *this, doc);

			root.Swap(v);
			std::string content;
			json->GetJsonContent(content);
			content = YXL::JSON::UTF8ToGBK(content);
			std::ofstream fout(fn);
			fout << content;
			fout.close();
		}
	}
}