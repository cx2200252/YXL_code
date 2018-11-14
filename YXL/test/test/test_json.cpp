#include "../../YXLJsonWarpper.h"
#include "../../YXLHelper.h"
#include "../../YXLJsonStruct.h"
#include "test.h"


struct TestNode
{
	float f;
	int i;
	std::string s;
	std::vector<int> vec;
};

namespace YXL
{
	namespace JSON
	{
		template<> struct ValueParser<TestNode>
		{
			std::string str;
			static rapidjson::Value Parse(const TestNode& val, rapidjson::Document& doc)
			{
				rapidjson::Value value(rapidjson::Type::kObjectType);
				auto& alloc = doc.GetAllocator();

				value.AddMember(JsonParseCString("f", doc), val.f, alloc);
				value.AddMember(JsonParseCString("i", doc), val.i, alloc);
				value.AddMember(JsonParseCString("s", doc), JsonParseString(val.s, doc), alloc);
				value.AddMember(JsonParseCString("vecI", doc), ToJsonValueVec(int, val.vec, doc), alloc);

				return value;
			}
		};

		template <> struct ValueGetter<TestNode>
		{
			static TestNode Get(const rapidjson::Value & val)
			{
				TestNode node;
				node.f = JsonGetFloat(val["f"]);
				node.i = JsonGetInt(val["i"]);
				node.s = JsonGetString(val["s"]);
				node.vec = FromJsonValueVec(int, val["vecI"]);
				return node;
			}
			static bool IsType(const rapidjson::Value & val)
			{
				//check all members' type if necessary
				return JsonValHasMemberAndIsFloat(val, "f")
					&& JsonValHasMemberAndIsInt(val, "i")
					&& JsonValHasMemberAndIsString(val, "s")
					&& JsonValHasMemberAndIsIntVec(val, "vecI");
			}
		};
	}
}

namespace YXL_TEST
{
	void TestJson()
	{
		const std::string test_json = GetTestDir() + "test_json.json";
		//if(false)
		{
			if(true==YXL::File::FileExist(test_json))
				YXL::File::RmFile(test_json.c_str());
			//create an empty json if the file not exist
			//in this case, create an empty json
			auto json = YXL::JSON::Json::New(test_json);

			//single value read/write
			{
				json->AddMember("float", 0.0f);//{"float":0.0}
				json->AddMember("float", 1.0f);//{"float":0.0, "float":1.0}
											   //will remove all member with the same name
				json->SetMember("float", 2.0f);//{"float":2.0}
				auto f0 = json->ReadValue("float", -1.0f);
				auto f1 = json->ReadValue("float1", -1.0f);//not exist, use default value (param 2)
				std::cout << "f0: " << f0 << "\nf1: " << f1 << std::endl;

				rapidjson::Value v(rapidjson::Type::kObjectType);
				json->AddJSONValue("obj", v);//{"float":2.0,"obj":{}}
											 //must use reference
				auto& v_new = json->GetJSONValue("obj");
				json->SetMember("string", std::string("string0"), v_new);//{"float":2.0,"obj":{"string":"string0"}}
				json->SetMember("string2", std::string("Â¬ÞÈäÖ"), v_new);//{"float":2.0,"obj":{"string":"string0","string2":"Â¬ÞÈäÖ"}}

				auto str = json->ReadValue<std::string>("string", "", v_new);
				auto str2 = json->ReadValue<std::string>("string2", "", v_new);
				std::cout << "\nstr: " << str << "\nstr2: " << str2 << std::endl;
			}
			//array value read/write
			{
				int vals[] = { 0, 1,2 };
				std::vector<int> vecVals({ 3,4,5 });

				json->SetMember("array", vals, 3);//{"float":2.0,"obj":{"string":"string0","string2":"Â¬ÞÈäÖ"},"array":[0,1,2]}
				json->SetMember("array2", vecVals);//{"float":2.0,"obj":{"string":"string0","string2":"Â¬ÞÈäÖ"},"array":[0,1,2],"array2":[3,4,5]}

				int vals2[3];
				json->ReadValue(vals2, 3, "array");
				std::vector<int> vecVals2;
				json->ReadValue(vecVals2, "array2");

				std::cout << std::endl;
				for (auto val : vals2)
					std::cout << val << " ";
				std::cout << std::endl;
				for (auto val : vecVals2)
					std::cout << val << " ";
				std::cout << std::endl;
			}
			//customized type
			{
				std::vector<TestNode> nodes{
					{ 0.0f, 1, "abc",{ 2,3,4 } },
					{ 5.0f, 6, "def",{ 7,8,9 } },
				};

				//need full specialization of parser for TestNode
				json->SetMember("test_node", nodes);
				/*
				{
				"float":2.0,
				"obj":{"string":"string0","string2":"Â¬ÞÈäÖ"},
				"array":[0,1,2],
				"array2":[0,1,2],
				"test_node":[{"f":0.0,"i":1,"s":"abc","vecI":[2,3,4]}, {"f":5.0,"i":6,"s":"def","vecI":[7,8,9]}]
				}
				*/
				std::vector<TestNode> nodes2;
				//need full specialization of gettter for TestNode
				json->ReadValue(nodes2, "test_node");

				std::cout << std::endl;
				for (auto& node : nodes2)
				{
					std::cout << "f: " << node.f << "\ti: " << node.i << "\ts: " << node.s << "\tvec: [";
					for (int i(0); i != node.vec.size(); ++i)
					{
						if (i)
							std::cout << ",";
						std::cout << node.vec[i];
					}
					std::cout << "]" << std::endl;
				}
			}

			json->Save(test_json);
		}

		//test json struct
		{
			auto json = YXL::JSON::Json::New(GetTestDir()+"test_struct.json");
			auto& root = json->GetRoot();
			std::map<std::string, std::shared_ptr<YXL::JSON::ParamBase>> items;
			for (auto iter = root.MemberBegin(); iter != root.MemberEnd(); ++iter)
			{
				auto name = JsonGetString(iter->name);
				auto tmp = json->ReadValue(name, std::shared_ptr<YXL::JSON::ParamBase>(nullptr));
				if (tmp)
					items[name] = tmp;
			}
		}
	}
}