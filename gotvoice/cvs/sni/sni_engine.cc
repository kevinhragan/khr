#include <stdio.h>
#include "sni.h"
#include "fftw.h"
#include "rfftw.h"
#include "sni_engine.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <linux/soundcard.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <math.h>

extern CSniEngine *pTheEngine;
static char szRcsId[] = "$Id: sni_engine.cc,v 1.34 2005-06-21 00:06:23 martind Exp $";

CSniEngine::CSniEngine()
{
	pFirstSignature = NULL;
	pLastSignature = NULL;
	nSignature = 0;
	nDftSamPerBlock = 0;
	nAudioSamPerBlock = 0;	// 250ms
	pCep = pIn = pOut = pPwr = NULL;
}

void CSniEngine::Initialize(int aspb, int quantum)
{
	int i;

	nDftSamPerBlock = aspb/2+1;
	nAudioSamPerBlock = aspb;	// 250ms

	// First We Initialize the FFT Engine.
	if( bDebug && (nDebug >= 1) )
		printf("creating FFT plans for %d samples\n", this->nAudioSamPerBlock);

	//	Create FFT Plan
	this->thePlan = rfftw_create_plan(this->nAudioSamPerBlock, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);

	// cepstral plans.
	this->theForwardPlan = rfftw_create_plan(this->nAudioSamPerBlock, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
	this->theReversePlan = rfftw_create_plan(this->nAudioSamPerBlock, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);

	if( pIn ) free(pIn); if( pOut ) free(pOut); if( pPwr ) free(pPwr); if( pCep ) free(this->pCep);
	// Allocate FFT Buffers
	// pIn is twice the size to allow windowing.
	this->pIn = (fftw_real *)malloc(sizeof(fftw_real)*this->nAudioSamPerBlock*2);
	this->pOut = (fftw_real *)malloc(sizeof(fftw_real)*this->nAudioSamPerBlock+1);
	this->pPwr = (fftw_real *)malloc(sizeof(fftw_real)*(this->nAudioSamPerBlock+1));	// for complex
	this->pCep = (fftw_real *)malloc(sizeof(fftw_real)*(this->nAudioSamPerBlock+1));	// for complex
}

CSniEngine::~CSniEngine()
{
	free(this->pIn);
	free(this->pOut);
	free(this->pPwr);
	free(this->pCep);
}

// data is returned in r, theta format.
//
BOOL	CSniEngine::MakePhaseDft(short *pAudio, fftw_complex *pDft, fftw_real *pfTotalPower)
{
	fftw_real	fSum, fMean;
	int j, k;

	for( j=0; j<nAudioSamPerBlock; j++ )
	{
		this->pIn[j] = (fftw_real)pAudio[j];
		if( pfTotalPower )
		{
			*pfTotalPower += pow(this->pIn[j],2);
		}
		// apply hamming window
		this->pIn[j] *= 0.54 - 0.46*cos((2*M_PI*j)/(nAudioSamPerBlock-1));
	}

	// Perform the FFT
	rfftw_one(this->thePlan, pIn, pOut);

	// overload re, im as r, th.

	pDft[0].re = sqrt(pOut[0]*pOut[0]);
	pDft[0].im = 0;

	for( k=1; k<nAudioSamPerBlock/2; k++ )
	{
		pDft[k].re = sqrt(pOut[k]*pOut[k]+pOut[nAudioSamPerBlock-k]*pOut[nAudioSamPerBlock-k]);

		if( (pOut[k]>0) && (pOut[nAudioSamPerBlock-k]>0) )
                        // first quadrant ++
                        pDft[k].im = atan(pOut[nAudioSamPerBlock-k]/pOut[k]);
                else if( (pOut[k]<0) && (pOut[nAudioSamPerBlock-k]>0) )
                        // second quadrant -+
                        pDft[k].im = M_PI-atan(pOut[nAudioSamPerBlock-k]/-pOut[k]);
                else if( (pOut[k]<0) && (pOut[nAudioSamPerBlock-k]<0) )
                        // third quadrant --
                        pDft[k].im = M_PI+atan(-pOut[nAudioSamPerBlock-k]/-pOut[k]);
                else if( (pOut[k]>0) && (pOut[nAudioSamPerBlock-k]<0) )
                        // fourth quadrant +-
                        pDft[k].im = 2*M_PI-atan(-pOut[nAudioSamPerBlock-k]/pOut[k]);
                else
                        pDft[k].im = 0;

	}

	// Normalize to the Mean
	fSum = 0.0;
	fMean = 0.0;

	for( k = 0; k < nAudioSamPerBlock/2; k++)
		fSum += pDft[k].re;

	fMean = fSum/k;

	for( k = 0; k < nAudioSamPerBlock/2; k++)
		pDft[k].re = pDft[k].re/fMean;

	if( bDebug && (nDebug >= 2))
	{
		for( k=0; k<5; k++ )
			printf("[%-10f %-10f] ", pDft[k].re, pDft[k].im);
		printf("\n");
	}

	if( pfTotalPower )
		*pfTotalPower = sqrt(*pfTotalPower/nAudioSamPerBlock);

	return(TRUE);
}

BOOL	CSniEngine::MakeDft(short *pAudio, fftw_real *pDft, fftw_real *pfTotalPower)
{
	fftw_real	fSum, fMean;
	int j, k;

	for( j=0; j<nAudioSamPerBlock; j++ )
	{
		this->pIn[j] = (fftw_real)pAudio[j];
		if( pfTotalPower )
		{
			*pfTotalPower += pow(this->pIn[j],2);
		}
	}

	// Perform the FFT
	rfftw_one(this->thePlan, pIn, pOut);

	// Now set up the power spectrum
	pDft[0] = sqrt(pOut[0] * pOut[0]);	// DC component

	for( k = 1; k < (nAudioSamPerBlock+1)/2; k++ )
		pDft[k] = sqrt(pOut[k]*pOut[k] + pOut[nAudioSamPerBlock-k]*pOut[nAudioSamPerBlock-k]);

	if( nAudioSamPerBlock%2 == 0 )	// N is even
		pDft[nAudioSamPerBlock/2] = sqrt(pOut[nAudioSamPerBlock/2]*pOut[nAudioSamPerBlock/2]);

	// Normalize to the Mean
	fSum = 0.0;
	fMean = 0.0;

	for( k = 0; k < nAudioSamPerBlock/2 + 1; k++)
		fSum += pDft[k];

	fMean = fSum/k;

	for( k = 0; k < nAudioSamPerBlock/2 + 1; k++)
		pDft[k] = pDft[k]/fMean;


	if( bDebug && (nDebug >=2))
	{
		for( k=0; k<10; k++ )
			printf("%-10f", pDft[k]);
		printf("\n");
	}

	if( pfTotalPower )
		*pfTotalPower = sqrt(*pfTotalPower/nAudioSamPerBlock);

	return(TRUE);
}

//	LoadSignatures - supercedes LoadWordTable: 
//
//	Loads the Signatures referenced by the given host "hostname"
//	into memory. The newly loaded signatures are attached to this->pSignatures.
//
BOOL CSniEngine::LoadSignatures(int cc, char **cv, char *libdir)
{
	// Clear any old signatures
	if( this->pFirstSignature == NULL )
		this->pLastSignature = NULL;
	else
	{
		CSniSig *pSig, *pxSig;

		// Clean out the Lot.
		for( pSig = this->pFirstSignature; pSig; pSig = pxSig )
		{
			pxSig = pSig->pNext;
			delete pSig;
		}
		this->pFirstSignature = NULL;
		this->pLastSignature = NULL;
	}

	if( bDebug && (nDebug >= 0) )
		printf("loading signatures\n");

	int is, nr;

	if( libdir )
	{ 
#define LINEBUFSIZE 256
		char libname[LINEBUFSIZE];

		// we load from a library
		DIR *sigdir = opendir(libdir);

		if( sigdir == NULL )
		{
			fprintf(stderr, "cannot open library '%s'\n", libdir);
			return FALSE;
		}

		struct dirent *dep;

		while( (dep = readdir(sigdir)) )
		{
			if( strstr(dep->d_name, ".sni") == NULL )
				continue;

			sprintf(libname, "%s/%s", libdir, dep->d_name);

			if( bDebug && (nDebug >= 1) )
				printf("loading signature %s\n", libname);

			CSniSig *pNewSig = this->LoadSignature(libname);

			if( pNewSig == NULL )
			{
				printf("cant open signature file %s\n", libname);
				delete pNewSig;
				continue;
			}

			// Link the Signature onto theApp.pHostWizard->pSig;
			if( this->pFirstSignature )
			{
				this->pLastSignature->pNext = pNewSig;
				pNewSig->pLast = this->pLastSignature;

				this->pLastSignature = pNewSig;
			}
			else
			{
				this->pLastSignature = pNewSig;
				this->pFirstSignature = pNewSig;
			}
		}

		closedir(sigdir);
	}
	else
	{
		for( is = 0; is < cc; is++ )
		{
			if( bDebug && (nDebug >= 0) )
				printf("loading signature %s\n", cv[is]);

			CSniSig *pNewSig = this->LoadSignature(cv[is]);

			if( pNewSig == NULL )
			{
				printf("cant open signature file %s\n", cv[is]);
				delete pNewSig;
				continue;
			}

			// Link the Signature onto theApp.pHostWizard->pSig;
			if( this->pFirstSignature )
			{
				this->pLastSignature->pNext = pNewSig;
				pNewSig->pLast = this->pLastSignature;

				this->pLastSignature = pNewSig;
			}
			else
			{
				this->pLastSignature = pNewSig;
				this->pFirstSignature = pNewSig;
			}

		}
	}

	// Now Everything is Kosher
	return TRUE;
}

//
//	treating each point as a vector component, compute the Euclidian distance between the two spectra
//	the comparison is currently done on the summaries. But we need to get more sophisticated.
//
//
fftw_real CSniEngine::SummaryDistance(sni_block *pb1, sni_block* pb2, fftw_real fMeanFactor)
{
	int id, iq, is;

	if( (pb1->block.nDftSamPerBlock != pb2->block.nDftSamPerBlock) || (pb2->block.iType != SNIT_PHASE) 
		|| (pb2->block.iType != SNIT_PHASE) )
	{
		printf("mismatched dfts\n");
		return 0;
	}

	// first we compute the sumamries
	fftw_real *summary1 = (fftw_real *)calloc(pb1->block.nDftSamPerBlock, sizeof(fftw_real));
	fftw_real *summary2 = (fftw_real *)calloc(pb2->block.nDftSamPerBlock, sizeof(fftw_real));

	fftw_real ftotal1 = 0;
	fftw_real fmax1 = 0;

	for( id=0; id<pb1->block.nDfts; id++ )
	{
		for( iq=0; iq<pb1->block.nQuantum; iq++)
		{
			int ibase = (id*pb1->block.nDftSamPerBlock*pb1->block.nQuantum)+(pb1->block.nDftSamPerBlock*iq);

			for( is=0; is<pb1->block.nDftSamPerBlock; is++)
			{
				fftw_complex *pc = (fftw_complex *)pb1->pdata;

				if( pc[ibase+is].re > fmax1 )fmax1 = pc[ibase+is].re;
				summary1[is] += pc[ibase+is].re;
				ftotal1 += pc[ibase+is].re;
			}
		}
	}

	ftotal1 /= pb1->block.nDfts*pb1->block.nQuantum;

	for(is = 0; is < pb1->block.nDftSamPerBlock; is++ )
		summary1[is] /= pb1->block.nDfts*pb1->block.nQuantum;

	fftw_real ftotal2 = 0;
	fftw_real fmax2 = 0;

	for( id=0; id<pb2->block.nDfts; id++ )
	{
		for( iq=0; iq<pb2->block.nQuantum; iq++)
		{
			int ibase = (id*pb2->block.nDftSamPerBlock*pb2->block.nQuantum)+(pb2->block.nDftSamPerBlock*iq);

			for( is=0; is<pb2->block.nDftSamPerBlock; is++)
			{
				fftw_complex *pc = (fftw_complex *)pb2->pdata;

				if( pc[ibase+is].re > fmax2 )fmax2 = pc[ibase+is].re;
				summary2[is] += pc[ibase+is].re;
				ftotal2 += pc[ibase+is].re;
			}
		}
	}

	ftotal2 /= pb2->block.nDfts*pb2->block.nQuantum;

	for(is = 0; is < pb2->block.nDftSamPerBlock; is++ )
		summary2[is] /= pb2->block.nDfts*pb2->block.nQuantum;

	fftw_real result = this->Distance(summary1, summary2, pb1->block.nDftSamPerBlock, fMeanFactor);

	free(summary1);
	free(summary2);

	return(result);
}

// add all the spectra in a block, then correlate
//
fftw_real CSniEngine::CompareSummary(sni_block *pb1, sni_block* pb2, fftw_real fMeanFactor)
{
	int id, iq, is;

	if( (pb1->block.nDftSamPerBlock != pb2->block.nDftSamPerBlock) || (pb2->block.iType != SNIT_PHASE) 
		|| (pb2->block.iType != SNIT_PHASE) )
	{
		printf("mismatched dfts\n");
		return 0;
	}
	fftw_real *summary1 = (fftw_real *)calloc(pb1->block.nDftSamPerBlock, sizeof(fftw_real));
	fftw_real *summary2 = (fftw_real *)calloc(pb2->block.nDftSamPerBlock, sizeof(fftw_real));

	fftw_real ftotal1 = 0;
	fftw_real fmax1 = 0;

	for( id=0; id<pb1->block.nDfts; id++ )
	{
		for( iq=0; iq<pb1->block.nQuantum; iq++)
		{
			int ibase = (id*pb1->block.nDftSamPerBlock*pb1->block.nQuantum)+(pb1->block.nDftSamPerBlock*iq);

			for( is=0; is<pb1->block.nDftSamPerBlock; is++)
			{
				fftw_complex *pc = (fftw_complex *)pb1->pdata;

				if( pc[ibase+is].re > fmax1 )fmax1 = pc[ibase+is].re;
				summary1[is] += pc[ibase+is].re;
				ftotal1 += pc[ibase+is].re;
			}
		}
	}

	ftotal1 /= pb1->block.nDfts*pb1->block.nQuantum;

	for(is = 0; is < pb1->block.nDftSamPerBlock; is++ )
		summary1[is] /= pb1->block.nDfts*pb1->block.nQuantum;

	fftw_real ftotal2 = 0;
	fftw_real fmax2 = 0;

	for( id=0; id<pb2->block.nDfts; id++ )
	{
		for( iq=0; iq<pb2->block.nQuantum; iq++)
		{
			int ibase = (id*pb2->block.nDftSamPerBlock*pb2->block.nQuantum)+(pb2->block.nDftSamPerBlock*iq);

			for( is=0; is<pb2->block.nDftSamPerBlock; is++)
			{
				fftw_complex *pc = (fftw_complex *)pb2->pdata;

				if( pc[ibase+is].re > fmax2 )fmax2 = pc[ibase+is].re;
				summary2[is] += pc[ibase+is].re;
				ftotal2 += pc[ibase+is].re;
			}
		}
	}

	ftotal2 /= pb2->block.nDfts*pb2->block.nQuantum;

	for(is = 0; is < pb2->block.nDftSamPerBlock; is++ )
		summary2[is] /= pb2->block.nDfts*pb2->block.nQuantum;

	fftw_real result = this->Correlate2(summary1, summary2, pb1->block.nDftSamPerBlock, fMeanFactor);

	free(summary1);
	free(summary2);

	return(result);
}

//
//	very simple
//
CFeatureVector *CSniEngine::ComputeDftPeaks(fftw_complex *pDft, fftw_real fMeanFactor)
{
	int i;
	CFeatureVector *pFV = NULL;

	// everything is half complex.
	fftw_real *pData = pIn;

	// now we set up the data. as a half-complex array.
	for( i=0; i<this->nDftSamPerBlock; i++)
	{
		pData[i] = pDft[i].re;
	}

	sni_formant formants[this->nDftSamPerBlock];	// overkill
	int nformant = 0;

	fftw_real dldt = 0.0;
	fftw_real flast = pData[0];
	BOOL bInPeak = FALSE;
	fftw_real fpower, fmax;
	int istart, iend, imax;

	for( i=2; i<this->nDftSamPerBlock-2; i++)
	{
		fftw_real fthis = (pData[i]+pData[i-1]+pData[i+1]+pData[i-2]+pData[i+2])/5;
		fftw_real fdelta = fthis - flast;

		if( bInPeak )
		{
			fpower += fthis;
			if( fthis >= fmax )
			{
				fmax = fthis;
				imax = i;
			}
		}

		if( bInPeak && (fdelta < 0) && (dldt < 0) )
		{
			bInPeak = FALSE;
			iend = i;

			fftw_real fstep = 4000.0/(this->nAudioSamPerBlock/2); // Hz per data index

			if( fmax > fMeanFactor )
			{
				formants[nformant].fcenter = fstep*imax;
				formants[nformant].fpower = fmax;
				nformant++;
			}

		}
		else if( (fdelta < 0) && (dldt > 0) && (i>2) )
		{
			if( bInPeak == FALSE )
			{
				fmax = fpower = flast;
				imax = istart = i-1;
				bInPeak = TRUE;
			}
		}

		flast = fthis;
		dldt = fdelta;
	}

	if( nformant <= 0 )
		return NULL;

	pFV = new CFeatureVector(nformant, 0);

	for( i=0; i<nformant; i++ )
		pFV->pFormant[i] = formants[i];

	return pFV;
}
//
//	using homo-morphic deconvolution we create a feature vector for formants and principle pitch harmonics.
//
CFeatureVector *CSniEngine::ComputeFeatureVector(fftw_complex *pDft, int ilifter)
{
	int i;

	if( ilifter == 0 )
		return NULL;

	CFeatureVector *pFV = NULL;

	// everything is half complex.
	fftw_real *pData = pIn;

	pData[0] = 0;
	pData[this->nDftSamPerBlock/2] = 0;

	// now we set up the data. as a half-complex array.
	for( i=1; i<this->nDftSamPerBlock; i++)
	{
		// in the phase data format the data in already in |z|, arg(z) format!
		// log(z) = log(|z|) + j*arg(z)
		pData[i] = log(pDft[i].re);
		pData[this->nAudioSamPerBlock-i] = 2*M_PI*(pDft[i].im/360);
	}

	// compute the cepstrum, we end-up with this->nAudioSamPerBlock 'real' samples.
	rfftw_one(pTheEngine->theReversePlan, pData, pCep);

	// if we get here, we perform a homo-morphic decomposition (liftering) of the 
	// excitation and the formants.
	// first the formants
	// pCep now contains the complex cepstrum (real data), symmetric about the middle
	// of the buffer. 
	//
	// we provide two different masks, which can be applied to the cepstrum to achieve
	// de-convolution of U(w) (the excitation), from V(w)the formant envelope.
	// 
	for( i=ilifter; i<this->nDftSamPerBlock; i++ )
		pCep[i] = pCep[this->nAudioSamPerBlock-i] = 0;

	// now reverse the process
	rfftw_one(pTheEngine->theForwardPlan, pCep, pData);

	pData[0] /= this->nAudioSamPerBlock;
	for( i=1; i<this->nDftSamPerBlock; i++ )
	{
		pData[i] /= this->nAudioSamPerBlock;
		pData[this->nAudioSamPerBlock-i] /= this->nAudioSamPerBlock;
	}

	sni_formant formants[this->nDftSamPerBlock];	// overkill
	int nformant = 0;

	fftw_real dldt = 0.0;
	fftw_real flast = pData[1];
	BOOL bInPeak = FALSE;
	fftw_real fpower, fmax;
	int istart, iend, imax;

	// find the formants, by looking at the log(|z|) values. In the first half
	// of the half-complex array (see fftw 2.15 docs).
	//
	for( i=3; i<this->nAudioSamPerBlock/2-2; i++)
	{
		fftw_real fthis = (pData[i]+pData[i-1]+pData[i+1]+pData[i-2]+pData[i+2])/5;
		fftw_real fdelta = fthis - flast;

		if( bInPeak )
		{
			fpower += fthis;
			if( fthis >= fmax )
			{
				fmax = fthis;
				imax = i;
			}
		}

		if( bInPeak && (fdelta < 0) && (dldt < 0) )
		{
			bInPeak = FALSE;
			iend = i;

			fftw_real fstep = 4000.0/(this->nAudioSamPerBlock/2); // Hz per data index

			formants[nformant].fcenter = fstep*imax;
			formants[nformant].fpower = fmax;
			nformant++;

		}
		else if( (fdelta < 0) && (dldt > 0) && (i>2) )
		{
			if( bInPeak == FALSE )
			{
				fmax = fpower = flast;
				imax = istart = i-1;
				bInPeak = TRUE;
			}
		}

		flast = fthis;
		dldt = fdelta;
	}

	if( nformant <= 0 )
		return NULL;

	// now we calculate the pitch harmonics.
	pData[0] = 0;
	pData[this->nDftSamPerBlock/2] = 0;

	// now we set up the data. as a half-complex array.
	for( i=1; i<this->nDftSamPerBlock; i++)
	{
		// in the phase data format the data in already in |z|, arg(z) format!
		// log(z) = log(|z|) + j*arg(z)
		pData[i] = log(pDft[i].re);
		pData[this->nAudioSamPerBlock-i] = 2*M_PI*(pDft[i].im/360);
	}

	// compute the cepstrum, we end-up with this->nAudioSamPerBlock 'real' samples.
	rfftw_one(pTheEngine->theReversePlan, pData, pCep);

	//
	// we provide two different masks, which can be applied to the cepstrum to achieve
	// de-convolution of U(w) (the excitation), from V(w)the formant envelope.
	// 
	for( i=1; i<ilifter; i++ )
		pCep[i] = pCep[this->nAudioSamPerBlock-i] = 0;

	// now reverse the process
	rfftw_one(pTheEngine->theForwardPlan, pCep, pData);

	pData[0] /= this->nAudioSamPerBlock;
	for( i=1; i<this->nDftSamPerBlock; i++ )
	{
		pData[i] /= this->nAudioSamPerBlock;
		pData[this->nAudioSamPerBlock-i] /= this->nAudioSamPerBlock;
	}

#define MINPITCH 8
	int npitch = 0;
	sni_formant pitches[this->nDftSamPerBlock];
	fftw_real fdbcutoff = 0.0;

	do
	{
		npitch = 0;
		dldt = 0.0;
		flast = pData[1];
		bInPeak = FALSE;

		// find the formants, by looking at the log(|z|) values. In the first half
		// of the half-complex array (see fftw 2.15 docs).
		//
		for( i=3; i<this->nAudioSamPerBlock/2-2; i++)
		{
			fftw_real fthis = (pData[i]+pData[i-1]+pData[i+1]+pData[i-2]+pData[i+2])/5;
			fftw_real fdelta = fthis - flast;

			if( bInPeak )
			{
				fpower += fthis;
				if( fthis >= fmax )
				{
					fmax = fthis;
					imax = i;
				}
			}

			if( bInPeak && (fdelta < 0) && (dldt < 0) )
			{
				bInPeak = FALSE;
				iend = i;

				fftw_real fstep = 4000.0/(this->nAudioSamPerBlock/2); // Hz per data index

				if( fmax >= fdbcutoff )
				{
					pitches[npitch].fcenter = fstep*imax;
					pitches[npitch].fpower = fmax;
					npitch++;
				}

			}
			else if( (fdelta < 0) && (dldt > 0) && (i>2) )
			{
				if( bInPeak == FALSE )
				{
					fmax = fpower = flast;
					imax = istart = i-1;
					bInPeak = TRUE;
				}
			}

			flast = fthis;
			dldt = fdelta;
		}

		fdbcutoff -= 0.01;
	}
	while( npitch < MINPITCH );
	
	pFV = new CFeatureVector(nformant, npitch);

	for( i=0; i<nformant; i++ )
		pFV->pFormant[i] = formants[i];

	for( i=0; i<npitch; i++ )
		pFV->pPitch[i] = pitches[i];

	return pFV;
}

//
//	Use the feature vector for the distance computations
//
sni_result *CSniEngine::ComputeVectorDistances(CSniSig *psU, CSniSig *psK)
{
	sni_result *pni = NULL;
	pni = (sni_result *)calloc(sizeof(sni_result),1);

	pni->pbr = NULL;
	pni->pbc = NULL;
	pni->rows = psU->header.nVectors;
	pni->cols = psK->header.nVectors;

	if( bDebug && (nDebug >=0) )
		printf("vector rows = %d, cols = %d rxc = %d\n", pni->rows, pni->cols, pni->rows*pni->cols);

	pni->presult = (fftw_real *)calloc(pni->rows*pni->cols*sizeof(fftw_real), 1);

	int row, col, nq;
	CFeatureVector *pvu, *pvk;

	for( pvu = psU->pFirstFV, row = 0; (row < pni->rows) && pvu; row++, pvu = pvu->pnext )
	{
		if( bDebug && (nDebug >= 1) )
			printf("F-vector %d\n", row);
		for( pvk = psK->pFirstFV, col = 0; (col < pni->cols) && pvk; col++, pvk = pvk->pnext )
		{
				pni->presult[(row*pni->cols)+col] = this->VectorDistance(pvu, pvk);
				pni->presult[(row*pni->cols)+col] += this->VectorDistance(pvk, pvu);
				pni->presult[(row*pni->cols)+col] /= 2;

				if( isnan(pni->presult[(row*pni->cols)+col]) )
					pni->presult[(row*pni->cols)+col] = INFINITY;

		}
	}

	return pni;
}
//
//	cross compute distances for between all the dfts in one block and all the dfts in another.
//
sni_result *CSniEngine::ComputeDistances(sni_block *pbr, sni_block *pbc, fftw_real fMeanFactor)
{
	fftw_real *pdftr, *pdftc;
	BOOL bFr = FALSE, bFc = FALSE;
	sni_result *pni = NULL;
	fftw_complex *pcr = NULL, *pcc = NULL;

	int nSamples = 0;
	
	if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_POWER) )
	{
		// both power
		pdftr = pbr->pdata+1;	// skip DC component
		pdftc = pbc->pdata+1;

		if( pbr->block.nDftSamPerBlock != pbc->block.nDftSamPerBlock )
		{
			printf("dft sizes do not match\n");
			return NULL; 
		}

		nSamples = pbr->block.nDftSamPerBlock-1; 
	}
	else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_PHASE) )
	{	
		// both complex
		bFc = bFr = TRUE;

		if( pbr->block.nDftSamPerBlock != pbc->block.nDftSamPerBlock )
		{
			printf("dft sizes do not match\n");
			return NULL; 
		}

		nSamples = pbr->block.nDftSamPerBlock;

		pdftr = (fftw_real *)malloc(pbr->block.nDftSamPerBlock*sizeof(fftw_real));
		pdftc = (fftw_real *)malloc(pbr->block.nDftSamPerBlock*sizeof(fftw_real));

		// copy the real part (r)
		int ic;

		pcr = (fftw_complex *)pbr->pdata;

		for( ic=0; ic<nSamples; ic++ )
			pdftr[ic] = pcr[ic].re;

		pcc = (fftw_complex *)pbc->pdata;

		for( ic=0; ic<nSamples; ic++ )
			pdftc[ic] = pcc[ic].re;

	}
	else if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_PHASE) )
	{
		// pbr real, pbc complex
		bFc = TRUE;

		if( pbr->block.nDftSamPerBlock != pbc->block.nDftSamPerBlock+1 )
		{
			printf("dft sizes do not match\n");
			return NULL; 
		}

		nSamples = pbc->block.nDftSamPerBlock; // this should be one less that for POWER

		pdftr = pbr->pdata+1;
		pdftc = (fftw_real *)malloc(nSamples*sizeof(fftw_real));

		// copy the real part (r)
		int ic;

		pcc = (fftw_complex *)pbc->pdata;

		for( ic=0; ic<nSamples; ic++ )
			pdftc[ic] = pcc[ic].re;
	}
	else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_POWER) )
	{ 
		// pbr complex, pbc real
		bFr = TRUE;

		if( pbr->block.nDftSamPerBlock != pbc->block.nDftSamPerBlock-1 )
		{
			printf("dft sizes do not match\n");
			return NULL; 
		}

		nSamples = pbr->block.nDftSamPerBlock; // this should be one less that for POWER

		pdftr = (fftw_real *)malloc(nSamples*sizeof(fftw_real));
		pdftc = pbc->pdata+1;

		// copy the real part (r)
		int ic;

		pcr = (fftw_complex *)pbr->pdata;

		for( ic=0; ic<nSamples; ic++ )
			pdftr[ic] = pcr[ic].re;
	}
	else
	{
		printf("combination %d with %d not supported\n", pbr->block.iType, pbc->block.iType);
		return NULL;
	}

	pni = (sni_result *)calloc(sizeof(sni_result),1);

	pni->pbr = pbr;
	pni->pbc = pbc;
	pni->rows = pbr->block.nDfts*pbr->block.nQuantum;
	pni->cols = pbc->block.nDfts*pbc->block.nQuantum;

	if( bDebug && (nDebug >=0) )
		printf("rows = %d[%dx%d], cols = %d[%dx%d] rxc = %d\n", pni->rows, 
				pbr->block.nDfts, pbr->block.nQuantum, pni->cols,
				pbc->block.nDfts, pbc->block.nQuantum, pni->rows*pni->cols);

	pni->presult = (fftw_real *)calloc(pni->rows*pni->cols*sizeof(fftw_real), 1);

	int row, col, nq;

	for( row = 0; row < pni->rows; row++ )
	{
		//printf("row %d\n", row);
		for( col = 0; col < pbc->block.nDfts; col++ )
		{
			//printf("col %d\n", col);
			for( nq = 0; nq < pbc->block.nQuantum; nq++ )
			{
				//printf("%d,%d,%d pcc = %x, %d\n", row, col, nq, pcc, (row*pni->cols)+(col*pbc->block.nQuantum)+nq);

				pni->presult[(row*pni->cols)+(col*pbc->block.nQuantum)+nq] 
									= this->Distance(pdftc, pdftr, nSamples, fMeanFactor);

				if( isnan(pni->presult[(row*pni->cols)+(col*pbc->block.nQuantum)+nq]) )
					pni->presult[(row*pni->cols)+(col*pbc->block.nQuantum)+nq] = 0;

				if( (col*pbc->block.nQuantum+nq+1) >= (pni->cols) )
					break;

				if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_POWER) )
				{
					// both power
					pdftc += pbc->block.nDftSamPerBlock;
				}
				else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_PHASE) )
				{	
					// copy the real part (r)
					int ic;

					pcc += pbc->block.nDftSamPerBlock;

					for( ic=0; ic<nSamples; ic++ )
						pdftc[ic] = pcc[ic].re;

				}
				else if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_PHASE) )
				{
					// copy the real part (r)
					int ic;

					pcc += pbc->block.nDftSamPerBlock;

					for( ic=0; ic<nSamples; ic++ )
						pdftc[ic] = pcc[ic].re;
				}
				else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_POWER) )
				{ 
					// pbr complex, pbc real
					pdftc += pbc->block.nDftSamPerBlock;
				}
			}
		}
		// advance pdftr data.
		if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_POWER) )
		{
			// both power
			pdftr += pbr->block.nDftSamPerBlock;
			pdftc = pbc->pdata+1;
		}
		else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_PHASE) )
		{	
			// both complex
			// copy the real part (r)
			int ic;

			if( row < (pni->rows-1) )
			{
				pcr += pbr->block.nDftSamPerBlock;

				for( ic=0; ic<nSamples; ic++ )
					pdftr[ic] = pcr[ic].re;
			}

			pcc = (fftw_complex *)pbc->pdata;

			for( ic=0; ic<nSamples; ic++ )
				pdftc[ic] = pcc[ic].re;

		}
		else if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_PHASE) )
		{
			// pbr real, pbc complex
			pdftr += pbr->block.nDftSamPerBlock;

			// copy the real part (r)
			int ic;

			pcc = (fftw_complex *)pbc->pdata;

			for( ic=0; ic<nSamples; ic++ )
				pdftc[ic] = pcc[ic].re;
		}
		else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_POWER) )
		{ 
			// pbr complex, pbc real
			pdftc = pbc->pdata+1;

			// copy the real part (r)
			int ic;

			if( row < (pni->rows-1) )
			{
				pcr += pbr->block.nDftSamPerBlock;

				for( ic=0; ic<nSamples; ic++ )
					pdftr[ic] = pcr[ic].re;
			}
		}
	}

	if( bFr ) free(pdftr);
	if( bFc ) free(pdftc);

	return pni;
}

