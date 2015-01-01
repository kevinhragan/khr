// Signature.h: interface for the CSignature class.
#include <string.h>
#include <stdio.h>
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
        long    nSamplesPerBlock;       // Must be 400
        long    nDftBlocks;             // Must be 1.
#ifdef LINUX
        long padding;
#endif
        double  fdCorrel;               // default Correlation Coefficient
        unsigned long   dAudioOffset;   // Offset of DCT block in the audio, resolution 50ms.
        long    nTimeout;       // Timeout in seconds
        char    sTag[STRINGSIZE+1];     //
        char    sHostname[STRINGSIZE+1];        //
        char    sWaveFile[STRINGSIZE+1];        //
        char    sDTMFTones[STRINGSIZE+1];       //
};

#define SIGNEWHDRMAGIC  "2sng"

typedef struct _hitpoint
{
	int nhits;
	int dftindex;
	int nchains;	// >= 2
	int maxchain;	
	int nkluster;
} hitpoint;

inline BOOL isperiodic(double small, double big)
{

	double c, f;
	double r;

	r = big/small;
	c = ceil(r);
	f = floor(r);

	double min;

	if( fabs(c-r) < fabs(f-r) )
		min = fabs(c-r);
	else
		min = fabs(f-r);

	if( min < 0.08 )
		return TRUE;
	else
		return FALSE;
};

// new stuff for speaker identification.
// this new format will also work for new signatures in the future.
//
//	The structure of the new sni files is more flexible than the old sng file and it has been trimmed of superfluous info.
//
//	The file is structured as follows:
//
//		sni_header
//		sni_block
//		sni_block
//		featurevectors
//		...		upto sig_header.nDftBlocks
//		dft data...
//	
//	each sni_block points to DFT's of size nDftSamPerBlock*sizeof(double);
//
#define SNIT_POWER	1
#define SNIT_PHASE	2
#define SNIT_SNG	3
#define SNIHDRMAGIC	"3sng"

//	sni_block flags
#define SNIF_LIFTER	4

struct _sni_block {
	long magic;
	long	nDfts;
	long	iFileOffset;	// offset into the sni file to find beginning of nDfts arrays of size: 
	long	iAudioOffset;	// in nAudioSamPerBlock's
	long	nQuantum;	// nDftSamPerBlock*sizeof(double)*nQuantum*iType
	long	nAudioSamPerBlock;	// doublecheck
	long	nDftSamPerBlock;	// doublecheck == nAudioSamPerBlock/2 (i.e. no DC component).
	short	iType;		// type of DFT
	short	iSizeof;	// sizeof(fftw_real) or sizeof(fftw_complex)
	long 	iFlags;
};

typedef struct __sni_block {
	struct _sni_block block;
	struct __sni_block *pnext, *plast;
	fftw_real *pdata;
} sni_block;

typedef struct _sni_header {
	char szmagic[4];	// == "3sng"
	time_t	tCreate;		// time the sni was created
	char	sTag[STRINGSIZE+1];	// identity of caller
	char	sRouting[STRINGSIZE+1];	// the account the message was pulled for.
	char	sMessageId[STRINGSIZE+1];	// the message ID
	char	sCallerId[STRINGSIZE+1];	// any caller id information for this message
	char	sAuthor[STRINGSIZE+1];	// any caller id information for this message
	long 	nDftBlocks;		// number of following sni_block's
	long	iVectorFileOffset;	// offset to vectors.
	long	nVectors;		// number of feature vectors.
} sni_header;

//
//	correlation result
//	<col0>
//	<col1> etc.
//	
//	rows come from pbr, cols from pbc
typedef struct _sni_result {
	sni_block *pbr, *pbc;
	int rows, cols;
	fftw_real *presult;
	struct _sni_result *pnext, *plast;
} sni_result;


typedef struct _formant
{
	fftw_real fcenter;	// center frequency
	fftw_real fpower;	// log power
} sni_formant;

typedef struct _vector_hdr
{
	long nformant;
	long npitch;
	double fenergy;
	long isampleoffset;
} sni_vector_hdr;

class CFeatureVector 
{
public:
	inline CFeatureVector(int nformant, int npitch)
	{
		if( nformant > 0 )
		{
			pFormant = (sni_formant *)calloc(nformant,sizeof(sni_formant)); 
			nFormant = nformant;
		}
		else
		{
			pFormant = NULL;
			nFormant = 0;
		}

		if( npitch > 0 )
		{
			pPitch = (sni_formant *)calloc(npitch,sizeof(sni_formant)); 
			nPitch = npitch;
		}
		else
		{
			pPitch = NULL;
			nPitch = 0;
		}

		plast = pnext = NULL;
		fEnergy = 0;
		iSampleOffset = 0;
	};

	inline ~CFeatureVector()
	{
		if(pFormant)free( pFormant );
		if(pPitch)free( pPitch );
	};

	inline void SaveVector(FILE *fp)
	{
		sni_vector_hdr vhdr;

		vhdr.nformant = nFormant;
		vhdr.npitch = nPitch;
		vhdr.fenergy = fEnergy;
		vhdr.isampleoffset = iSampleOffset;

		fwrite(&vhdr, 1, sizeof(sni_vector_hdr), fp);

		int i;

		for( i=0; i<nFormant; i++ )
			fwrite(&pFormant[i], 1, sizeof(sni_formant), fp);

		for( i=0; i<nPitch; i++ )
			fwrite(&pPitch[i], 1, sizeof(sni_formant), fp);
	};

	sni_formant *pFormant;
	sni_formant *pPitch;
	double fEnergy;		// original mean energy/sample in frame.
	long iSampleOffset;
	int nFormant, nPitch;
	CFeatureVector *pnext, *plast;
};

class CSniSig
{
public:
	inline CSniSig()
	{
		pNext = pLast = NULL;
		pFirstBlock = pLastBlock = NULL;
		memset(&header, 0, sizeof(sni_header));	// clear the header.
		pFirstFV = pLastFV = NULL;
		this->header.nVectors = 0;
		this->header.iVectorFileOffset = 0;
	};

	inline ~CSniSig()
	{
	};

	inline void AddFeatureVector(CFeatureVector *pFV)
	{
		if( pFirstFV )
		{
			pLastFV->pnext = pFV;
			pFV->plast = pLastFV;
			pLastFV = pFV;
		}
		else
		{
			pFirstFV = pLastFV = pFV;
		}
		this->header.nVectors++;
	};

	sni_header header;
	sni_block *pFirstBlock, *pLastBlock;
	CFeatureVector *pFirstFV, *pLastFV;

	CSniSig *pNext, *pLast;
};
