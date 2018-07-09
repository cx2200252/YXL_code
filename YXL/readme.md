# outlines
- [YXLJsonReader](#yxljsonreader)
- [YXLHelper](#yxlhelper)
- [TriMesh](#trimesh)
- [MeshRenderer](#meshrenderer)
- [MeshSimplify](#meshsimplify)
- [YXLWebpWarpper](#yxlwebpwarpper)

## YXLJsonReader
A C++ warpper of [rapidjson](https://github.com/Tencent/rapidjson) for easy json read/write and it is header only.

#### example
##### initlization
```C
//open a json file or
//create an empty json if the file not exist
auto json = YXL::JSON::Json::New("test.json");
```
##### basic type read/write
```C
//suppose this is an empty json

/*
read/write at the root node
*/
//write
json->AddMember("float", 0.0f);
//the json will be: {"float":0.0}
json->AddMember("float", 1.0f);
//the json will be: {"float":0.0, "float":1.0}
//the following call will remove all member with the same name
json->SetMember("float", 2.0f);
//the json will be: {"float":2.0}

//read
//if the member not exist, will return the default value (param 2)
auto f0 = json->ReadValue("float", -1.0f);
//f0=2.0
auto f1 = json->ReadValue("float1", -1.0f);
//f1=-1.0

/*
read/write at sub-node
*/
rapidjson::Value v(rapidjson::Type::kObjectType);
json->AddJSONValue("obj", v);
//the json will be: {"float":2.0,"obj":{}}
//must use reference
auto& v_new = json->GetJSONValue("obj");
json->SetMember("string", std::string("string0"), v_new);
//the json will be: {"float":2.0,"obj":{"string":"string0"}}
json->SetMember("string2", std::string("卢奕渲"), v_new);
//the json will be: {"float":2.0,"obj":{"string":"string0","string2":"卢奕渲"}}

auto str = json->ReadValue<std::string>("string", "", v_new);
//str=string0
auto str2 = json->ReadValue<std::string>("string2", "", v_new);
//str2="卢奕渲"
```
##### array read/write
```C
//suppose this is an empty json

int vals[] = { 0, 1,2 };
std::vector<int> vecVals({3,4,5});

json->SetMember("array", vals, 3);
//the json will be: {"array":[0,1,2]}
json->SetMember("array2", vecVals);
//the json will be: {"array":[0,1,2],"array2":[3,4,5]}

//read to array
int vals2[3];
json->ReadValue(vals2, 3, "array");
//vals2={0,1,2};
//read to vector
std::vector<int> vecVals2;
json->ReadValue(vecVals2, "array2");
//vecVals2={3,4,5};
```
##### struct read/write
```C
struct TestNode
{
	float f;
	int i;
	std::string s;
	std::vector<int> vec;
};

//implement getter/parser for TestNode
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

				value.AddMember(JsonParseCStr("f", doc), val.f, alloc);
				value.AddMember(JsonParseCStr("i", doc), val.i, alloc);
				value.AddMember(JsonParseCStr("s", doc), JsonParseStr(val.s, doc), alloc);
				value.AddMember(JsonParseCStr("vecI", doc),
					ToJsonValueVec(int, val.vec, doc), alloc);

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
				node.s = JsonGetStr(val["s"]);
				node.vec = FromJsonValueVec(int, val["vecI"]);
				return node;
			}
			static bool IsType(const rapidjson::Value & val)
			{
				//check all members' type if necessary
				return JsonValHasMemberAndIsFloat(val, "f")
					&& JsonValHasMemberAndIsInt(val, "i")
					&& JsonValHasMemberAndIsStr(val, "s")
					&& JsonValHasMemberAndIsIntVec(val, "vecI");
			}
		};
	}
}

//read/write
//suppose this is an empty json

std::vector<TestNode> nodes{
	{ 0.0f, 1, "abc",{ 2,3,4 } },
	{ 5.0f, 6, "def",{ 7,8,9 } },
};
//need implement full specialization of parser for TestNode
json->SetMember("test_node", nodes);
/*
the json will be:
{
  "test_node":[
    {"f":0.0,"i":1,"s":"abc","vecI":[2,3,4]},
    {"f":5.0,"i":6,"s":"def","vecI":[7,8,9]}
  ]
}
*/
std::vector<TestNode> nodes2;
//need implement full specialization of gettter for TestNode
json->ReadValue(nodes2, "test_node");
```

## YXLHelper
useful functions/classes.
### marco to switch on/off
```C
#define _YXL_OTHER_
#define _YXL_FILES_
#define _YXL_STRING_
#define _YXL_PARAM_PARSER_
#define _YXL_PRINT_
#define _YXL_OUT_STREAM_
#define _YXL_UNION_FIND_
#define _YXL_KD_TREE_
#define _YXL_TIME_
#define _YXL_CONSOLE_
#define _YXL_TRANSFORM_
#define _YXL_GRAPHIC_
#define _YXL_IMG_PROC_
//#define _YXL_GLFW_
```

## TriMesh
class for loading model.
- mixed triangle/quad in one file
- remove unused points   
- find connected componenets
- converte to renderable topology
- consistency of in/out files

## MeshRenderer
class to render TriMesh.
- call & actual render can run in differt threads
  - define _USE_OWN_GL_CONTEXT_
  - return render result as a cv::Mat

## MeshSimplify
mesh simplify with:
- fix some vertices
- tolerance: #face or #vertex

## YXLWebpWarpper
webp read/write