void CSniEngine::ComputeClusterCenters(hitpoint *centers, int *nc, fftw_real *dtab, int ndimension)
{
	int i, j;
	int nk = *nc;

	int closest[nk];
	fftw_real dtoc[nk];
	int nset[nk];
	int *kluster[nk];
	int ni = 0;	// new index into centers

	for( i=0; i<nk; i++ )
	{
		int k = centers[i].dftindex;
		int iclose = -1;
		fftw_real fd = 0.0;

		for( j=0; j<nk; j++ )
		{
			if( centers[i].dftindex == centers[j].dftindex )
				continue;

			if( bDebug && (nDebug >= 0) )
				printf("distance from %d to %d is %3.2f\n", k, centers[j].dftindex, 
								dtab[centers[i].dftindex*ndimension+centers[j].dftindex]);

			if( (dtab[centers[i].dftindex*ndimension+centers[j].dftindex] < fd) || (iclose < 0) )
			{
				if( dtab[centers[i].dftindex*ndimension+centers[j].dftindex] > 0 )
				{
					closest[i] = iclose = centers[j].dftindex;
					dtoc[i] = fd = dtab[centers[i].dftindex*ndimension+centers[j].dftindex];
				}
			}
		}

		if( bDebug && (nDebug >= 0) )
			printf("closest member to %d is %d (%3.2f)\n", k, closest[i], dtoc[i]);

		nset[i] = 0;
		kluster[i] = (int *)calloc(ndimension, sizeof(int));

		// now we find all the blocks closer then the closest in the set.
		for( j=0; j<ndimension; j++ )
		{
			if( (dtab[centers[i].dftindex*ndimension+j] < dtoc[i]) && (j != centers[i].dftindex) )
			{
				if( bDebug && (nDebug >= 0) )
					printf("+%d (%4.1f) ", j, dtab[centers[i].dftindex*ndimension+j]);

				kluster[i][nset[i]++] = j;
			}
		}
		kluster[i] = (int *)realloc(kluster[i], nset[i]*sizeof(int));

		if( bDebug && (nDebug >= 0) )
			printf("\ntotal in kluster[%d] = %d\n", i, nset[i]);

		if( nset[i] <= 5 )
		{
			// printf("cluster %d too small\n", i);
			continue;
		}

		int icenter = -1;
		fftw_real dcenter = 0;

		for( j=0; j<nset[i]; j++ )
		{
			fftw_real fcenter = 0;
			int r, c;

			for( k=0; k<nset[i]; k++ )
			{
				fcenter += dtab[(kluster[i][j]*ndimension)+kluster[i][k]] * dtab[(kluster[i][j]*ndimension)+kluster[i][k]];
			}

			if( (icenter < 0) || (fcenter < dcenter) )
			{
				dcenter = fcenter;
				icenter = kluster[i][j];
			}
		}
		//if( bDebug && (nDebug >= 0) )
			printf("center of gravity of kluster %d is %d (%4.1f, %d)\n", i, icenter, sqrt(dcenter)/nset[i], nset[i]);
		centers[ni].dftindex = icenter;
		centers[ni++].nkluster = nset[i];
	}

	*nc = ni;
}

