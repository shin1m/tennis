#ifndef PORTABLE_H
#define PORTABLE_H

#include <string>

std::string f_convert(const std::wstring& a_string);
std::wstring f_convert(const std::string& a_string);

#ifdef __unix__
const wchar_t v_directory_separator = L'/';
#endif
#ifdef _WIN32
const wchar_t v_directory_separator = L'\\';
#endif

std::wstring f_absolute_path(const std::wstring& a_path);

#endif
