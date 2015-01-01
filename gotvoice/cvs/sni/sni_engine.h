#ifndef GV_ENGINE_H
#define GV_ENGINE_H
#include "fftw.h"
#include "rfftw.h"
#include <sys/time.h>
#include <math.h>
#include "sni_sig.h"

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

#define AUDIOSAMPERBLOCK	1000
#define DFTSAMPERBLOCK	((AUDIOSAMPERBLOCK/2)+1)

class CSniEngine
{
public:
	CSniEngine();
	~CSniEngine();

	void Initialize(int aspb = AUDIOSAMPERBLOCK, int quantum=5);
	// Process Samples
	BOOL	SaveSignature(CSniSig *pSig, char *filename, BOOL bDft = TRUE);
	CSniSig	*LoadSignature(char *filename);
	void PrintSignatureHeader(CSniSig *pSig, char *snifile);
	BOOL	LoadSignatures(int cc = 0, char **cv = NULL, char *libdir = NULL);

	BOOL	MakeDft(short *pAudio, fftw_real *pDft, fftw_real *pfTotalPower = NULL);
	BOOL	MakePhaseDft(short *pAudio, fftw_complex *pDft, fftw_real *pfTotalPower = NULL);
	BOOL	MakeRealToComplexFDft(fftw_real *pAudio, fftw_real *pCDft);
	BOOL	MakeComplexToRealFDft(fftw_real *pCDft, fftw_real *pAudio);
	fftw_real Correlate(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor);
	fftw_real Correlate2(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor);
	fftw_real Distance(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor);
	fftw_real VectorDistance(CFeatureVector *pvU, CFeatureVector *pvK);
	sni_block *ExtractLikeBlocks(sni_block *pbr, sni_block *pbc, fftw_real fThreshold);
	sni_block *MergeBlocks(sni_block *pb1, sni_block *pb2);
	sni_result *CompareBlock(sni_block *pbr, sni_block *pbc, fftw_real fMeanFactor = 2.0, BOOL bEverything = FALSE);
	sni_result *ComputeDistances(sni_block *pbr, sni_block *pbc, fftw_real fMeanFactor = 2.0);
	sni_result *ComputeVectorDistances(CSniSig *psU, CSniSig *psK);
	void ComputeClusterCenters(hitpoint *centers, int *nc, fftw_real *dtab, int ndimension);
	fftw_real CompareSummary(sni_block *pb1, sni_block* pb2, fftw_real fMeanFactor=2.0);
	fftw_real SummaryDistance(sni_block *pb1, sni_block* pb2, fftw_real fMeanFactor=2.0);
	int *LoadPMatrix(char *filename = NULL);
	fftw_real ScoreFeatureVector(int *pmatrix, CFeatureVector *pfv);

	CFeatureVector *ComputeFeatureVector(fftw_complex *pDft, int ilifter = 20);
	CFeatureVector *ComputeDftPeaks(fftw_complex *pDft, fftw_real fMeanFactor = 2.0);

	int	nAudioSamPerBlock;	// Mirrors nSamples, also = (nDftSamPerBlock-1)*2, read from library
	int	nDftSamPerBlock;
	
	// Profile Settings
	CSniSig *pFirstSignature, *pLastSignature;
	int nSignature;

	rfftw_plan thePlan;
	rfftw_plan theForwardPlan;
	rfftw_plan theReversePlan;

private:
	// FFT Variables
	fftw_real *pIn;
	fftw_real *pOut;
	fftw_real *pPwr;
	fftw_real *pCep;
};

#endif