sni_block *CSniEngine::ExtractLikeBlocks(sni_block *pbr, sni_block *pbc, fftw_real fThreshold)
{
	return NULL;
}

sni_block *CSniEngine::MergeBlocks(sni_block *pb1, sni_block *pb2)
{
	return NULL;
}
//
// compare two signature blocks and return the correlation.
//
sni_result *CSniEngine::CompareBlock(sni_block *pbr, sni_block *pbc, fftw_real fMeanFactor, BOOL bEverything)
{
	fftw_real *pdftr, *pdftc;
	BOOL bFr = FALSE, bFc = FALSE;
	sni_result *pni = NULL;
	fftw_complex *pcr = NULL, *pcc = NULL;

	int nSamples = 0;
	
	if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_POWER) )
	{
		// both power
		pdftr = pbr->pdata+1;	// skip DC component
		pdftc = pbc->pdata+1;

		if( pbr->block.nDftSamPerBlock != pbc->block.nDftSamPerBlock )
		{
			printf("dft sizes do not match\n");
			return NULL; 
		}

		nSamples = pbr->block.nDftSamPerBlock-1; 
	}
	else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_PHASE) )
	{	
		// both complex
		bFc = bFr = TRUE;

		if( pbr->block.nDftSamPerBlock != pbc->block.nDftSamPerBlock )
		{
			printf("dft sizes do not match\n");
			return NULL; 
		}

		nSamples = pbr->block.nDftSamPerBlock;

		pdftr = (fftw_real *)malloc(pbr->block.nDftSamPerBlock*sizeof(fftw_real));
		pdftc = (fftw_real *)malloc(pbr->block.nDftSamPerBlock*sizeof(fftw_real));

		// copy the real part (r)
		int ic;

		pcr = (fftw_complex *)pbr->pdata;

		for( ic=0; ic<nSamples; ic++ )
			pdftr[ic] = pcr[ic].re;

		pcc = (fftw_complex *)pbc->pdata;

		for( ic=0; ic<nSamples; ic++ )
			pdftc[ic] = pcc[ic].re;

	}
	else if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_PHASE) )
	{
		// pbr real, pbc complex
		bFc = TRUE;

		if( pbr->block.nDftSamPerBlock != pbc->block.nDftSamPerBlock+1 )
		{
			printf("dft sizes do not match\n");
			return NULL; 
		}

		nSamples = pbc->block.nDftSamPerBlock; // this should be one less that for POWER

		pdftr = pbr->pdata+1;
		pdftc = (fftw_real *)malloc(nSamples*sizeof(fftw_real));

		// copy the real part (r)
		int ic;

		pcc = (fftw_complex *)pbc->pdata;

		for( ic=0; ic<nSamples; ic++ )
			pdftc[ic] = pcc[ic].re;
	}
	else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_POWER) )
	{ 
		// pbr complex, pbc real
		bFr = TRUE;

		if( pbr->block.nDftSamPerBlock != pbc->block.nDftSamPerBlock-1 )
		{
			printf("dft sizes do not match\n");
			return NULL; 
		}

		nSamples = pbr->block.nDftSamPerBlock; // this should be one less that for POWER

		pdftr = (fftw_real *)malloc(nSamples*sizeof(fftw_real));
		pdftc = pbc->pdata+1;

		// copy the real part (r)
		int ic;

		pcr = (fftw_complex *)pbr->pdata;

		for( ic=0; ic<nSamples; ic++ )
			pdftr[ic] = pcr[ic].re;
	}
	else
	{
		printf("combination %d with %d not supported\n", pbr->block.iType, pbc->block.iType);
		return NULL;
	}

	pni = (sni_result *)calloc(sizeof(sni_result),1);

	pni->pbr = pbr;
	pni->pbc = pbc;
	pni->rows = pbr->block.nDfts*pbr->block.nQuantum;
	pni->cols = pbc->block.nDfts*pbc->block.nQuantum;

	if( bDebug && (nDebug >=0) )
		printf("rows = %d[%dx%d], cols = %d[%dx%d] rxc = %d\n", pni->rows, 
				pbr->block.nDfts, pbr->block.nQuantum, pni->cols,
				pbc->block.nDfts, pbc->block.nQuantum, pni->rows*pni->cols);

	pni->presult = (fftw_real *)calloc(pni->rows*pni->cols*sizeof(fftw_real), 1);

	int row, col, nq;

	for( row = 0; row < pni->rows; row++ )
	{
		//printf("row %d\n", row);
		for( col = 0; col < pbc->block.nDfts; col++ )
		{
			//printf("col %d\n", col);
			for( nq = 0; nq < pbc->block.nQuantum; nq++ )
			{
				//printf("%d,%d,%d pcc = %x, %d\n", row, col, nq, pcc, (row*pni->cols)+(col*pbc->block.nQuantum)+nq);

				pni->presult[(row*pni->cols)+(col*pbc->block.nQuantum)+nq] 
					= (this->Correlate(pdftc, pdftr, nSamples, fMeanFactor)
						+ this->Correlate(pdftr, pdftc, nSamples, fMeanFactor))/2;

				if( isnan(pni->presult[(row*pni->cols)+(col*pbc->block.nQuantum)+nq]) )
					pni->presult[(row*pni->cols)+(col*pbc->block.nQuantum)+nq] = 0;

				if( (col*pbc->block.nQuantum+nq+1) >= (pni->cols) )
					break;

				if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_POWER) )
				{
					// both power
					pdftc += pbc->block.nDftSamPerBlock;
				}
				else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_PHASE) )
				{	
					// copy the real part (r)
					int ic;

					pcc += pbc->block.nDftSamPerBlock;

					for( ic=0; ic<nSamples; ic++ )
						pdftc[ic] = pcc[ic].re;

				}
				else if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_PHASE) )
				{
					// copy the real part (r)
					int ic;

					pcc += pbc->block.nDftSamPerBlock;

					for( ic=0; ic<nSamples; ic++ )
						pdftc[ic] = pcc[ic].re;
				}
				else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_POWER) )
				{ 
					// pbr complex, pbc real
					pdftc += pbc->block.nDftSamPerBlock;
				}
			}
		}
		// advance pdftr data.
		if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_POWER) )
		{
			// both power
			pdftr += pbr->block.nDftSamPerBlock;
			pdftc = pbc->pdata+1;
		}
		else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_PHASE) )
		{	
			// both complex
			// copy the real part (r)
			int ic;

			if( row < (pni->rows-1) )
			{
				pcr += pbr->block.nDftSamPerBlock;

				for( ic=0; ic<nSamples; ic++ )
					pdftr[ic] = pcr[ic].re;
			}

			pcc = (fftw_complex *)pbc->pdata;

			for( ic=0; ic<nSamples; ic++ )
				pdftc[ic] = pcc[ic].re;

		}
		else if( (pbr->block.iType == SNIT_POWER) && (pbc->block.iType == SNIT_PHASE) )
		{
			// pbr real, pbc complex
			pdftr += pbr->block.nDftSamPerBlock;

			// copy the real part (r)
			int ic;

			pcc = (fftw_complex *)pbc->pdata;

			for( ic=0; ic<nSamples; ic++ )
				pdftc[ic] = pcc[ic].re;
		}
		else if( (pbr->block.iType == SNIT_PHASE) && (pbc->block.iType == SNIT_POWER) )
		{ 
			// pbr complex, pbc real
			pdftc = pbc->pdata+1;

			// copy the real part (r)
			int ic;

			if( row < (pni->rows-1) )
			{
				pcr += pbr->block.nDftSamPerBlock;

				for( ic=0; ic<nSamples; ic++ )
					pdftr[ic] = pcr[ic].re;
			}
		}
	}

	if( bFr ) free(pdftr);
	if( bFc ) free(pdftc);

	return pni;
}

