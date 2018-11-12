#pragma once

#include <cassert>
#include <cerrno>
#include <map>
#include <memory>
#include <string>
#include <vector>

#if __cplusplus >= 201103L || _MSVC_LANG >= 201103L
#  define JSON_HAS_CXX11 1
#endif 

#if __cplusplus >= 201402L || _MSVC_LANG >= 201402L
#  define JSON_HAS_CXX14 1
#endif 

#if __cplusplus >= 201703L || _MSVC_LANG >= 201703L
#  define JSON_HAS_CXX17 1
#endif 

#ifdef JSON_HAS_CXX17
#include <variant>
#else
#include <boost/variant.hpp>
#endif 

namespace Json
{

namespace detail
{
#ifdef JSON_HAS_CXX17
    using std::get;
#else
    using boost::get;
#endif
}


enum class json_type : size_t
{
    null,
    boolean,
    number,
    string,
    array,
    object,
    error
};

struct error_type { };
constexpr error_type error_value{};

class json
{
public:
    using null_t    = std::nullptr_t;
    using boolean_t = bool;
    using number_t  = double;
    using string_t  = std::string;
    using array_t   = std::vector<json>;
    using object_t  = std::map<string_t, json>;
    using error_t   = error_type;

private:
#ifdef JSON_HAS_CXX17
    using json_value = std::variant<null_t, boolean_t, number_t,
        string_t, array_t, object_t, error_t>;
#else 
    using json_value = boost::variant<null_t, boolean_t, number_t,
        string_t, array_t, object_t, error_t>;
#endif
    
    json_value m_value;
public:
    json() : m_value(nullptr) { }
    json(null_t) : m_value(nullptr) { }
    json(bool b) : m_value(b) { }

    template <typename T, std::enable_if_t<
        std::is_arithmetic<T>::value, int> = 0>
    json(T val) : m_value(static_cast<double>(val)) { }

    json(std::string str) : m_value(std::move(str)) { }
    json(const char* str) : m_value(std::string(str)) { }
    json(array_t arr) : m_value(std::move(arr)) { }
    json(object_t obj) : m_value(std::move(obj)) { }
    json(error_t) : m_value(error_value) { }

    json_type get_type() const noexcept
    {
#ifdef JSON_HAS_CXX17
        return static_cast<json_type>(m_value.index());
#else
        return static_cast<json_type>(m_value.which());
#endif
    }

    const char* type_name() const noexcept
    {
        json_type type = get_type();
        switch (type)
        {
        case Json::json_type::null:
            return "null";
        case Json::json_type::boolean:
            return "boolean";
        case Json::json_type::number:
            return "number";
        case Json::json_type::string:
            return "string";
        case Json::json_type::array:
            return "array";
        case Json::json_type::object:
            return "object";
        default:
            return "error";
        }
    }

    template <typename T>
    T& get_value()
    {
        return detail::get<T>(m_value);
    }

    template <typename T>
    const T& get_value() const
    {
        return detail::get<T>(m_value);
    }

    size_t size() const
    {
        json_type type = get_type();
        switch (type)
        {
        case Json::json_type::null:
        case Json::json_type::boolean:
        case Json::json_type::number:
        case Json::json_type::string:
            return 1;
        case Json::json_type::array:
            return detail::get<array_t>(m_value).size();
        case Json::json_type::object:
            return detail::get<object_t>(m_value).size();
        default:
            return 0;
        }
    }

    json& operator[](size_t idx)
    {
        assert(is_array());
        return detail::get<array_t>(m_value)[idx];
    }

    const json& operator[](size_t idx) const
    {
        assert(is_array());
        return detail::get<array_t>(m_value)[idx];
    }

    json& operator[](const std::string& key)
    {
        assert(is_object());
        return detail::get<object_t>(m_value)[key];
    }

    const json& operator[](const std::string& key) const
    {
        assert(is_object());
        return const_cast<object_t&>(detail::get<object_t>(m_value))[key];
    }

    bool is_null()    const noexcept { return get_type() == json_type::null;    }
    bool is_boolean() const noexcept { return get_type() == json_type::boolean; }
    bool is_number()  const noexcept { return get_type() == json_type::number;  }
    bool is_string()  const noexcept { return get_type() == json_type::string;  }
    bool is_array()   const noexcept { return get_type() == json_type::array;   }
    bool is_object()  const noexcept { return get_type() == json_type::object;  }
    bool is_error()   const noexcept { return get_type() == json_type::error;   }

private:
    // dump auxiliary function
    static void dump(null_t, std::string& out) 
    {
        out += "null";
    }

    static void dump(boolean_t val, std::string& out) 
    {
        out += val ? "true" : "false";
    }

    static void dump(number_t num, std::string& out) 
    {
        out += std::to_string(num);
    }

