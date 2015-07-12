#include "collada.h"

#include "portable.h"

int main(int argc, char* argv[])
{
	//std::wstring uri = L"../data/malers.dae";
	//std::wstring uri = L"../samples/negimiku/negimiku.dae";
	std::wstring uri = L"../samples/astro/astro.dae";
	t_document document;
	{
		std::unique_ptr<xmlParserInputBuffer, void (*)(xmlParserInputBufferPtr)> input(xmlParserInputBufferCreateFilename(f_convert(uri).c_str(), XML_CHAR_ENCODING_NONE), xmlFreeParserInputBuffer);
		t_reader reader(input.get());
		document.f_load(reader, uri.substr(0, uri.find_last_of(L'/') + 1));
	}
	document.f_dump(std::wcout);
	return 0;
}
