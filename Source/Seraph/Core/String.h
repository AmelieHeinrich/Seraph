//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-27 06:59:46
//

#pragma once

#include "Types.h"

#include <cstdarg>

// Credit to https://github.com/simco50/D3D12_Research/blob/master/Source/Core/CString.h

template<typename CharSource, typename CharDest>
inline size_t StringConvert(const CharSource* source, CharDest* dest, int size);

template<>
inline size_t StringConvert(const wchar_t* source, char* destination, int size)
{
    size_t converted = 0;
    wcstombs_s(&converted, destination, size, source, size);
    return converted;
}

template<>
inline size_t StringConvert(const char* source, wchar_t* destination, int size)
{
    size_t converted = 0;
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
#define UNICODE_TO_MULTIBYTE(input) UnicodeToMultibyte(input).Get()
#define MULTIBYTE_TO_UNICODE(input) MultibyteToUnicode(input).Get()
