#ifndef _ENGINESTRUCT_H_
#define	_ENGINESTRUCT_H_

//
//	(c) 2001, Martin R.M. Dunsmuir, All Rights Reserved
//
//	Snatchmail Recognition Engine Class
//
//
//	State Table Structures
//
//	Three tiered structure
//
//	REngine.pCurrentState -> current state (starting state must have label :start
//	REngine.pStateTable -> chain of StateTableEntry's each containing 
//				the label (szLabel)
//				this->pEventList -> a chain of events for this STE
//							the event type
//							this->pActionList -> a chain of actions for this Event
//			
//				Each STA contains the verb (as a string) and room for up to 4 string parameters
//
class CSignature;

// Signature File Data Structures
//
//	If present, Dft[] always precedes Dft'[]. If nDftBlocks == 0 then there is no Dft data and the same for nDftPrimeBlock
//	
#define	SIGTOKENSIZE	63
//	All samples are of type fftw_real.
//
struct sigheader {
	unsigned char szMagic[4];
	long	nSamplesPerBlock;
	long	nDftBlocks;
	long	nDftPrimeBlocks;
	char	sToken[SIGTOKENSIZE+1];	// Name of this word. Upto STS chars, NULL terminated.
};

#define	SIGHDRMAGIC	".sng"	

#define	MAXSYMBOLLENGTH	64	// maximum length of symbol
#define MAXPEAKS	1000
#define MAXTRIGGERS	30
#define TONEWIDTH	5

// DFT Statistics structure
typedef struct _tagDftStats {
	fftw_real fMax, fMedian, fMean, fSum;
	fftw_real fMaxPower;
	fftw_real fMeanPower;
	fftw_real fTotalPower;
	int iPeaks[MAXPEAKS];
	int iTroughs[MAXPEAKS];
	fftw_real fPower[MAXPEAKS];
	fftw_real fTroughPower[MAXPEAKS];
	int nPeaks;
	int nTroughs;
	int nTriggers;
	int iTriggers[MAXTRIGGERS];
	fftw_real fTruePower;
} DftStats;

#endif
