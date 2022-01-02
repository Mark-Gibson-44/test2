#pragma once
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"




class Opt{
	std::unique_ptr<llvm::Module> M;
	bool insert_point;
public:
	Opt(std::unique_ptr<llvm::Module> M) : M(std::move(M)), insert_point(true){}
	
	void IdentifyOptSpace();
	void optimiseMesh(llvm::Instruction* start, llvm::Instruction* end, int mesh_height, int vector_width);
	void dump_Instructions(llvm::Instruction* start, llvm::Instruction* end);
	//void mutateUnaryExpr();
	//void mutateParameterOrder();
	//void mutateInstructionOrder();
	
};	
