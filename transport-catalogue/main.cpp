#include "request_handler.h"

#include <iostream>
//#include <Windows.h>

using namespace transport_catalogue;

int main() {
	//SetConsoleCP(1251);
	//SetConsoleOutputCP(1251);
	TransportCatalogue catalogue;
	RequestHandler handler(catalogue);
	handler.ProcessRequest(std::cin, std::cout);
}