fftw_real CSniEngine::Distance(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor)
{
	fftw_real fdistance = 0;

	for(int i = 0; i<nSamples; i++)
	{
		fdistance += pow(pAudio[i] - pSignature[i], 2); // sum{(a[i] - s[i])^2}
	}

	return sqrt(fdistance);
}

//
//	This is the heart of the matter, along with ComputeFeatureVector.
//
//	The feature vector has two parts - the formants and the pitches. Normally the pitches
//	are greater in number.
//	
//	Everything is in the sni_formant data type. fcenter and fpower. fcenter is in Hz (0<fcenter<4000)
//	fpower is in dB and ranges between about -3 and +6. .
//	We arbitrarily set the axis crossing at -3dB.
//
//	The operation of comparison is a matter of research. If the feature vector is useless (i.e. does not map well),
//	then no distance measure will help. Therefore the relationship between these two is crucial.
//
//	The good news is that with the feature vector method fixed, we can play with the total performance
//	of the system by changing only this routine.
//
//	The approach is as follows: 
//
//	use snigen to create an snifile (with feature vectors) for the higher volume parts of each message).
//	use snicluster to generate a definitive snc file.
//	use snicmp to compare snc files.
//
fftw_real CSniEngine::VectorDistance(CFeatureVector *pvU, CFeatureVector *pvK)
{
#ifndef OLDCODE
#ifndef MINDIS
#ifndef JUNK
	fftw_real fdistance = 0;

	// we score the formants in unknown against known.
	// but before we do this we aim to normalize to the mean.
	fftw_real fkmean = 0, fumean = 0;
	int iu, ik;

	for( ik=0; ik<pvK->nFormant; ik++ )
		fkmean += pvK->pFormant[ik].fpower;

	fkmean /= pvK->nFormant;

	for( iu=0; iu<pvU->nFormant; iu++ )
		fumean += pvU->pFormant[iu].fpower;

	fumean /= pvU->nFormant;

	// now we compute the distance.
	int nmatch = 0;
	fftw_real mband = 50/4000.0;
	fftw_real cband = 60;	// y = mband*n + cband

	for( ik=0; ik<pvK->nFormant; ik++ )
	{
		if( bDebug && (nDebug >= 1) )
			printf("f%4.0f ", pvK->pFormant[ik].fcenter);

		fftw_real fband = mband*pvK->pFormant[ik].fcenter + cband;
		fftw_real fmin = fabs(pvU->pFormant[0].fcenter-pvK->pFormant[ik].fcenter);
		int imin = 0;

		for( iu=1; iu<pvU->nFormant; iu++ )
		{
			if( fabs(pvU->pFormant[iu].fcenter-pvK->pFormant[ik].fcenter) < fmin )
			{
				fmin = fabs(pvU->pFormant[iu].fcenter-pvK->pFormant[ik].fcenter);
				imin = iu;
			}
		}

		if( (pvU->pFormant[imin].fcenter < pvK->pFormant[ik].fcenter+fband)
				&& (pvU->pFormant[imin].fcenter > pvK->pFormant[ik].fcenter-fband) )
		{
			fftw_real factor = (pvU->pFormant[imin].fpower+pvK->pFormant[ik].fpower-fumean-fkmean)/2;
			factor -= pvK->pFormant[ik].fpower-fkmean;
			if( bDebug && (nDebug >= 1) )
				printf("fc=%3.2f ", factor);
	
			nmatch++;
			fdistance += fabs(pvU->pFormant[imin].fcenter - pvK->pFormant[ik].fcenter)*exp(fabs(factor));

			if( bDebug && (nDebug >= 1) )
				printf("[%d, %3.0f, +%3.2f] ", imin, pvU->pFormant[imin].fcenter, 
								fabs(pvU->pFormant[imin].fcenter - pvK->pFormant[ik].fcenter));
		}
	}
	
	if( (nmatch < (pvK->nFormant-2)) || (nmatch == 0) )
	{
		if( bDebug && (nDebug >= 1) )
			printf("%4.2f %d/%d\n", 4000.0/(pvK->nFormant+1)/2, nmatch, pvK->nFormant); 
		return 4000.0/(pvK->nFormant+1)/2;
	}

	// the distance is complicated
	fdistance /= nmatch;

	if( bDebug && (nDebug >= 1) )
		printf("%4.2f %d/%d\n", fdistance, nmatch, pvK->nFormant); 
	return fdistance;
#else
	fftw_real fdistance = 0;

	int ik, iu;

	fftw_real *usamples = (fftw_real *)calloc(4000, sizeof(fftw_real));
	fftw_real *ksamples = (fftw_real *)calloc(4000, sizeof(fftw_real));

	// strategy is to construct pseudo graphs and then correlate them using the standard method.
	// everything is cutoff at -2dBgv.
	fftw_real lx = 0;
	fftw_real ly = 0;

	// we trust formant centers to be ins ascending order. This is a by product of their creation.
	//
	for( iu=0; iu<pvU->nFormant; iu++ )
	{
		fftw_real nx, ny;

		nx = pvU->pFormant[iu].fcenter;
		ny = exp(pvU->pFormant[iu].fpower);

		int ig;
		fftw_real m = (ny-ly)/(nx-lx);

		for( ig=(int)lx; ig < (int)nx; ig++ )
		{
			// draw the first part.
			usamples[ig] = m*ig + ly;
		}

		// now the second part. we look ahead.
		// we set lx and ly to their values at the intersection of the next formant slope.
		// assuming the slope meets the origin at nx.

		// solve for lx.
		fftw_real mthis, mnext;
		fftw_real xx, xy; // next formant

		if( iu >= (pvU->nFormant-1) )
		{
			// last one, special case.
			xx = 3999.0;
			xy = 0;
			m = (xy-ny)/(xx-nx);

			for( ig = (int)nx; ig <= xx; ig++ )
				usamples[ig] = m*ig + ny;
		}
		else
		{
			xx = pvU->pFormant[iu+1].fcenter;
			xy = exp(pvU->pFormant[iu+1].fpower);

			mthis = -ny/(xx-nx);
			mnext = xy/(xx-nx);

			// solve for xx when y's are equal
			lx = ny/(mnext-mthis) + nx;
			ly = mnext*(lx-nx);

			for( ig = (int)nx; ig<(int)lx; ig++ )
				usamples[ig] = mthis*(ig-nx) + ny;
		}
	}

	// now the known samples.
	lx = 0;
	ly = 0;

	for( ik=0; ik<pvK->nFormant; ik++ )
	{
		fftw_real nx, ny;

		nx = pvK->pFormant[ik].fcenter;
		ny = exp(pvK->pFormant[ik].fpower);

		int ig;
		fftw_real m = (ny-ly)/(nx-lx);

		for( ig=(int)lx; ig < (int)nx; ig++ )
		{
			// draw the first part.
			ksamples[ig] = m*ig + ly;
		}

		// now the second part. we look ahead.
		// we set lx and ly to their values at the intersection of the next formant slope.
		// assuming the slope meets the origin at nx.

		// solve for lx.
		fftw_real mthis, mnext;
		fftw_real xx, xy; // next formant

		if( ik >= (pvK->nFormant-1) )
		{
			// last one, special case.
			xx = 3999.0;
			xy = 0;
			m = (xy-ny)/(xx-nx);

			for( ig = (int)nx; ig <= xx; ig++ )
				ksamples[ig] = m*ig + ny;
		}
		else
		{
			xx = pvK->pFormant[ik+1].fcenter;
			xy = exp(pvK->pFormant[ik+1].fpower);

			mthis = -ny/(xx-nx);
			mnext = xy/(xx-nx);

			// solve for xx when y's are equal
			lx = ny/(mnext-mthis) + nx;
			ly = mnext*(lx-nx);

			for( ig = (int)nx; ig<(int)lx; ig++ )
				ksamples[ig] = mthis*(ig-nx) + ny;
		}
	}

	free(usamples); free(ksamples);

	fdistance = this->Correlate(usamples, ksamples, 4000, 2.0);

	return 1-fdistance;
#endif
#else
	// we look at all the formants in K and find the best match with U.
	// if we find that nK > nU, we reject the comparison. However we tolerate nU > nK, and
	// find the best match for the nK by the minimum distance method.

	fftw_real fdistance = 0;

	int ik, iu;
	CFeatureVector *pf1, *pf2;
	
	if( pvK->nFormant > pvU->nFormant )
	{
		pf1 = pvU;
		pf2 = pvK;
	}
	else
	{
		pf1 = pvK;
		pf2 = pvU;
	}

	fftw_real fminmin = 4000.00;

	for( ik=0; ik<pf1->nFormant; ik++ )
	{
		fftw_real fmin = 4000.0;

		for( iu=0; iu<pf2->nFormant; iu++ )
		{
			if( fabs(pf1->pFormant[ik].fcenter - pf2->pFormant[iu].fcenter) < fmin )
				fmin = fabs(pf1->pFormant[ik].fcenter - pf2->pFormant[iu].fcenter);

			if( fmin < fminmin )
				fminmin = fmin;
		}

		fdistance += fmin;
	}

	return (fdistance/pf1->nFormant);
#endif
#else
	fftw_real fdistance = 0;
	fftw_real fpdistance = 0;
	int nusef, nusep = 4;

	// compare only the formants

	// first effort, look for formant matches.
	// the largest formant and the number of formants must match.

	if( pvU->nFormant < pvK->nFormant )
		nusef = pvU->nFormant;
	else
		nusef = pvK->nFormant;

	if( (pvU->nPitch < nusep) || (pvK->nPitch < nusep) )
	{
		if( pvU->nPitch < pvK->nPitch )
			nusep = pvU->nPitch;
		else
			nusep = pvK->nPitch;
	}

	if( (nusef <= 0) || (nusep <= 0) )	// no common formants
		return -1;

	int i;

#define FORMANTS
#ifdef FORMANTS
	for(i=0; i<nusef; i++)
	{
		// introduce power factors
		fftw_real factor;
		fftw_real fup, fkp;

		fup = exp(pvU->pFormant[i].fpower);
		fkp = exp(pvK->pFormant[i].fpower);

		// the powers are logs (in dB)
		factor = (fkp<fup)?fup/fkp:fkp/fup;
		fdistance += fabs(pvK->pFormant[i].fcenter-pvU->pFormant[i].fcenter)*factor;
	}

	if( bDebug && (nDebug >= 1) )
		printf("d=%f\n", fdistance);

	return fdistance;
#else
	for(i=0; i<nusep; i++)
	{
		fpdistance += pow(pvK->pPitch[i].fcenter-pvU->pPitch[i].fcenter, 2);
	}

	if( bDebug && (nDebug >= 1) )
		printf("p=%f\n", sqrt(fpdistance/nusep));

	return sqrt(fpdistance/nusep);
#endif
#endif
}

