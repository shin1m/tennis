#include "collada.h"

int main(int argc, char* argv[])
{
	t_document document;
	//document.f_load(L"../data/malers.dae");
	//document.f_load(L"../samples/negimiku/negimiku.dae");
	document.f_load(L"../samples/astro/astro.dae");
	document.f_dump(std::wcout);
	return 0;
}
