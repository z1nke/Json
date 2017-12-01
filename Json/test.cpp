#include "json.hpp"
#include <iostream>

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

using std::cout;
using std::endl;
using std::cerr;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define UNIT_TEST_BASE(equality, expect, actual)        \
    do                                                  \
    {                                                   \
        test_count++;                                   \
        if (equality)                                   \
            test_pass++;                                \
        else                                            \
        {                                               \
            cerr << __FILE__ << ":" << __LINE__         \
                << ":  except: " << expect              \
                << " actual: " << actual << endl;       \
            main_ret = 1;                               \
        }                                               \
    } while (0)

#define UNIT_TEST(expect, actual) UNIT_TEST_BASE((expect) == (actual), (expect), (actual))

using namespace Json;

void test_null()
{
    json j1;
    UNIT_TEST(true, j1.is_null());
    json j2 = nullptr;
    UNIT_TEST(true, j2.is_null());

    json j3 = json::parse("null");
    UNIT_TEST(true, j3.is_null());
}

void test_boolean()
{
    json j1 = true;
    UNIT_TEST(true, j1.is_boolean());
    UNIT_TEST("true", j1.dump());
    UNIT_TEST(true, j1.get_value<json::boolean_t>());

    json j2 = false;
    UNIT_TEST(true, j2.is_boolean());
    UNIT_TEST("false", j2.dump());
    UNIT_TEST(false, j2.get_value<json::boolean_t>());

    json j3 = json::parse("true");
    UNIT_TEST(true, j3.is_boolean());
    UNIT_TEST(true, j3.get_value<json::boolean_t>());

    json j4 = json::parse("false");
    UNIT_TEST(true, j4.is_boolean());
    UNIT_TEST(false, j4.get_value<json::boolean_t>());

    json j5 = json::parse("true ");
    UNIT_TEST(true, j5.is_boolean());
    UNIT_TEST(true, j5.get_value<json::boolean_t>());
}

#define TEST_NUMBER_VALUE(val)                          \
    do                                                  \
    {                                                   \
        json j = val;                                   \
        UNIT_TEST(true, j.is_number());                 \
        UNIT_TEST(static_cast<double>(val),             \
            j.get_value<json::number_t>());             \
    } while (0)

#define TEST_NUMBER_PARSE(val, str)                     \
    do                                                  \
    {                                                   \
        json j = json::parse(str);                      \
        UNIT_TEST(true, j.is_number());                 \
        UNIT_TEST(val, j.get_value<json::number_t>());  \
    } while (0) 

#define TEST_STRINGIFY(str)                             \
    do                                                  \
    {                                                   \
        json j = json::parse(str);                      \
        std::string s1 = j.dump();                      \
        json j2 = json::parse(s1);                      \
        std::string s2 = j2.dump();                     \
        UNIT_TEST(s1, s2);                              \
    } while (0)

