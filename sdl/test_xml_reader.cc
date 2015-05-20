#include <iostream>

#include "xml_reader.h"

int main(int argc, char* argv[])
{
	t_reader reader(L"../data/malers.player");
	while (reader.f_read()) {
		std::wcout << L"type: " << reader.f_node_type() << L", value: " << reader.f_value() << std::endl;
	}
	return 0;
}
