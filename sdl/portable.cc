#include "portable.h"

#include <vector>
#include <cstdlib>
#ifdef __unix__
#include <unistd.h>
#endif

std::string f_convert(const std::wstring& a_string)
{
	size_t n = std::wcstombs(NULL, a_string.c_str(), 0) + 1;
	std::vector<char> cs(n);
	std::wcstombs(&cs[0], a_string.c_str(), n);
	return &cs[0];
}

std::wstring f_convert(const std::string& a_string)
{
	size_t n = std::mbstowcs(NULL, a_string.c_str(), 0) + 1;
	std::vector<wchar_t> cs(n);
	std::mbstowcs(&cs[0], a_string.c_str(), n);
	return &cs[0];
}

#ifdef __unix__
std::wstring f_absolute_path(const std::wstring& a_path)
{
	if (a_path[0] == v_directory_separator) return a_path;
	char* mbs = getcwd(NULL, 0);
	auto cwd = f_convert(mbs);
	std::free(mbs);
	return cwd + v_directory_separator + a_path;
}
#endif

#ifdef _WIN32
std::wstring f_absolute_path(const std::wstring& a_path)
{
	DWORD n = GetFullPathNameW(a_path.c_str(), 0, NULL, NULL);
	std::vector<wchar_t> cs(n);
	wchar_t* p;
	GetFullPathNameW(a_path.c_str(), n, &cs[0], &p);
	return &cs[0];
}
#endif
