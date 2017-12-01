# A simple json library with c++

源代码在 `json\json.hpp`。

更多测试见 `json\test.cpp` 。

## c++ 版本问题：

使用了 `std::variant(c++17)`，所以编译需要支持 c++17 的编译器。

如果需要可以换成 `boost::variant`。需要改动以下接口。

`std::get => boost::get`， `std::variant::index() => boost::variant::which()` 。



## 数据类型：

|    json     |             c++              |
| :---------: | :--------------------------: |
|    null     |          nullptr_t           |
| true, false |             bool             |
|   number    |            double            |
|   string    |         std::string          |
|    array    |     std::vector\<json\>      |
|   object    | std::map\<std::string, json> |





## 例子：

```c++
// 头文件及命名空间
#include "json.hpp"

using namespace Json;
```



### 直接从值构造 json：

```c++
#include "json.hpp"
using namespace Json;	// 我在这里偷一些懒

json j0; 				// null
json j1 = nullptr;
json j2 = true;
json j3 = false;
json j4 = 42;
json j5 = "\nabc";
json j6 = json::array_t{ nullptr, true, 42, "abc", json::array_t{} };
json j7 = json::object_t{ "name" : "czn", "age" : 21 };
```



### 从字符串解析：

```c++
#include "json.hpp"
using namespace Json;

json j1 = json::parse("null");		// #1
json j2 = "null"_json;				// #2
json j3;
std::cin >> j3;						// #3, 这里输入内容不能有空白字符

json j4 = "[null, true, 42, \"abc\", []]"_json;

json j5 = " { "
        "\"n\" : null , "
        "\"f\" : false , "
        "\"t\" : true , "
        "\"i\" : 123 , "
        "\"s\" : \"abc\", "
        "\"a\" : [1, 2, 3],"
        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
        " } "_json;
```



### 生成字符串

```c++
#include "json.hpp"
using namespace Json;

json j1 = json::array_t{ nullptr, true, 42, "abc", json::array_t{} };

std::string str = j1.dump();  // [null, true, 42.000000, "abc", []]

std::cout << j1 << std::endl; // 直接输出 j1.dump() 到 console

```



### 访问 json 中的数据

```c++
json j1 = nullptr;
json j2 = false;
json j3 = 42;
json j4 = "\nabc";
json j5 = json::array_t{ nullptr, true, 42, "abc", json::array_t{} };
json j6 = json::object_t{ "name" : "czn", "age" : 21 };

// 所有成员都有 get<json_type_t>
auto v1 = j1.get<json::null_t>();
bool v2 = j2.get<json::boolean_t>();
double v3 = j3.get<json::number_t>();
auto v4 = j4.get<json::string_t>();
auto v5 = j5.get<json::array_t>();
auto v6 = j6.get<json::object_t>();

// 访问数组
json v7 = j5[0];
j5[1] = false;

// 访问对象
json v8 = j6["age"];
j6["age"] = 20;
```



### 其他接口

```c++
size();				// return error: 0; null, boolean, number, string: 1; array, object: n;
get_type();			// return enum class json_type
type_name();		// return const char* type name
is_null();			// return bool
is_boolean();
is_number();
is_string();
is_array();
is_object();
is_error();			// parse fail

lhs.swap(rhs);
Json::swap(lhs, rhs);
```