//
// Correlate two blocks of DFT's
//
fftw_real CSniEngine::Correlate(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor)
{
	int i, included = 0;
	fftw_real fCCsum = 0.0;
	fftw_real fThisS, fThisA;
	fftw_real spread_audio[nSamples];
	fftw_real spread_sig[nSamples];

	// new code
	// widen the peaks
	spread_audio[0] = pAudio[0]; spread_audio[nSamples-1] = pAudio[nSamples-1];
	spread_sig[0] = pAudio[0]; spread_sig[nSamples-1] = pAudio[nSamples-1];

	for( i=1; i< nSamples-1; i++ )
	{
#ifdef SPREAD
		spread_audio[i] = (pAudio[i-1] + pAudio[i] + pAudio[i+1])/3;
		spread_sig[i] = (pSignature[i-1] + pSignature[i] + pSignature[i+1])/3;
#else
		spread_audio[i] = pAudio[i];
		spread_sig[i] = pSignature[i];
#endif
	}
		

	// Calculate the sum of the signature
	for( i=0; i<nSamples; i+=2 )
	{
		if( (spread_sig[i] < fMeanFactor) )
			continue;

		included++;
		fThisS = spread_sig[i];
		fThisA = sqrt(spread_sig[i] * spread_audio[i]);

		if( fThisS <= fThisA )
			fCCsum += fThisS/fThisA;
		else
			fCCsum += fThisA/fThisS;
	}

	fCCsum /= included;

	return( pow(fCCsum,4) );
}

