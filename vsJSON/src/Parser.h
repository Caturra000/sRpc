#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__
#include <bits/stdc++.h>
#include "Json.h"

namespace ParserImpl {
    Json parseImpl(const char *&p);
    StringImpl parseString(const char *&p);
    Json parseObject(const char *&p);
    Json parseArray(const char *&p);
    IntegerImpl parseInteger(const char *&p);
    DecimalImpl parseDeciaml(const char *&p);
}

inline Json parse(const char *p) {
    return ParserImpl::parseImpl(p);
}

inline Json parse(const std::string &str) {
    return parse(str.data());
}

namespace ParserImpl {

inline bool isWhitespace(char ch) {
    return ch == ' ' || ch == '\n' || ch == '\t';
}
inline const char* skipWhiteSpace(const char *p) {
    while(p && (isWhitespace(*p))) ++p;
    return p;
}

inline Json parseImpl(const char *&p) {
    p = skipWhiteSpace(p);
    switch (*p) {
        case '{':
            return parseObject(p);
        case '[':
            return parseArray(p);
        case '\"':
            return parseString(p);
        case 'n':
            p += 4;
            return nullptr;
        case 't':
            p += 4;
            return true;
        case 'f':
            p += 5;
            return false;
        case '-':
            ++p;
            return -parseInteger(p);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': // 没有decimal
            return parseInteger(p);
        
        default:
            // impossible
            return nullptr;
    }
    return nullptr;
}

inline Json parseObject(const char *&p) {
    ++p; // {
    p = skipWhiteSpace(p);
    Json object(ObjectImpl{});
    if(*p == '}') {
        ++p;
        return object;
    }
    for(;;) {
        p = skipWhiteSpace(p);
        StringImpl key = parseString(p);
        p = skipWhiteSpace(p);
        if(*p != ':') ; // throw
        ++p; // :
        p = skipWhiteSpace(p);
        object[key] = parseImpl(p);
        p = skipWhiteSpace(p);
        if(*p == '}') {
            ++p;
            break;
        } else if(*p == ',') {
            ++p;
        } else {
            // throw
        }
    }
    return object;
}

inline Json parseArray(const char *&p) {
    ++p;
    p = skipWhiteSpace(p);
    Json array = Json::array();
    if(*p == ']') {
        ++p;
        return array;
    }
    for(;;) {
        p = skipWhiteSpace(p);
        auto r = parseImpl(p);
        array.append(r);
        p = skipWhiteSpace(p);
        if(*p == ']') {
            ++p;
            break;
        } else if(*p == ',') {
            ++p;
        } else {

        }
    }
    return array;
}

inline StringImpl parseString(const char *&p) {
    p = skipWhiteSpace(p);
    if(*p != '\"') ; // throw
    ++p; // "
    auto start = p;
    while(p && *p != '\"') ++p;
    if(!p) ; // throw
    auto end = p; //[start, end)
    ++p; // "
    return std::string(start, end - start); 
}

inline IntegerImpl parseInteger(const char *&p) {
    IntegerImpl i = 0;
    while(p && isdigit(*p)) { // -
        i = i*10 + (*p - '0');
        ++p;
    }
    if(!p) ; // throw
    return i;
}

inline DecimalImpl parseDeciaml(const char *&p) {
    DecimalImpl d = 0;
    while(p && (isdigit(*p) || *p == '.')) { // - E e
        if(*p == '.') ;
        ++p;
    }
    if(!p) ;
    return d;
}

} // ParserImpl
#endif