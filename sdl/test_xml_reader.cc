#include <iostream>

#include "xml_reader.h"

int main(int argc, char* argv[])
{
	const char* uri = "../data/malers.player";
	std::unique_ptr<xmlParserInputBuffer, void (*)(xmlParserInputBufferPtr)> input(xmlParserInputBufferCreateFilename(uri, XML_CHAR_ENCODING_NONE), xmlFreeParserInputBuffer);
	t_reader reader(input.get());
	while (reader.f_read()) {
		std::wcout << L"type: " << reader.f_node_type() << L", value: " << reader.f_value() << std::endl;
	}
	return 0;
}