//
// Correlate two blocks of DFT's
//
fftw_real CSniEngine::Correlate2(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor)
{
	int i, included = 0;
	fftw_real spread_audio[nSamples];
	fftw_real spread_sig[nSamples];

	// new code
	// widen the peaks
	spread_audio[0] = pAudio[0]; spread_audio[nSamples-1] = pAudio[nSamples-1];
	spread_sig[0] = pAudio[0]; spread_sig[nSamples-1] = pAudio[nSamples-1];

	for( i=1; i< nSamples-1; i++ )
	{
#ifdef SPREAD
		spread_audio[i] = (pAudio[i-1] + pAudio[i] + pAudio[i+1])/3;
		spread_sig[i] = (pSignature[i-1] + pSignature[i] + pSignature[i+1])/3;
#else
		spread_audio[i] = pAudio[i];
		spread_sig[i] = pSignature[i];
#endif
	}
		
	fftw_real fsum = 0.0, fdelta = 0.0;
	fftw_real fThisU, fThisK;

	// Calculate the sum of the signature
	for( i=0; i<nSamples; i++ )
	{
		fThisK = spread_sig[i];
		fThisU = spread_audio[i];

		fsum += fThisK;
		fdelta += fabs(fThisK-fThisU);
	}

	return( (fsum - fdelta)/fsum ); // thank god for normalization
}

