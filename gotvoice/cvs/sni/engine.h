#ifndef GV_ENGINE_H
#define GV_ENGINE_H
#include "fftw.h"
#include "rfftw.h"
#include "signature.h"
#include "enginestruct.h"
#include <sys/time.h>
#define AUDIOSAMPERBLOCK	2000
#define DFTSAMPERBLOCK	((AUDIOSAMPERBLOCK/2)+1)

typedef struct _tagDftFileBuffer {
	fftw_real	*pDft;
	int	nSamples;
	int	iBlockNumber;	// In file, useful.
	struct _tagDftFileBuffer *pNext, *pLast;
} DftFileBuffer;;

class CStaticEngine
{
public:
	CStaticEngine(double fMeanFactor = 2.0, double threshold = 0.50, int quantum=10, int aspb = 2000);
	~CStaticEngine();

	// Process Samples
	BOOL	SignatureReport(short *audiobuffer, char *result = NULL);
	BOOL	ScanReport(short *audiobuffer, char *result, int xl = 0);
	BOOL	MatchesSignature(short *audiobuffer, char *siglist);
	BOOL	CompareAudio(short *abuffer, short *sigbuffer);
	BOOL	LoadSignatures(int cc = 0, char **cv = NULL, char *libdir = NULL);
	
	BOOL 	GetDftStats(fftw_real *pDft, DftStats *pStats, fftw_real fMeanFactor = 2.0);
	BOOL	MakeFDft(fftw_real *pAudio, fftw_real *pDft, fftw_real *pfTotalPower = NULL);
	BOOL	MakeDft(short *pAudio, fftw_real *pDft, fftw_real *pfTotalPower = NULL);
	BOOL	MakeRealToComplexFDft(fftw_real *pAudio, fftw_real *pCDft);
	BOOL	MakeComplexToRealFDft(fftw_real *pCDft, fftw_real *pAudio);
	BOOL	MakeCepstrum(fftw_real *pDft, fftw_real *pCepstrum);
	BOOL	MaskVoice(fftw_real *p_mask_dft, fftw_real *p_in_dft, int iLo, int iHi);
	fftw_real Correlate(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor);
	fftw_real VCorrelate(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor);

	int	nAudioSamPerBlock;	// Mirrors nSamples, also = (nDftSamPerBlock-1)*2, read from library
	int	nDftSamPerBlock;
	int	nCepSamPerBlock;
	
	// Profile Settings
	CSignature *pFirstSignature, *pLastSignature;
	int nSignature;

private:

	rfftw_plan thePlan;
	rfftw_plan theForwardPlan;
	rfftw_plan theReversePlan;
	rfftw_plan theCPlan;
	rfftw_plan theReverseCPlan;

	int nWindowFraction;
	BOOL	bInitialized;

	// FFT Variables
	fftw_real *pIn;
	fftw_real *pOut;
	fftw_real *pPwr;

	fftw_real	fMeanFactor;
	fftw_real	fThreshold;

	// data used by Engine & ProcessSamples 
	short	*pLastAudio;
	
};

#endif
