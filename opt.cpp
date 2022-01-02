#include "opt.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include <llvm-c/BitReader.h>
#include <llvm-c/BitWriter.h>

#include <iostream>
#include <string>


using namespace llvm;

Value* vectorizeValue(Value* val, int VECTOR_SIZE, PHINode* indVar) {
      
	if (auto* constant = dyn_cast<ConstantData>(val)) {
			
        	return ConstantDataVector::getSplat(VECTOR_SIZE, constant);

      }
	  if (auto* inst = dyn_cast<Instruction>(val)) {
		IRBuilder<> builder(inst);
        	Value* initVec;
			raw_ostream &output2 = outs();
        	if (auto* intType = dyn_cast<IntegerType>(inst->getType())) {
				
          	initVec =
              	ConstantDataVector::getSplat(VECTOR_SIZE,
                  	ConstantInt::get(intType, 0));

        	} else {
				if (LoadInst *l = dyn_cast<LoadInst>(val)){
					auto q = l->getPointerOperandType();
					//std::cout << "\n";
					//p->print(output2);
					//std::cout << " " <<  p->isPointerTy();
					//std::cout << "<------||--\n";
					if(PointerType *p = dyn_cast<PointerType>(q)){
						//std::cout << "True\n\n";
						//q->print(output2);
						//std::cout << "<------||--\n";
						auto nested = p->getElementType();
						//nested->print(output2);


						//std::cout << "<==========\n";
						if(PointerType *p2 = dyn_cast<PointerType>(nested)){
							//std::cout << "NESTED\n";
							return val;
						}
					}

				}
				
          	initVec =
              	ConstantDataVector::getSplat(VECTOR_SIZE,
                  	ConstantFP::get(val->getType(), 0.0));
				
        	}
          
        	builder.SetInsertPoint(inst->getNextNode());

        	Value* curVec = initVec;
        	for (int i = 0; i < VECTOR_SIZE; i++) {
          		curVec = builder.CreateInsertElement(curVec, inst, builder.getInt64(i));
        	}

        	// vector of inductive variables has to have its stride
        	if (val == indVar) {
				
          		std::vector<uint64_t> strides;
          		for (uint64_t i = 0; i < VECTOR_SIZE; i++) {
            			strides.push_back(i);
          		}

          		ArrayRef<uint64_t> strideRef(strides);
          		Value* strideVec = ConstantDataVector::get(indVar->getContext(), strideRef);
          		Value* resultVec = builder.CreateAdd(curVec, strideVec);
          		return resultVec;

        	} else {
				
          		return curVec;
		}
	}
	
	return NULL;
}


void Opt::IdentifyOptSpace(){
	auto funcIter = M->begin();
	std::vector<Instruction*> opt_start;
	std::vector<Instruction*> opt_end;
	

	//Iterate over Instructions and record BinOps for manipulation
	while(funcIter != M->end()){
		for(auto& BB: *funcIter){
			for(auto& stmt : BB){
				
				if(stmt.isBitwiseLogicOp()){
					if(insert_point){
						//std::cout << "Can be Optimised" << std::endl;
						opt_start.push_back(&stmt);					
						insert_point = !insert_point;
					}else{
						//std::cout << "End of optimisable section" << std::endl;
						insert_point = !insert_point;
						opt_end.push_back(&stmt);
					}
					
				}
			}
		}
		funcIter++;
	}
	
	//For all insert points optimise corresponding to a certain version
	for(int i = 0; i < opt_start.size(); i++){
		dump_Instructions(opt_start[i], opt_end[i]);
	}
	raw_ostream &output2 = outs();
	M->print(output2, nullptr);
}


