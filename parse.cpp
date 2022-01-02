#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/SourceMgr.h"

using namespace llvm;

/*
 * Function concerned with parsing bitcode file
 * sets up context and error with the corresponding file
 */

std::unique_ptr<Module> ParseFile(const char* fName, LLVMContext& theContext, SMDiagnostic& err){
	
	return std::move(parseIRFile(fName, err, theContext));
}