BOOL	CSniEngine::MakeRealToComplexFDft(fftw_real *pAudio, fftw_real *pCDft)
{
	rfftw_one(this->theForwardPlan, pAudio, pCDft);

	return TRUE;
}

//
//	side effect: damages pCDft
//
BOOL	CSniEngine::MakeComplexToRealFDft(fftw_real *pCDft, fftw_real *pAudio)
{
	rfftw_one(this->theReversePlan, pCDft, pAudio);
	return TRUE;
}

//
// Create an SNI file from a CSniSig structure
//
//	bDft is by default true, in which case the Dfts are saved.
//	if bDft is false, then a single psuedo-sigblack is saved with nDft set to 0.
//
BOOL	CSniEngine::SaveSignature(CSniSig *pSig, char *filename, BOOL bDft)
{
	FILE *snifp;

	snifp = fopen(filename, "w");

	if( snifp == NULL )
	{
		printf("can't create %s\n", filename);
		return FALSE;
	}

	// Now create the file;
	int iFO = 0;	// file offset
	int nw;

	nw = fwrite(&pSig->header, 1, sizeof(sni_header), snifp);

	if( bDebug && (nDebug >= 0) )
		printf("writing header nw = %d (iFO=%d %d %d)\n", nw, nw + sizeof(struct _sni_block)*pSig->header.nDftBlocks,  
										sizeof(struct _sni_block), pSig->header.nDftBlocks);

	// now we step through the blocks, fixing up the file pointers.
	iFO = nw + sizeof(struct _sni_block)*pSig->header.nDftBlocks;

	sni_block *pb;
	int nb = 0;

	// write the sigblocks
	for( pb = pSig->pFirstBlock; pb; pb = pb->pnext )
	{
		struct _sni_block tb = pb->block;

		if( bDebug && (nDebug >= 0) )
			printf("writing block %d, iFO=%d\n", nb++, iFO);

		tb.iFileOffset = iFO;

		if( !bDft )
			tb.nDfts = 0;

		iFO += tb.nDfts * tb.nQuantum * tb.nDftSamPerBlock * tb.iSizeof;	// phew!

		nw += fwrite(&tb, 1, sizeof(struct _sni_block), snifp);
	}

	// write the dfts
	nb = 0;
	int ifo = 0;

	for( pb = pSig->pFirstBlock; bDft && pb; pb = pb->pnext )
	{
		if( bDebug && (nDebug >= 0) )
			printf("writing dft for block %d\n", nb++);

		fwrite(pb->pdata, pb->block.nDfts * pb->block.nQuantum * pb->block.nDftSamPerBlock, pb->block.iSizeof, snifp);
		ifo += pb->block.nDfts * pb->block.nQuantum * pb->block.nDftSamPerBlock * pb->block.iSizeof;
	}

	// now write the FeatureVectors
	if( pSig->header.nVectors > 0 )
	{
		CFeatureVector *pfv = pSig->pFirstFV;

		pSig->header.iVectorFileOffset = sizeof(sni_header)+pSig->header.nDftBlocks*sizeof(struct _sni_block) + ifo;

		for(; pfv; pfv = pfv->pnext )
		{
			pfv->SaveVector(snifp);
		}
	
		// rewrite the header
		fseek(snifp, 0, SEEK_SET);
		fwrite(&pSig->header, 1, sizeof(sni_header), snifp);
	}

	fclose(snifp);

	return TRUE;
}

//
//	score a feature vector against the pmatrix;
//
//	we look at each formant frequency and we compute the probability that it occurs with the
//	other formants;
//
//	note that the prob. that F1 is with F2 is different from the Prob. that F2 is with F1.
//
fftw_real CSniEngine::ScoreFeatureVector(int *pmatrix, CFeatureVector *pfv)
{
	if( pfv->nFormant <= 0 )
		return 0;

	int i, j;
	fftw_real finalscore = 0;

	for( i=0; i<pfv->nFormant; i++ )
	{
		fftw_real score = 0;

		int im = (int)(pfv->pFormant[i].fcenter/160);
		for( j=0; j<pfv->nFormant; j++ )
		{
			int iu = (int)(pfv->pFormant[j].fcenter/160);
			if( score == 0 )
				score = pmatrix[(im*26)+iu];
			else
				score *= pmatrix[(im*26)+iu];
		}

		// chose the best
		if( score > finalscore )
			finalscore = score;
	}

	return finalscore;
}
// load a .snp file. These files contain exactly 26 lines of ascii text.
//
//	p-matrix f <nfreq> <filename>
//	000 000 ... 25 times. | 000
//	...
//
//	the entries in the first 25 columns of each row is the transition probability (over the test data)
//	of a given formant being assocaiated with a another of a given frequency.
//
//	The last column is the probability (over the test data) of that formant being seen.
//
//	The p-matirx is a 25x26 array of ints.
//		column 26 is the probability of a given formant occuring.
//
int *CSniEngine::LoadPMatrix(char *filename)
{
	FILE *fp;
	char libuf[256];
	
	if( filename == NULL )
		return NULL;

	fp = fopen(filename, "r");

	if( fp == NULL )
	{
		printf("cant open and load %s\n", filename);
		return NULL;
	}

	int *pmatrix = (int *)calloc(25*26, sizeof(int));
	
	fgets(libuf, 255, fp);
	if( strncmp(libuf, "p-matrix", 8) != 0 )
	{
		printf("bad file header for %s -> %s", filename, libuf);
		return NULL;
	}

	int row, col;

	for( row=0; row<25; row++ )
	{
		fgets(libuf, 255, fp);

		char *cp = libuf;

		for( col=0; col<25; col++ )
		{
			pmatrix[(row*26)+col] = atoi(cp);
			cp += 4;
		}
		pmatrix[(row*26)+25] = atoi(cp+2);
	}

	return pmatrix;
}