void Opt::dump_Instructions(Instruction* start, Instruction* end){
	int original = M->getInstructionCount();
	//std::cout << "Start instruction" << std::endl;
	int i = 0;
	raw_ostream &output = outs();	
	auto iterator = start;
	while(iterator != end){
		if(StoreInst *s = dyn_cast<StoreInst>(&*iterator)){

		} else if (LoadInst *l = dyn_cast<LoadInst>(&*iterator)){

		}else{
		i++;
		//std::cout << iterator->getOpcodeName() << " With " << iterator->getNumOperands() << std::endl;
		for(int opNum = 0; opNum < iterator->getNumOperands(); opNum++){
			auto operand = iterator->getOperand(opNum);
			//operand->print(output);
			//std::cout << std::endl;
			PHINode* indVar2;// = L->getCanonicalInductionVariable();
			auto test2 = vectorizeValue(operand,4, indVar2);
			//test2->print(output);
			//std::cout << opNum << "\n";
		}
	
		Value* v = ConstantFP::get(M->getContext(), APFloat(0.0));
		
		//auto builder = std::make_unique<IRBuilder<>>(iterator);
		//builder->CreateBitCast(v, Type::getDoubleTy(M->getContext()));
		//PHINode* indVar;// = L->getCanonicalInductionVariable();
		//auto copy = iterator;
		//auto test = vectorizeValue(copy ,4, indVar);
		//test->print(output);
		//std::cout << std::endl;
		}
		if(StoreInst *si = dyn_cast<StoreInst>(&*iterator)){
			//std::cout << "Store" << std::endl;
		}
		if(LoadInst *li = dyn_cast<LoadInst>(&*iterator)){
			//std::cout << "Load" << std::endl;
		}
		iterator = iterator->getNextNonDebugInstruction();
	}

	//std::cout << "Finished with " << i << " Instructions" << std::endl;
	//std::cout << "Increase " << M->getInstructionCount() - original << std::endl;
}



void optimiseMesh(Instruction* start, Instruction* end, int mesh_height, int vector_width){
	//Builder object to be used for insertion of packed instructions & removal of scalar
	auto builder = std::make_unique<IRBuilder<>>(start);
	auto inst_iter = start;
	//Variable to represent number of doubles that can be vectorised in a single register
	int packed_size = vector_width / sizeof(double);
	//Variable to represent number of iterations required for full vertical vectorisation
	//EG Height = 256, packed_size = 8 elements, 
	int step_size = mesh_height / packed_size;
	
	std::vector<std::vector<Instruction*>> packing;
	
	while(inst_iter != end){
		std::vector<Instruction*> Index1s;
		std::vector<Instruction*> Index2s;
		std::vector<Instruction*> pack;
		auto current = inst_iter;
		//Basic Algorithm:
		//Perform Vector_width / size_of(data) loads for each index calculation
		//Peform number of indexs * number of values packed into a register data loads
		//Pack each corresponding array into a vector reg
		//Perform vector multiplication
		//Sum each value into accumulator
		
		//get instructions to pack into vector_inst
		/*for(int i = 0; i < packed_size; i++){
			pack.push_back(current);
			//For loop used to iterate along instruction to reach face above prior
			for(int j = 0; j < step_size; j++){
				current = current->getNextNonDebugInstruction();
			}
		}
		*/
		
		// Do integer loading and calculation
		
	
		// Do Double loading
		
		//Extract elements from double & packing
		
		//Value* vec = ConstantDataVector::getSplat(vector_size,
                  //ConstantDouble::get(doubleType, 0));
		for(int i = 0; i < vector_width; i++){
			//vector = builder.CreateInsertElement(curVec, inst, builder.getDouble(i));
		}
		
		//Load Accumulation value

	       	//Add each vector element to accumulator	
		
		//Bitcast Vector into Array of Doubles
		//%val = bitcast <4 x double> <double 1, double 2, double 3, double 5> to double*
		//builder.CreateBitCast(vec, 4 * ConstantDouble::get(doubleType, 0));
		for(int i = 0; i < vector_width; i++){
			//fadd accumulator vector[i]
		}
		//packing.push_back(pack)
		//go to next instruction
		//
		
		inst_iter = inst_iter->getNextNonDebugInstruction();
	}
	for(auto& vectorInst: packing){
		//identify insertpoint;
	

	}

}