    static void dump(const string_t& str, std::string& out)
    {
        out += '"';
        for (char ch : str)
        {
            switch (ch)
            {
            case '\\':
                out += "\\\\";
                break;
            case '/':
                out += "\\/";
                break;
            case '"':
                out += "\\\"";
                break;
            case '\b':
                out += "\\b";
                break;
            case '\f':
                out += "\\f";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\t':
                out += "\\t";
                break;
            case '\r':
                out += "\\r";
                break;
            default:
                if (ch < 0x20)
                {
                    char buff[7];
                    std::snprintf(buff, sizeof(buff), "\\u%04X", ch);
                    out += buff;
                }
                else
                {
                    out += ch;
                }
                break;
            }
        }

        out += '"';
    }

    static void dump(const array_t& arr, std::string& out)
    {
        bool first_time = true;
        out += '[';
        for (const auto& j : arr)
        {
            if (!first_time)
                out += ", ";
            j.dump(out);
            first_time = false;
        }

        out += ']';
    }

    static void dump(const object_t& obj, std::string& out) 
    {
        bool first_time = true;
        out += '{';
        for (const auto& p : obj)
        {
            if (!first_time)
                out += ", ";
            dump(p.first, out);
            out += ": ";
            p.second.dump(out);
            first_time = false;
        }

        out += '}';
    }

private:
    // parse auxiliary class
    struct parser
    {
        const std::string& str;
        size_t idx;
        bool fail;

        parser(const std::string& s) : str(s), idx(0), fail(false) {}
        parser(std::string&&) = delete;
        parser(parser&&) = delete;

        bool is_digit0()
        {
            return str[idx] >= '0' && str[idx] <= '9';
        }

        bool is_digit1()
        {
            return str[idx] >= '1' && str[idx] <= '9';
        }

        json parse_error()
        {
            fail = true;
            return json{ error_value };
        }

        void parse_whitespace()
        {
            if (str[idx] == ' ' || str[idx] == '\t' || str[idx] == '\n' || str[idx] == '\r')
                ++idx;
        }

        char get_next_char()
        {
            parse_whitespace();

            if (idx == str.size())
            {
                fail = true;
                return '\0';
            }

            return str[idx++];
        }

        json parse_null()
        {
            if (str.compare(idx, 3, "ull") == 0)
            {
                idx += 3;
                return json{ nullptr };
            }
            return parse_error();
        }

        json parse_boolean(const char* s, size_t count, bool val)
        {
            if (str.compare(idx, count, s) == 0)
            {
                idx += count;
                fail = false;
                return json{ val };
            }
            return parse_error();
        }

        json parse_number()
        {
            size_t begin = --idx;
            if (str[idx] == '-') ++idx;
            
            if (str[idx] == '0') ++idx;
            else
            {
                if (!is_digit1())
                    return parse_error();
                for (++idx; is_digit0(); ++idx);
            }
            
            if (str[idx] == '.')
            {
                ++idx;
                if (!is_digit0())
                    return parse_error();
                for (++idx; is_digit0(); ++idx);
            }

            if (str[idx] == 'e' || str[idx] == 'E')
            {
                ++idx;
                if (str[idx] == '+' || str[idx] == '-') 
                    ++idx;

                if (!is_digit0())
                    return parse_error();
                for (++idx; is_digit0(); ++idx);
            }
            
            errno = 0;
            double value = strtod(str.c_str() + begin, nullptr);
            if (errno == ERANGE && (value == HUGE_VAL || value == -HUGE_VAL))
                return parse_error();

            return json{ value };
        }

        void encode_utf8(std::string& ret, uint32_t u)
        {
            if (u < 0x80)
            {
                ret += static_cast<char>(u);
            }
            else if (u < 0x800)
            {
                ret += static_cast<char>(0xC0 | ((u >> 6)  & 0x1F));
                ret += static_cast<char>(0x80 | ( u        & 0x3F));
            }
            else if (u < 0x10000)
            {
                ret += static_cast<char>(0xE0 | ((u >> 12) & 0x0F));
                ret += static_cast<char>(0x80 | ((u >> 6)  & 0x3F));
                ret += static_cast<char>(0x80 | ( u        & 0x3F));
            }
            else
            {
                ret += static_cast<char>(0xF0 | ((u >> 18) & 0x07));
                ret += static_cast<char>(0x80 | ((u >> 12) & 0x3F));
                ret += static_cast<char>(0x80 | ((u >> 6)  & 0x3F));
                ret += static_cast<char>(0x80 | ( u        & 0x3F));
            }
        }

        bool parse_hex4(uint32_t& u)
        {
            for (int i = 0; i < 4; ++i)
            {
                char ch = str[idx++];
                u <<= 4;
                if (ch >= '0' && ch <= '9')
                    u |= (ch - '0');
                else if (ch >= 'A' && ch <= 'F')
                    u |= (ch - ('A' - 10));
                else if (ch >= 'a' && ch <= 'f')
                    u |= (ch - ('a' - 10));
                else
                    return false;
            }

            return true;
        }

