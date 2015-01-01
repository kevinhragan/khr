// Signature.h: interface for the CSignature class.
//
// Signature File Data Structures
//
//	If present, Dft[] always precedes Dft'[]. If nDftBlocks == 0 then there is no Dft data and the same for nDftPrimeBlock
//	
#define	STRINGSIZE	63
//	All samples are of type fftw_real.
//
struct newsigheader {
	char szMagic[4];
	long	nSamplesPerBlock;	// Must be 400
	long	nDftBlocks;		// Must be 1.
#ifdef LINUX
	long padding;
#endif
	double	fdCorrel;		// default Correlation Coefficient
	unsigned long	dAudioOffset;	// Offset of DCT block in the audio, resolution 50ms.
	long	nTimeout;	// Timeout in seconds
	char	sTag[STRINGSIZE+1];	//
	char	sHostname[STRINGSIZE+1];	// 
	char	sWaveFile[STRINGSIZE+1];	// 
	char	sDTMFTones[STRINGSIZE+1];	//
};

#define	SIGNEWHDRMAGIC	"2sng"	

class CSignature  
{
public:
	CSignature();
	virtual ~CSignature();

	CSignature *pNext, *pLast;
	BOOL bReady;
	int iListIndex;
	BOOL bLoadedWave;

	struct newsigheader SigHeader;

	// Pointer to Dft
	fftw_real	*pDft;
	fftw_real	*pMaskDft;
	fftw_real	*pZep;	// cepstrum
};