CSniSig	*CSniEngine::LoadSignature(char *filename)
{
	FILE *snifp;
	CSniSig *ps = new CSniSig;
	fftw_real *pd;

	snifp = fopen(filename, "r");

	if( snifp == NULL )
	{
		printf("can't open %s\n", filename);
		return NULL;
	}

	// read the header
	if( bDebug && (nDebug >= 0) )
		printf("loading sni file header for '%s'\n", filename);

	int nr = fread(&ps->header, 1, sizeof(sni_header), snifp);

	if( strncmp(ps->header.szmagic, SIGNEWHDRMAGIC, 4) == 0 )
	{
		delete ps;
		ps = new CSniSig;

		// fixup an old signature
		printf("loading and translating old signature\n");

		// read the old header
		struct newsigheader *poh = (newsigheader *)calloc(sizeof(newsigheader), 1);

		fseek(snifp, 0, SEEK_SET);
		nr = fread(poh, 1, sizeof(newsigheader), snifp);

		printf("timeout:\t%d\t(discarded)\n", poh->nTimeout);
		printf("threshold:\t%-4.2f\t(discarded)\n", poh->fdCorrel);
		printf("hostname:\t'%s'\t(discarded)\n", poh->sHostname);
		printf("dtmftones:\t'%s'\t(discarded)\n", poh->sDTMFTones);

		// set up the header block
		strcpy(ps->header.szmagic, SNIHDRMAGIC);
		ps->header.tCreate = 0;
		strcpy(ps->header.sTag, poh->sTag);
		char *dp = strrchr( poh->sWaveFile, '.');

		if( dp )
			*dp = '\0';

		strcpy(ps->header.sMessageId, poh->sWaveFile);

		// create one DFT Block.
		ps->header.nDftBlocks = 1;
		sni_block *pb;

		pb = (sni_block *)calloc(sizeof(sni_block), 1);
		ps->pFirstBlock = ps->pLastBlock = pb;

		// set up the block header.
		pb->block.nDfts = 1;
		pb->block.iFileOffset = sizeof(sni_header) + sizeof(struct _sni_block);
		pb->block.iAudioOffset = poh->dAudioOffset*400;
		pb->block.nQuantum = 1;
		pb->block.nDftSamPerBlock = poh->nSamplesPerBlock;
		pb->block.nAudioSamPerBlock = (poh->nSamplesPerBlock-1)*2;
		pb->block.iType = SNIT_POWER;
		pb->block.iSizeof = sizeof(fftw_real);
		pb->block.iFlags = 0;
		pb->pdata = (fftw_real *)malloc(sizeof(fftw_real)*pb->block.nDftSamPerBlock);

		// Read the Dft
#ifdef FFTW_ENABLE_FLOAT
		double dDft[this->nDftSamPerBlock];

		nr = fread(dDft, sizeof(double), poh->nSamplesPerBlock, snifp);

		for( int id = 0; id < this->nDftSamPerBlock; id++ )
			pb->pdata[id] = (fftw_real)dDft[id];

#else
		nr = fread(pb->pdata, sizeof(fftw_real), poh->nSamplesPerBlock, snifp);
#endif

		// Close the Signature File.
		fclose(snifp);

		return ps;
	}

	assert( strncmp(ps->header.szmagic, SNIHDRMAGIC, 4) == 0 );

	if( nr <= 0 )
	{
		printf("file too short\n");
		return NULL;
	}

	int i;
	sni_block *pb;

	for( i=0; i<ps->header.nDftBlocks; i++ )
	{

		// step through the sigblocks, loading each one's header
		pb = (sni_block *)calloc(sizeof(sni_block), 1);

		nr = fread(&pb->block, sizeof(struct _sni_block), 1, snifp);

		if( bDebug && (nDebug >= 1) )
			printf("ndfts = %d, iFO = %d, iType = %d, iSizeof = %d\n",
				pb->block.nDfts, pb->block.iFileOffset, pb->block.iType, pb->block.iSizeof);

		if( ps->pFirstBlock == NULL )
		{
			ps->pFirstBlock = ps->pLastBlock = pb;
		}
		else
		{
			ps->pLastBlock->pnext = pb;
			pb->plast = ps->pLastBlock;
			ps->pLastBlock = pb;
		}

		if( bDebug && (nDebug >= 0) )
			printf("loading block header %d\n", i);

		// assert(sizeof(fftw_real) == pb->block.iSizeof);
	}

	// read the dfts
	int nb = 0;

	for( pb = ps->pFirstBlock; pb; pb = pb->pnext )
	{
		if( bDebug && (nDebug >= 0) )
			printf("reading dft for block %d\n", nb++);

		pb->pdata = (fftw_real *)malloc(pb->block.nDfts * pb->block.nQuantum * pb->block.nDftSamPerBlock * pb->block.iSizeof);

		fseek(snifp, pb->block.iFileOffset, SEEK_SET);
		nr = fread(pb->pdata, pb->block.iSizeof, pb->block.nDfts * pb->block.nQuantum * pb->block.nDftSamPerBlock, snifp);

		assert(nr == pb->block.nDfts * pb->block.nQuantum * pb->block.nDftSamPerBlock);
	}

	// read the feature vectors
	if( ps->header.nVectors > 0 )
	{
		fseek(snifp, ps->header.iVectorFileOffset, SEEK_SET);

		for( nb=0; nb<ps->header.nVectors; nb++ )
		{
			CFeatureVector *pfv;
			sni_vector_hdr vhdr;

			if( bDebug && (nDebug >= 0) )
				printf("reading feature vector for block %d\n", nb);

			int nr = fread(&vhdr, 1, sizeof(sni_vector_hdr), snifp);

			if( nr <= 0 ) break;

			pfv = new CFeatureVector(vhdr.nformant, vhdr.npitch);
			pfv->fEnergy = vhdr.fenergy;
			pfv->iSampleOffset = vhdr.isampleoffset;

			nr = fread(pfv->pFormant, vhdr.nformant, sizeof(sni_formant), snifp);
			if( nr <= 0 ) break;

			nr = fread(pfv->pPitch, vhdr.npitch, sizeof(sni_formant), snifp);
			if( nr <= 0 ) break;

			if( ps->pFirstFV == NULL )
			{
				ps->pFirstFV = ps->pLastFV = pfv;
			}
			else
			{
				ps->pLastFV->pnext = pfv;
				pfv->plast = ps->pLastFV;
				ps->pLastFV = pfv;
			}
		}
	}

	fclose(snifp);

	return ps;
}

void CSniEngine::PrintSignatureHeader(CSniSig *pSig, char *snifile)
{
	// now we print out the header details
	printf("filename:\t%s\n", snifile);
	printf("created:\t%s", ctime(&pSig->header.tCreate));
	printf("signature:\t'%s'\n", pSig->header.sTag);
	printf("routing:\t'%s'\n", pSig->header.sRouting);
	printf("messageid:\t'%s'\n", pSig->header.sMessageId);
	printf("callerid:\t'%s'\n", pSig->header.sCallerId);
	printf("author:\t\t'%s'\n", pSig->header.sAuthor);
	printf("dftblocks:\t%d\n", pSig->header.nDftBlocks);
	printf("nvectors:\t%d\n", pSig->header.nVectors);
	printf("vectorfileoffset:\t%d\n", pSig->header.iVectorFileOffset);

	int idft;

	sni_block *pb;

	for( idft=0, pb=pSig->pFirstBlock; pb; pb = pb->pnext, idft++ )
	{
		printf("dft %d:\tblocks=%dx%d", idft,  pb->block.nDfts, pb->block.nQuantum);
		printf(", aspb=%d (%.0f)", pb->block.nAudioSamPerBlock, (float)(pb->block.nAudioSamPerBlock)/8.0); 
		printf(", dspb=%d", pb->block.nDftSamPerBlock);
		switch( pb->block.iType )
		{
		case SNIT_POWER:
			printf(", type=power (%d)", pb->block.iSizeof);
			break;

		case SNIT_PHASE:
			printf(", type=phase (%d)", pb->block.iSizeof);
			break;

		case SNIT_SNG:
			printf(", type=sng (%d)", pb->block.iSizeof);
			break;

		default:
			printf(", type=%d (%d)", pb->block.iType, pb->block.iSizeof);
			break;
		}
		printf(", aoffset=%-6d (%4.2f)", pb->block.iAudioOffset, (float)(pb->block.iAudioOffset)/8000.00);
		printf(", foffset=%d\n", pb->block.iFileOffset);
	}
}
