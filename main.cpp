#include <iostream>

#include "opt.h"
#include "parse.cpp"

int main(int argv, char** argc){
	if(argv == 1){
		std::cout << "ERROR: Expected Bitcode Filename Argument\n";
		return 1;
	}
	LLVMContext theContext;
	SMDiagnostic err;
	auto parsed = ParseFile(argc[1], theContext, err);
	if(parsed == nullptr){
		std::cout << "ERROR: Invalid FileName\n";
		return 1;
	}

	
	auto opt = Opt(std::move(parsed));
	
	opt.IdentifyOptSpace();

	return 0;
}

