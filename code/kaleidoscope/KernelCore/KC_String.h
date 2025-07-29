//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 17:06:55
//

#pragma once

#include <iostream>
#include <string>

#include "KC_Common.h"
#include "KC_Array.h"

namespace KC
{
    using String = std::string;

    namespace StringUtil
    {
        String Format(const char* fmt, ...);
        Array<String> ToLines(const String& source);
    
        void TrimLeft(String& s);
        void TrimRight(String& s);
        void Trim(String& s);
        bool StartsWith(const String& s, const String& prefix);
    }

    template<typename CharSource, typename CharDest>
    inline uint64 StringConvert(const CharSource* source, CharDest* dest, int size);
    
    template<>
    inline uint64 StringConvert(const wchar_t* source, char* destination, int size)
    {
        uint64 converted = 0;
        wcstombs_s(&converted, destination, size, source, size);
        return converted;
    }
    
    template<>
    inline uint64 StringConvert(const char* source, wchar_t* destination, int size)
    {
        uint64 converted = 0;
    	mbstowcs_s(&converted, destination, size, source, size);
    	return converted;
    }
    
    template<typename CharSource, typename CharDest>
    struct StringConverter
    {
        StringConverter(const CharSource* str)
            : mString({})
        {
            StringConvert<CharSource, CharDest>(str, mString, 128);
        }
    
        CharDest* Get() { return mString; }
    
        const CharDest* operator*() const { return mString; }
    private:
        CharDest mString[128];
    };
    
    using UnicodeToMultibyte = StringConverter<wchar_t, char>;
    using MultibyteToUnicode = StringConverter<char, wchar_t>;
}

#define UNICODE_TO_MULTIBYTE(input) KC::UnicodeToMultibyte(input).Get()
#define MULTIBYTE_TO_UNICODE(input) KC::MultibyteToUnicode(input).Get()