        json parse_string()
        {
            string_t ret;

            while (true)
            {
                if (idx == str.size())
                    return parse_error();

                char ch = str[idx++];

                if (ch == '\"')
                {
                    break;
                }
                else if (ch == '\\')
                {
                    ch = str[idx++];
                    switch (ch)
                    {
                    case '\"': ret += '\"'; break;
                    case '\\': ret += '\\'; break;
                    case '/' : ret += '/' ; break;
                    case 'b' : ret += '\b'; break;
                    case 'f' : ret += '\f'; break;
                    case 'n' : ret += '\n'; break;
                    case 'r' : ret += '\r'; break;
                    case 't' : ret += '\t'; break;
                    case 'u' :
                    {
                        uint32_t high = 0;
                        if (!parse_hex4(high))
                            return parse_error();

                        if (high >= 0xD800 && high <= 0xDBFF)   // is high surrogate
                        {
                            if (str[idx++] != '\\')         
                                return parse_error();
                            if (str[idx++] != 'u')
                                return parse_error();

                            uint32_t low = 0;
                            if (!parse_hex4(low))
                                return parse_error();

                            if (low < 0xDC00 || low > 0xDFFF)   // is not low surrogate
                                return parse_error();

                            high = 0x10000 + (((high - 0xD800) << 10) | (low - 0xDC00));
                        }
                        encode_utf8(ret, high);
                        break;
                    }
                    default:
                        return parse_error();
                        break;
                    }
                }
                else
                {
                    if (ch < 0x20)
                        return parse_error();
                    ret += ch;
                }
            }

            return json{ ret };
        }

        json parse_array()
        {
            array_t vec;
            char ch = get_next_char();

            if (ch == ']')
                return json{ std::move(vec) };

            for (;;)
            {
                --idx;
                vec.push_back(parse_json());

                ch = get_next_char();

                if (ch == ']')
                    break;
                else if (ch != ',')
                    return parse_error();

                ch = get_next_char();
            }

            return json{ std::move(vec) };
        }

        json parse_object()
        {
            object_t obj;
            char ch = get_next_char();

            if (ch == '}')
                return json{ obj };

            for (;;)
            {
                if (ch != '"')
                    return parse_error();

                json jstr = parse_string();
                
                ch = get_next_char();
                if (ch != ':')
                    return parse_error();

                obj[jstr.get_value<string_t>()] = parse_json();

                ch = get_next_char();

                if (ch == '}')
                    break;
                else if (ch != ',')
                    return parse_error();

                ch = get_next_char();
            }

            return json{ std::move(obj) };
        }

        json parse_json()
        {
            char ch = get_next_char();
            switch (ch)
            {
            case 'n':
                return parse_null();
            case 't':
                return parse_boolean("rue", 3, true);
            case 'f':
                return parse_boolean("alse", 4, false);
            case '-': 
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return parse_number();
            case '"':
                return parse_string();
            case '[':
                return parse_array();
            case '{':
                return parse_object();
            default:
                return json{ error_value };
                break;
            }
        }
    };

public:
    std::string dump() const
    {
        std::string ret;
        dump(ret);
        return ret;
    }

    void dump(std::string& out) const
    {
        json_type type = get_type();
        switch (type)
        {
        case Json::json_type::null:
            dump(detail::get<null_t>(m_value), out);
            break;
        case Json::json_type::boolean:
            dump(detail::get<boolean_t>(m_value), out);
            break;
        case Json::json_type::number:
            dump(detail::get<number_t>(m_value), out);
            break;
        case Json::json_type::string:
            dump(detail::get<string_t>(m_value), out);
            break;
        case Json::json_type::array:
            dump(detail::get<array_t>(m_value), out);
            break;
        case Json::json_type::object:
            dump(detail::get<object_t>(m_value), out);
            break;
        default:
            out = "parse error";
            break;
        }
    }

    void swap(json& rhs) noexcept
    {
        using std::swap;
        swap(m_value, rhs.m_value);
    }

    static json parse(const std::string& str)
    {
        parser p{ str };
        json ret = p.parse_json();

        p.parse_whitespace();
        if (p.fail || p.idx != str.size())
            return json{ error_value };

        return ret;
    }

    friend std::ostream& operator<<(std::ostream& os, const json& j)
    {
        os << j.dump();
        return os;
    }

    friend std::istream& operator>>(std::istream& is, json& j)
    {
        std::string str;
        is >> str;
        j = parse(str);
        return is;
    }
};

inline void swap(json& lhs, json& rhs) noexcept
{
    lhs.swap(rhs);
}

#ifdef JSON_HAS_CXX11
json operator ""_json(const char* str, size_t n)
{
    return json::parse(std::string(str, n));
}
#endif 

}   // namespace Json