void test_number()
{
    TEST_NUMBER_VALUE(0);
    TEST_NUMBER_VALUE(1.5);
    TEST_NUMBER_VALUE(1e10);
    TEST_NUMBER_VALUE(1e-10000);

    TEST_NUMBER_PARSE(0.0, "0");
    TEST_NUMBER_PARSE(0.0, "-0");
    TEST_NUMBER_PARSE(0.0, "-0.0");
    TEST_NUMBER_PARSE(1.0, "1");
    TEST_NUMBER_PARSE(-1.0, "-1");
    TEST_NUMBER_PARSE(1.5, "1.5");
    TEST_NUMBER_PARSE(-1.5, "-1.5");
    TEST_NUMBER_PARSE(3.1416, "3.1416");
    TEST_NUMBER_PARSE(1E10, "1E10");
    TEST_NUMBER_PARSE(1e10, "1e10");
    TEST_NUMBER_PARSE(1E+10, "1E+10");
    TEST_NUMBER_PARSE(1E-10, "1E-10");
    TEST_NUMBER_PARSE(-1E10, "-1E10");
    TEST_NUMBER_PARSE(-1e10, "-1e10");
    TEST_NUMBER_PARSE(-1E+10, "-1E+10");
    TEST_NUMBER_PARSE(-1E-10, "-1E-10");
    TEST_NUMBER_PARSE(1.234E+10, "1.234E+10");
    TEST_NUMBER_PARSE(1.234E-10, "1.234E-10");
    TEST_NUMBER_PARSE(0.0, "1e-10000"); // underflow 

    TEST_NUMBER_PARSE(1.0000000000000002, "1.0000000000000002");            // the smallest number > 1 
    TEST_NUMBER_PARSE(4.9406564584124654e-324, "4.9406564584124654e-324");  // minimum denormal 
    TEST_NUMBER_PARSE(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER_PARSE(2.2250738585072009e-308, "2.2250738585072009e-308");  // max subnormal double 
    TEST_NUMBER_PARSE(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER_PARSE(2.2250738585072014e-308, "2.2250738585072014e-308");  // min normal positive double
    TEST_NUMBER_PARSE(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER_PARSE(1.7976931348623157e+308, "1.7976931348623157e+308");  // max double
    TEST_NUMBER_PARSE(-1.7976931348623157e+308, "-1.7976931348623157e+308");

    TEST_STRINGIFY("0");
    TEST_STRINGIFY("-0");
    TEST_STRINGIFY("1");
    TEST_STRINGIFY("-1");
    TEST_STRINGIFY("1.5");
    TEST_STRINGIFY("-1.5");
    TEST_STRINGIFY("3.25");
    TEST_STRINGIFY("1e+20");
    TEST_STRINGIFY("1.234e+20");
    TEST_STRINGIFY("1.234e-20");

    TEST_STRINGIFY("1.0000000000000002");       // the smallest number > 1
    TEST_STRINGIFY("4.9406564584124654e-324");  // minimum denormal
    TEST_STRINGIFY("-4.9406564584124654e-324");
    TEST_STRINGIFY("2.2250738585072009e-308");  // max subnormal double 
    TEST_STRINGIFY("-2.2250738585072009e-308");
    TEST_STRINGIFY("2.2250738585072014e-308");  // min normal positive double
    TEST_STRINGIFY("-2.2250738585072014e-308");
    TEST_STRINGIFY("1.7976931348623157e+308");  // max double
    TEST_STRINGIFY("-1.7976931348623157e+308");
}

#define TEST_STRING_PARSE(str, jstr)                    \
    do                                                  \
    {                                                   \
        json j = json::parse(jstr);                     \
        UNIT_TEST(true, j.is_string());                 \
        UNIT_TEST(str, j.get_value<json::string_t>());  \
    } while (0)

void test_string()
{
    TEST_STRING_PARSE("", "\"\"");
    TEST_STRING_PARSE("Hello", "\"Hello\"");
    TEST_STRING_PARSE("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING_PARSE("\"\\/\b\f\n\r\t", "\"\\\"\\\\/\\b\\f\\n\\r\\t\"");
    TEST_STRING_PARSE("\x24", "\"\\u0024\"");                       // Dollar sign U+0024 
    TEST_STRING_PARSE("\xC2\xA2", "\"\\u00A2\"");                   // Cents sign U+00A2 
    TEST_STRING_PARSE("\xE2\x82\xAC", "\"\\u20AC\"");               // Euro sign U+20AC 
    TEST_STRING_PARSE("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");    // G clef sign U+1D11E 
    TEST_STRING_PARSE("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");    // G clef sign U+1D11E 
}

void test_array()
{
    json j1 = "[ ]"_json;
    UNIT_TEST(true, j1.is_array());
    UNIT_TEST(true, j1.get_value<json::array_t>().empty());

    json j2 = "[null, true, 42, \"abc\", []]"_json;
    UNIT_TEST(true, j2.is_array());
    UNIT_TEST(5, j2.get_value<json::array_t>().size());
    // cout << j2 << endl;
    UNIT_TEST(true, j2[0].is_null());
    UNIT_TEST(true, j2[1].is_boolean());
    UNIT_TEST(true, j2[1].get_value<json::boolean_t>());
    UNIT_TEST(42.0, j2[2].get_value<json::number_t>());
    UNIT_TEST("abc", j2[3].get_value<json::string_t>());
    UNIT_TEST(true, j2[4].is_array());
    
    json j3 = "[[], [0], [0,1], [0, 1,  2]]"_json;
    UNIT_TEST(true, j3.is_array());
    UNIT_TEST(4, j3.get_value<json::array_t>().size());
    UNIT_TEST(true, j3[0].is_array());
    UNIT_TEST(true, j3[0].get_value<json::array_t>().empty());
    UNIT_TEST(0, j3[0].get_value<json::array_t>().size());
    UNIT_TEST(1, j3[1].get_value<json::array_t>().size());
    UNIT_TEST(2, j3[2].get_value<json::array_t>().size());
    UNIT_TEST(3, j3[3].get_value<json::array_t>().size());

    json j4 = json::array_t{ nullptr, true, 42, "abc", json::array_t{} };
    UNIT_TEST(true, j4.is_array());
    UNIT_TEST(5, j4.size());
    UNIT_TEST(true, j4[0].is_null());
    UNIT_TEST(true, j4[1].is_boolean());
    UNIT_TEST(true, j4[1].get_value<json::boolean_t>());
    UNIT_TEST(42.0, j4[2].get_value<json::number_t>());
    UNIT_TEST("abc", j4[3].get_value<json::string_t>());
    UNIT_TEST(true, j4[4].is_array());
}

void test_object()
{
    json j1 = " { "
        "\"n\" : null , "
        "\"f\" : false , "
        "\"t\" : true , "
        "\"i\" : 123 , "
        "\"s\" : \"abc\", "
        "\"a\" : [1, 2, 3],"
        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
        " } "_json;

    UNIT_TEST(true, j1.is_object());
    UNIT_TEST(true, j1["n"].is_null());
    UNIT_TEST(false, j1["f"].get_value<json::boolean_t>());
    UNIT_TEST(true, j1["t"].get_value<json::boolean_t>());
    UNIT_TEST(123.0, j1["i"].get_value<json::number_t>());
    UNIT_TEST("abc", j1["s"].get_value<json::string_t>());
    UNIT_TEST(true, j1["a"].is_array());
    UNIT_TEST(3, j1["a"].size());
    UNIT_TEST(1.0, j1["a"][0].get_value<json::number_t>());
    UNIT_TEST(true, j1["o"].is_object());
    UNIT_TEST(1.0, j1["o"]["1"].get_value<json::number_t>());
    UNIT_TEST(3, j1["o"].size());
}

void test_error()
{
    UNIT_TEST(true, json::parse("nullptr").is_error());
    UNIT_TEST(true, json::parse("truex").is_error());
    UNIT_TEST(true, json::parse("+0").is_error());
    UNIT_TEST(true, json::parse("+1").is_error());
    UNIT_TEST(true, json::parse("0123").is_error());
    UNIT_TEST(true, json::parse("0x0").is_error());
    UNIT_TEST(true, json::parse("0x123").is_error());
    UNIT_TEST(true, json::parse("1e309").is_error());   // too big
    UNIT_TEST(true, json::parse("-1e309").is_error());
    UNIT_TEST(true, json::parse(".123").is_error());
    UNIT_TEST(true, json::parse("1.").is_error());
    UNIT_TEST(true, json::parse("INF").is_error());
    UNIT_TEST(true, json::parse("NaN").is_error());
    UNIT_TEST(true, json::parse("\"").is_error());
    UNIT_TEST(true, json::parse("\"abc").is_error());
    UNIT_TEST(true, json::parse("\"\\v\"").is_error());
    UNIT_TEST(true, json::parse("\"\\'\"").is_error());
    UNIT_TEST(true, json::parse("\"\\0\"").is_error());
    UNIT_TEST(true, json::parse("\"\\x12\"").is_error());
    UNIT_TEST(true, json::parse("\"\x01\"").is_error());    
    UNIT_TEST(true, json::parse("\"\x1F\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u0\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u01\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u012\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u/000\"").is_error());
    UNIT_TEST(true, json::parse("\"\\uG000\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u0/00\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u0G00\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u0/00\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u00G0\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u000/\"").is_error());
    UNIT_TEST(true, json::parse("\"\\u000G\"").is_error());
    UNIT_TEST(true, json::parse("\"\\uD800\"").is_error());
    UNIT_TEST(true, json::parse("\"\\uDBFF\"").is_error());
    UNIT_TEST(true, json::parse("\"\\uD800\\\\\"").is_error());
    UNIT_TEST(true, json::parse("\"\\uD800\\uDBFF\"").is_error());
    UNIT_TEST(true, json::parse("\"\\uD800\\uE000\"").is_error());
    UNIT_TEST(true, json::parse("[1,").is_error());
    UNIT_TEST(true, json::parse("[\"a\", nul]").is_error());
    UNIT_TEST(true, json::parse("[1").is_error());
    UNIT_TEST(true, json::parse("[1}").is_error());
    UNIT_TEST(true, json::parse("[1 2").is_error());
    UNIT_TEST(true, json::parse("[[]").is_error());
    UNIT_TEST(true, json::parse("{:1,").is_error());
    UNIT_TEST(true, json::parse("{1:1,").is_error());
    UNIT_TEST(true, json::parse("{false:1,").is_error());
    UNIT_TEST(true, json::parse("{null:1,").is_error());
    UNIT_TEST(true, json::parse("{[]:1,").is_error());
    UNIT_TEST(true, json::parse("{{}:1,").is_error());
    UNIT_TEST(true, json::parse("{\"a\":1,").is_error());
    UNIT_TEST(true, json::parse("{\"a\"}").is_error());
    UNIT_TEST(true, json::parse("{\"a\",\"b\"}").is_error());
    UNIT_TEST(true, json::parse("{\"a\":1").is_error());
    UNIT_TEST(true, json::parse("{\"a\":1]").is_error());
    UNIT_TEST(true, json::parse("{\"a\":1 \"b\"").is_error());
    UNIT_TEST(true, json::parse("{\"a\":{}").is_error());
}

void test_all()
{
    test_null();
    test_boolean();
    test_number();
    test_string();
    test_array();
    test_object();
    test_error();
}

int main()
{
#ifdef _WIN32
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    test_all();
    std::cout << test_pass << "/" << test_count
        << " (passed " << test_pass * 100.0 / test_count << "%)" << std::endl;
    
    return 0;
}