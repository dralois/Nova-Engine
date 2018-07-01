#pragma once

#include <string>
#include <DXUT.h>

namespace Util 
{
	// Converts string to wchar_t pointer (use with caution, don't forget to delete)
	static wchar_t * strToWChar_t(std::string input) {
		wchar_t * output = new wchar_t[input.length() + 1];
		size_t out;
		mbstowcs_s(&out, output, input.length() + 1, input.c_str(), input.length());
		return output;
	}
}