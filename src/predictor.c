//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Chenyu Wu";
const char *studentID   = "A59011003";
const char *email       = "chw080@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

//define number of bits required for indexing the BHT here. 
int ghistoryBits; // Number of bits used for Global History
int lhistoryBits = 10; // Number of bits used for Local History
int pcIndexBits = 10;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;


//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
//gshare
int *gpredictors;
int gHistoryTable;
//Tournament
int *ChoicePredictor; //Choice Predictor for Tournament
int *LocalPredictor;  //Local Predictor for Tournament
int *GlobalPredictor; //Global Predictor for Tournament
int *PatternHistory; // Pattern Hsitory Table for LocalPredictor
int GHR;              //Global Histroy register
//Perceptron
int Perceptron = 7;         
int history_Bits = 31;     //Number of bits in History Table
int* x;                   //The past history of each Branch
int** Perceptrons_Table; //The Perceptron table weight for each Branch

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

//gshare functions
void init_gshare() {
  int historyBits = 1 << ghistoryBits;
  gpredictors = (int*) malloc(historyBits * sizeof(int));
  for(int i = 0; i <= historyBits; i++) {
    gpredictors[i] = WN;
  }
  gHistoryTable = 0;
}



uint8_t 
gshare_predict(uint32_t pc) {
  int historyBits = 1 << ghistoryBits;
  int pc_lower_bits = pc & (historyBits - 1);
  int ghistory_lower = gHistoryTable & (historyBits - 1);
  int historyIndex = pc_lower_bits ^ (ghistory_lower);

  switch(gpredictors[historyIndex]) {
    case SN:
      return NOTTAKEN;
    case WN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Undefined state in predictor table");
      return NOTTAKEN;
  }
}

void
train_gshare(uint32_t pc, uint8_t outcome) {
  uint32_t historyBits = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (historyBits - 1);
  uint32_t ghistory_lower = gHistoryTable & (historyBits - 1);
  uint32_t historyIndex = pc_lower_bits ^ (ghistory_lower);

  switch(gpredictors[historyIndex]) {
    case SN:
      gpredictors[historyIndex] = (outcome == TAKEN) ? WN : SN;
      break;
    case WN:
      gpredictors[historyIndex] = (outcome == TAKEN) ? WT : SN;
      break;
    case WT:
      gpredictors[historyIndex] = (outcome == TAKEN) ? ST : WN;
      break;
    case ST:
      gpredictors[historyIndex] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      break;
  }
  gHistoryTable = (1 << gHistoryTable) | outcome;
}

void
cleanup_gshare() {
  free(gpredictors);
}

//Tournament Functions
void init_Tournament(){
	int historyBits = 1 << lhistoryBits;  //number of local history total index historyBits = 1024
    int GhistoryBits = 1 << ghistoryBits; //number of global history table and choice history table GhistoryBits = 4096
	int PHT = 1 << pcIndexBits;           //number of Pattern history table total index PHT = 1024
	LocalPredictor = (int*) malloc(historyBits * sizeof(int)); // Size 1024*2 = 2048
	ChoicePredictor = (int*) malloc(GhistoryBits * sizeof(int)); // Size 4096*2 = 8192
	GlobalPredictor = (int*) malloc(GhistoryBits * sizeof(int)); // Size 4096*2 = 8192
	PatternHistory = (int*) malloc(PHT * sizeof(int));   // Size 1024*10=10240  Total Size = 2048+8192+8192+10240=28672bits < 32K+320 bits
	
	for(int i = 0; i < historyBits; i++){
		LocalPredictor[i] = WN;
	}
	
	for(int i = 0; i < PHT; i++){
		PatternHistory[i] = 0;
	}
	
	for(int i = 0; i < GhistoryBits; i++){
		ChoicePredictor[i] = WN;
		GlobalPredictor[i] = WN;
	}
	
	GHR = 0;   	
}

uint8_t
Tournament_predict(uint32_t pc){
        int i,j;
	
	int historyBits = 1 << lhistoryBits; //number of local history total index
        int GhistoryBits = 1 << ghistoryBits; //number of global history table and choice history table
	int pc_lower_bits = pc & (historyBits - 1); //Lower 10 bits of PC
	int GHR_Lower_bits = GHR & (GhistoryBits - 1); //Lower 12 bits of GHR
	int flag = PatternHistory[pc_lower_bits] & (historyBits-1);
	
	switch(LocalPredictor[flag]){
		case SN:
		  i = 0; break;
		case WN:
		  i = 0; break;
		case WT:
		  i = 1; break;
		case ST:
		  i = 1; break;
		default:
		  i = 0;
          printf("Undefined state in predictor table"); break;
		
	}
	
	switch(GlobalPredictor[GHR_Lower_bits]){
		case SN:
		  j = 0; break;
		case WN:
		  j = 0; break;
		case WT:
		  j = 1; break;
		case ST:
		  j = 1; break;
		default:
		  j = 0;
		  printf("Undefined state in predictor table"); break;
	}
                if(ChoicePredictor[GHR_Lower_bits] < WT && i == 0) return NOTTAKEN;
		else if(ChoicePredictor[GHR_Lower_bits] <WT && i == 1) return TAKEN;
		else if(ChoicePredictor[GHR_Lower_bits] >= WT && j == 0) return NOTTAKEN;
		else return TAKEN;
	

}

