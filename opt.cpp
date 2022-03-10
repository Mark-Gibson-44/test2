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
#include "llvm/Frontend/OpenMP/OMPIRBuilder.h"
#include "llvm/IR/Constants.h"


#include <iostream>
#include <string>


using namespace llvm;

std::vector<Instruction*> rem;


Value* vectorizeValue(Value* val, int VECTOR_SIZE, PHINode* indVar) {
      
	if (auto* constant = dyn_cast<ConstantData>(val)) {
			//If a constant value vectorise directly
			return val;
        	return ConstantDataVector::getSplat(VECTOR_SIZE, constant);

      }
	  if (auto* inst = dyn_cast<Instruction>(val)) {
		IRBuilder<> builder(inst);
        	Value* initVec;
			raw_ostream &output2 = outs();
        	if (auto* intType = dyn_cast<IntegerType>(inst->getType())) {
				return val;
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
						while(PointerType *p2 = dyn_cast<PointerType>(nested)){
							//std::cout << "NESTED\n";
							
							nested = p2->getElementType();
							p = p2;
						}
						
						//nested->print(output2);
						//FUCK ABOUT WITH nested type to increase load size;
						//auto x = ConstantDataVector::getSplat(VECTOR_SIZE, ConstantFP::get(nested, 0.0));
						return val;
					}

				}
				if (val->getType()->isDoubleTy()) {
					std::cout << "DOUBLE\n";
          	initVec =
              	ConstantDataVector::getSplat(VECTOR_SIZE,
                  	ConstantInt::get(val->getType(), 0));

        	}
			
          	initVec =
              	ConstantDataVector::getSplat(VECTOR_SIZE,
                  	ConstantFP::get(val->getType(), 0.0));
				
        	}
          
        	//builder.SetInsertPoint(inst->getNextNode());

			builder.SetInsertPoint(inst);



        	Value* curVec = initVec;
        	for (int i = 0; i < VECTOR_SIZE; i++) {
          		//curVec = builder.CreateInsertElement(curVec, inst, initVec);
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

		for(auto& r: rem){
			r->eraseFromParent();
		}
	}

	raw_ostream &output2 = outs();
	M->print(output2, nullptr);
}


//vectoriseOperand(v1, 8 , Type::getDoubleTy(M->getContext));
/*
void vectoriseOperand(Value* operand, int vector_size, Type* doubleTyID){
	
	
	
	Type* vecTy = VectorType::get(DoubleTyID, vector_size);
	Value* newlt;
	builder->CreateInsertElement(vecTy, newlt, operand);

}
*/

void Opt::dump_Instructions(Instruction* start, Instruction* end){
	int original = M->getInstructionCount();
	//std::cout << "Start instruction" << std::endl;
	//std::cout << "BEFORE " << original << "\n";
	int i = 0;
	raw_ostream &output = outs();	
	auto iterator = start;
	while(iterator != end){
		IRBuilder<> builder(iterator);
		if(StoreInst *s = dyn_cast<StoreInst>(&*iterator)){

		} else if (LoadInst *l = dyn_cast<LoadInst>(&*iterator)){

		}else{
		i++;
		Value* t;
		//std::cout << iterator->getOpcodeName() << " With " << iterator->getNumOperands() << std::endl;
		for(int opNum = 0; opNum < iterator->getNumOperands(); opNum++){
			auto operand = iterator->getOperand(opNum);
			//operand->print(output);
			//std::cout << std::endl;
			PHINode* indVar2;// = L->getCanonicalInductionVariable();
			auto test2 = vectorizeValue(operand,8, indVar2);
			t = test2;
			//test2->print(output);
			//std::cout << opNum << "\n";
		}
		//iterator->print(output);
		//std::cout << "\n";
		auto opcode__ = iterator->getOpcode();

		//std::cout << "OPCODE " << opcode__ << "\n";
		

		switch(opcode__){
			case Instruction::Load :{
					Type* vecTy = VectorType::get(Type::getDoubleTy(M->getContext()), 8);
					Value* help = UndefValue::get(vecTy);
					Instruction* fullVector;
					for(int  i = 0; i < 8; i++){
						Constant* index = Constant::getIntegerValue(Type::getDoubleTy(M->getContext()), APInt(32, i));
						 fullVector = InsertElementInst::Create(help, iterator->getOperand(0), index);

					}
					auto insertion = builder.Insert(fullVector);
					builder.CreateLoad(help, fullVector, "LOAD");
			}
			break;

			case Instruction::FMul: {
				Type* vecTy = VectorType::get(Type::getDoubleTy(M->getContext()), 8);
				Value* help = UndefValue::get(vecTy);
				
				Constant* index = Constant::getIntegerValue(Type::getDoubleTy(M->getContext()), APInt(32, 0));
				
				Instruction* fullVector = InsertElementInst::Create(help, iterator->getOperand(0), index);
				auto insertion = builder.Insert(fullVector);
				//builder.CreateFMul(help, insertion, "inserter");
				auto a = builder.CreateFMul(insertion, insertion, "inserter");
				//builder.CreateGEP(Type::getDoubleTy(M->getContext()), fullVector, index, "gep");
				//builder.CreateGEP(vecTy, a, 0, "gep");
				//builder.CreateInsertElement(newlt, iterator->getOperand(1), uit);
				//builder.CreateFMul(iterator->getOperand(0), iterator->getOperand(1), "insert");
				//Instruction* extract = ExtractElementInst::Create(iterator->getOperand(0), help, index);
				
				Value* v1 = builder.CreateExtractElement(help, builder.getInt32(0));
				//builder.Insert(v1);
				builder.CreateFAdd(iterator->getOperand(0), v1, "VEC ADD");
				builder.CreateStore(help, v1, "Sore");
				//builder.CreateExtractElement(help, index, "Extract");
			}
				break;
			case Instruction::FAdd:
				builder.CreateFAdd(iterator->getOperand(0), iterator->getOperand(1), "insert");

			
			 
		}


		//rem.push_back(iterator);
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

	std::cout << "After " << M->getInstructionCount() << "\n";
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