void 
train_Tournament(uint32_t pc, uint8_t outcome){
	int flag1,flag2;
	uint32_t historyBits = 1 << lhistoryBits; //number of local history total index
        uint32_t GhistoryBits = 1 << ghistoryBits; //number of global history table and choice history table
	uint32_t pc_lower_bits = pc & (historyBits - 1); //Lower 10 bits of PC
	uint32_t GHR_Lower_bits = GHR & (GhistoryBits - 1); //Lower 12 bits of GHR
	uint32_t flag = PatternHistory[pc_lower_bits] & (historyBits-1);
	
	if(LocalPredictor[flag] < WT) flag1 = 0;
	else flag1 = 1;
	if(GlobalPredictor[GHR_Lower_bits] < WT) flag2 = 0;
	else flag2 = 1;
		
		if(flag1 != flag2){
		  if(ChoicePredictor[GHR_Lower_bits] == SN){		
		  if(outcome == NOTTAKEN && (LocalPredictor[flag] <WT)) ChoicePredictor[GHR_Lower_bits] = SN;
		  else if(outcome == TAKEN && (LocalPredictor[flag] >= WT)) ChoicePredictor[GHR_Lower_bits] = SN;
		  else ChoicePredictor[GHR_Lower_bits] = WN;
		  }
		  
		else if(ChoicePredictor[GHR_Lower_bits] == WN){
		  if(outcome == NOTTAKEN && (LocalPredictor[flag] < WT)) ChoicePredictor[GHR_Lower_bits] = SN;
		  else if(outcome == TAKEN && (LocalPredictor[flag] >= WT)) ChoicePredictor[GHR_Lower_bits] = SN;
		  else ChoicePredictor[GHR_Lower_bits] = WT; 
		}
		
		else if(ChoicePredictor[GHR_Lower_bits] == WT){
		  if(outcome == NOTTAKEN && (GlobalPredictor[GHR_Lower_bits] < WT )) ChoicePredictor[GHR_Lower_bits] = ST;
		  else if(outcome == TAKEN && (GlobalPredictor[GHR_Lower_bits] >= WT)) ChoicePredictor[GHR_Lower_bits] = ST;
		  else ChoicePredictor[GHR_Lower_bits] = WN; 
		}
		
		else if(ChoicePredictor[GHR_Lower_bits] == ST){
		  if(outcome == NOTTAKEN && (GlobalPredictor[GHR_Lower_bits] < WT)) ChoicePredictor[GHR_Lower_bits] = ST;
		  else if(outcome == TAKEN && (GlobalPredictor[GHR_Lower_bits] >= WT)) ChoicePredictor[GHR_Lower_bits] = ST;
		  else ChoicePredictor[GHR_Lower_bits] = WT;
		}		  
		}
	
	switch(LocalPredictor[flag]) {
		case SN:
		  LocalPredictor[flag] = (outcome == TAKEN) ? WN : SN;
		  break;
		case WN:
		  LocalPredictor[flag] = (outcome == TAKEN) ? WT : SN;
		  break;
		case WT:
		  LocalPredictor[flag] = (outcome == TAKEN) ? ST : WN;
		  break;
		case ST:
		  LocalPredictor[flag] = (outcome == TAKEN) ? ST : WT;
		  break;
		default:
          break;
  }
  
    switch(GlobalPredictor[GHR_Lower_bits]) {
		case SN:
          GlobalPredictor[GHR_Lower_bits] = (outcome == TAKEN) ? WN : SN;
          break;
		case WN:
          GlobalPredictor[GHR_Lower_bits] = (outcome == TAKEN) ? WT : SN;
          break;
		case WT:
          GlobalPredictor[GHR_Lower_bits] = (outcome == TAKEN) ? ST : WN;
          break;
		case ST:
          GlobalPredictor[GHR_Lower_bits] = (outcome == TAKEN) ? ST : WT;
          break;
		default:
          break;
  }
  

	PatternHistory[pc_lower_bits] = (PatternHistory[pc_lower_bits] << 1)|outcome;
	GHR = (GHR << 1) | outcome;   
}

void
cleanup_Tournament() {
  free(ChoicePredictor);
  free(LocalPredictor);
  free(GlobalPredictor);
  free(PatternHistory);
}

//Perceptron Functions combination with gshare
void
init_Perceptron(){
	int i = 0;
	int j = 0;
	int historyBits = 1 << Perceptron; //The total index of the Perceptron table historyBits = 128
	x = (int*) malloc(history_Bits * sizeof(int)); //size of the history table 31(bits) 
	
	Perceptrons_Table = (int**) malloc(historyBits * sizeof(int*)); //First Dimension of the array, the total index is 128
	//Size of each weights in index, Total Size = 31*8*128+31*2 = 31806 bits < 32K+320 bits//
	//7 comes from log2(1.93*N+14)+1, N is the history bits.                              //
	for(i = 0; i <= historyBits; i++){
		Perceptrons_Table[i] = (int*) malloc(history_Bits * sizeof(int)); 
	}
	
	for(i = 0; i <= historyBits; i++){
		for(j = 0; j <= history_Bits; j++)
		Perceptrons_Table[i][j] = 0; //Initial weight;
	}
	
	for(i = 0; i <= history_Bits; i++){
		x[i] = -1; //-1 means Nottaken in Perceptron predictor, initiallize to be not taken
	}
	GHR = 0;
}

uint8_t Perceptron_predict(uint32_t pc){
	int y = 1; //outcome of the dot product of matrix, Biased value W0*x0: 1*1
	int historyBits = 1 << Perceptron; //number of Perceptron predictor index
	int pc_lower = pc & (historyBits - 1); //Lower 7 bits of PC
	int GHR_Lower_Bits = GHR & (historyBits - 1); //Lower 7 bits of GHR
	int pc_lower_bits = GHR_Lower_Bits ^ pc_lower; //reduce alias
	
	for(int i = 0; i < history_Bits; i++){
		y += Perceptrons_Table[pc_lower_bits][i] * x[i]; //Dot product of the weights and past history
	}
	
	if(y>=0) return TAKEN;
	else    return NOTTAKEN;
}

void
train_Perception(uint32_t pc, uint8_t outcome){
	int y = 1; //Biased value W0*x0: 1*1
	float threshold = 1.93*history_Bits+14; //threshold to determine whether it needs training
	uint32_t historyBits = 1 << Perceptron; //number of Perceptron predictor index
	int pc_lower = pc & (historyBits - 1); //Lower 7 bits of PC
	int GHR_Lower_Bits = GHR & (historyBits - 1); //Lower 7 bits of GHR
	int pc_lower_bits = GHR_Lower_Bits ^ pc_lower; //reduce alias
	
    int      max = (1 << 7) - 1;
	int      min = -max - 1;                         // To constrain the bits in the range of 8 bits signed interger
	
	for(int i = 0; i < history_Bits; i++){
		y += Perceptrons_Table[pc_lower_bits][i] * x[i];
	}
	
	int flag = (outcome == TAKEN) ? 1:(-1);  //Transfer the outcome to -1(Not taken) and 1(Taken)
	int flag1 = (y < 0) ? -1:1;
	
	if((abs(y) <= threshold) || flag1 != flag){
		for(int i = 0; i <= history_Bits; i++){
			if(flag != x[i] && Perceptrons_Table[pc_lower_bits][i] > min) Perceptrons_Table[pc_lower_bits][i] = Perceptrons_Table[pc_lower_bits][i] - 1;
			else if(Perceptrons_Table[pc_lower_bits][i] <= min) Perceptrons_Table[pc_lower_bits][i] = min; // constrain the size within the range
			if(Perceptrons_Table[pc_lower_bits][i] < max && flag == x[i]) Perceptrons_Table[pc_lower_bits][i] = Perceptrons_Table[pc_lower_bits][i] + 1; 
			else if(Perceptrons_Table[pc_lower_bits][i] >= max) Perceptrons_Table[pc_lower_bits][i] = max; // constrain the size within the range
		}
	}
	//update the history table
	for(int i = history_Bits-1; i >= 1; i--){
		x[i] = x[i-1];
	}
	
	GHR = (GHR << 1) | outcome;
	x[0] = (outcome == TAKEN) ? 1:(-1); //shift the history table and update the branch predict outcome
	
}

void
cleanup_Perception() {
  free(Perceptrons_Table);
  free(x);
}


void
init_predictor()
{
  switch (bpType) {
    case STATIC:
    case GSHARE:
	  ghistoryBits = 10;
          init_gshare();
      break;
    case TOURNAMENT:
      ghistoryBits = 12;
	  init_Tournament();
	  break;
    case CUSTOM:
	  init_Perceptron();
    default:
      break;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_predict(pc);
    case TOURNAMENT:
      return Tournament_predict(pc);
    case CUSTOM:
	  return Perceptron_predict(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void
train_predictor(uint32_t pc, uint8_t outcome)
{

  switch (bpType) {
    case STATIC:
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return train_Tournament(pc,outcome);
    case CUSTOM:
	  return train_Perception(pc,outcome);
    default:
      break;
  }
  

}
