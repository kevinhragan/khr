#include <stdio.h>
#include "sni.h"
#include "fftw.h"
#include "rfftw.h"
#include "engine.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <math.h>

static char szRcsId[] = "$Id: engine.cc,v 1.8 2007-03-06 01:51:28 martind Exp $";

CStaticEngine::CStaticEngine(double meanfactor, double threshold, int quantum, int aspb)
{
	int i;

	bInitialized = FALSE;

	nDftSamPerBlock = aspb/2+1;
	nCepSamPerBlock = aspb/4+1;		// Cepstrum
	nAudioSamPerBlock = aspb;	// 250ms
	nWindowFraction = quantum;	// aka 'quantum'
	fMeanFactor = meanfactor;
	fThreshold = threshold;

	pFirstSignature = NULL;
	pLastSignature = NULL;
	nSignature = 0;

	pLastAudio = NULL;

	// First We Initialize the FFT Engine.
	if( this->bInitialized == FALSE )
	{
		if( bDebug && (nDebug >= 1) )
			printf("creating FFT plan for %d samples\n", this->nAudioSamPerBlock);

		//	Create FFT Plan
		this->thePlan = rfftw_create_plan(this->nAudioSamPerBlock, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
		this->theForwardPlan = rfftw_create_plan(this->nAudioSamPerBlock/10, FFTW_FORWARD, FFTW_ESTIMATE);
		this->theReversePlan = rfftw_create_plan(this->nAudioSamPerBlock/10, FFTW_BACKWARD, FFTW_ESTIMATE);
		this->theCPlan = rfftw_create_plan(this->nDftSamPerBlock-1, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
		this->theReverseCPlan = rfftw_create_plan(this->nDftSamPerBlock-1, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);

		// Allocate FFT Buffers
		// pIn is twice the size to allow windowing.
		this->pIn = (fftw_real *)malloc(sizeof(fftw_real)*this->nAudioSamPerBlock*2);
		this->pOut = (fftw_real *)malloc(sizeof(fftw_real)*this->nAudioSamPerBlock);
		this->pPwr = (fftw_real *)malloc(sizeof(fftw_real)*((this->nAudioSamPerBlock/2)+1));

		this->bInitialized = TRUE;


	}
}

CStaticEngine::~CStaticEngine()
{
	free(this->pIn);
	free(this->pOut);
	free(this->pPwr);

}

extern BOOL bNumeric, bInDigit, bOldstyle;
extern char *etoken, *btoken;
extern BOOL bb, be;

char lasttag[40] = "";
int nseen = 0;

//
//	We rotate the buffers between pFirstAudio and pLastAudio and construct this->WindowFraction windows
//	from this double buffer.
//
BOOL	CStaticEngine::SignatureReport(short *audiobuffer, char *result)
{
	CSignature *pThisSig;
	int ic;
	// Fake out the old parameters.
	int	j, k;

	if( pLastAudio == NULL )
	{
		// first call
		pLastAudio = audiobuffer;

		// copy in the first buffer.
		for( j = nAudioSamPerBlock; j < nAudioSamPerBlock*2; j++)
		{
			pIn[j] = (fftw_real)pLastAudio[j-nAudioSamPerBlock];
		}

		return TRUE;
	}
	
	pLastAudio = audiobuffer;

	// Copy the Raw Audio into the FFT Buffer
	for( j=0; j < nAudioSamPerBlock; j++)
	{
		pIn[j] = pIn[j+nAudioSamPerBlock];
	}

	for( j = nAudioSamPerBlock; j < nAudioSamPerBlock*2; j++)
	{
		pIn[j] = (fftw_real)pLastAudio[j-nAudioSamPerBlock];
	}

	// Now we correlate the newly FFT'd block with the each of the Dft blocks for the Events for this state!

	int iWindow;
	int iOffset;
	int np = 0;

	fftw_real	LazyPwr[this->nWindowFraction][sizeof(fftw_real)*((this->nAudioSamPerBlock/2)+1)];
	DftStats 	LazyStats[this->nWindowFraction];

	for( iWindow = 0, iOffset = 0; iOffset < nAudioSamPerBlock; 
						iOffset += nAudioSamPerBlock/this->nWindowFraction, iWindow++ )
	{
		assert( iWindow < this->nWindowFraction );

		this->MakeFDft(pIn+iOffset, &LazyPwr[iWindow][0], &LazyStats[iWindow].fTruePower);
	}

	BOOL bMatched = FALSE;
	char thisdigit[10];

	// Do the difficult work of looking for triggered expect clauses
	//
	for( iWindow = 0; iWindow < this->nWindowFraction; iWindow++ )
	{
		fftw_real fThisCorrel;

		for( pThisSig = this->pFirstSignature; pThisSig; pThisSig = pThisSig->pNext )
		{
			// | corresponds to 'from' in most cases, it demarcates the cid.
			//
			// Just look for best match with given block
			fThisCorrel = Correlate(&LazyPwr[iWindow][0], pThisSig->pDft, nDftSamPerBlock, fMeanFactor);

			if( fThisCorrel >= fThreshold )
			{
				bMatched = TRUE;
				// once we've matched we fill the rest of the buffer.

				if( btoken && (bb == FALSE) )
				{
					char *sp, *ep;

					sp = btoken;

					while( (ep = strchr(sp, ',')) )	// comma separated choices
					{
						if( strncmp(sp, pThisSig->SigHeader.sTag, ep - sp) == 0 )
						{
							bb = TRUE;
							break;
						}

						sp = ep+1;
					}

					if( ep == NULL )
					{
						if( strncmp(sp, pThisSig->SigHeader.sTag, strlen(sp)) == 0 )
							bb = TRUE;
					}
				}

				if( bb  && !be )
				{
					// print message
					if( bNumeric )
					{
						sprintf(thisdigit, "%c", pThisSig->SigHeader.sTag[0]);
						if(result)
							strcat(result, thisdigit);
					}
					else
						printf("%s %4.2f\n", pThisSig->SigHeader.sTag, fThisCorrel);
				}

				if( etoken && (be == FALSE) )
				{
					char *sp, *ep;

					sp = etoken;

					while( (ep = strchr(sp, ',')) )	// comma separated choices
					{
						if( strncmp(sp, pThisSig->SigHeader.sTag, ep - sp) == 0 )
						{
							be = TRUE;
							break;
						}

						sp = ep+1;
					}

					if( ep == NULL )
					{
						if( strncmp(sp, pThisSig->SigHeader.sTag, strlen(sp)) == 0 )
							be = TRUE;
					}
				}

			}
		}

		if( result && !bOldstyle && !bMatched  && ( bb  && !be ) )
			strcat(result, ".");

	}

	return(TRUE);
}


//
//	We rotate the buffers between pFirstAudio and pLastAudio and construct this->WindowFraction windows
//	from this double buffer.
//

BOOL	CStaticEngine::ScanReport(short *audiobuffer, char *result, int xl)
{
	CSignature *pThisSig;
	int ic;
	// Fake out the old parameters.
	int	j, k;

	if( pLastAudio == NULL )
	{
		// first call
		pLastAudio = audiobuffer;

		// copy in the first buffer.
		for( j = nAudioSamPerBlock; j < nAudioSamPerBlock*2; j++)
		{
			pIn[j] = (fftw_real)pLastAudio[j-nAudioSamPerBlock];
		}

		return TRUE;
	}
	
	pLastAudio = audiobuffer;

	// Copy the Raw Audio into the FFT Buffer
	for( j=0; j < nAudioSamPerBlock; j++)
	{
		pIn[j] = pIn[j+nAudioSamPerBlock];
	}

	for( j = nAudioSamPerBlock; j < nAudioSamPerBlock*2; j++)
	{
		pIn[j] = (fftw_real)pLastAudio[j-nAudioSamPerBlock];
	}

	// Now we correlate the newly FFT'd block with the each of the Dft blocks for the Events for this state!

	int iWindow;
	int iOffset;
	int np = 0;

	fftw_real	LazyPwr[this->nWindowFraction][sizeof(fftw_real)*((this->nAudioSamPerBlock/2)+1)];
	DftStats 	LazyStats[this->nWindowFraction];

	for( iWindow = 0, iOffset = 0; iOffset < nAudioSamPerBlock; 
						iOffset += nAudioSamPerBlock/this->nWindowFraction, iWindow++ )
	{
		assert( iWindow < this->nWindowFraction );

		this->MakeFDft(pIn+iOffset, &LazyPwr[iWindow][0], &LazyStats[iWindow].fTruePower);
	}

	BOOL bMatched = FALSE;

	// Do the difficult work of looking for triggered expect clauses
	//
	for( iWindow = 0; iWindow < this->nWindowFraction; iWindow++ )
	{
		fftw_real fThisCorrel;

		for( pThisSig = this->pFirstSignature; pThisSig; pThisSig = pThisSig->pNext )
		{
			// Just look for best match with given block
			fThisCorrel = Correlate(&LazyPwr[iWindow][0], pThisSig->pDft, nDftSamPerBlock, fMeanFactor);

			if( fThisCorrel >= fThreshold )
			{
				bMatched = TRUE;
				// once we've matched we fill the rest of the buffer.
				if( bDebug && (nDebug >= 1) )
					printf("comparing %s with %s bb = %s, be = %s\n", 
						btoken, pThisSig->SigHeader.sTag, bb?"true":"false", be?"true":"false" );

				if( (strlen(lasttag) > 0) && (strncmp(lasttag, pThisSig->SigHeader.sTag, strlen(lasttag)) == 0) )
				{
					// same tag
					nseen++;
				}
				else
				{
					// different tag
					if( strlen(lasttag) > 0 )
					{
						if( btoken && (bb == FALSE) )
						{
							char *sp, *ep;

							sp = btoken;

							while( (ep = strchr(sp, ',')) )	// comma separated choices
							{
								if( strncmp(sp, lasttag, ep - sp) == 0 )
								{
									bb = TRUE;
									break;
								}

								sp = ep+1;
							}

							if( ep == NULL )
							{
								if( strncmp(sp, lasttag, strlen(sp)) == 0 )
									bb = TRUE;
							}
						}

						if( bb  && !be )
						{
							if( xl == 0 )
								sprintf(result + strlen(result), "%s ", lasttag);
							else
								sprintf(result + strlen(result), "%s(%d) ", lasttag, nseen);
						}

						if( etoken && (be == FALSE) )
						{
							char *sp, *ep;

							sp = etoken;

							while( (ep = strchr(sp, ',')) )	// comma separated choices
							{
								if( strncmp(sp, lasttag, ep - sp) == 0 )
								{
									be = TRUE;
									break;
								}

								sp = ep+1;
							}

							if( ep == NULL )
							{
								if( strncmp(sp, lasttag, strlen(sp)) == 0 )
									be = TRUE;
							}
						}

					}
					strcpy(lasttag, pThisSig->SigHeader.sTag);

					char *stp;

					if( stp = strchr(lasttag, '_') ) 
						*stp = '\0';

					nseen = 0;
				}
			}
		}

	}

	return(TRUE);
}

BOOL	CStaticEngine::MatchesSignature(short *audiobuffer, char *siglist)
{
	if( siglist == NULL )
		return FALSE;

	assert( siglist && (strlen(siglist) > 0) );
	assert(audiobuffer);

	CSignature *pThisSig;
	// Fake out the old parameters.
	int	i, j;

	if( pLastAudio == NULL )
	{
		// first call
		pLastAudio = audiobuffer;

		// copy in the first buffer.
		for( j = nAudioSamPerBlock; j < nAudioSamPerBlock*2; j++)
		{
			pIn[j] = (fftw_real)pLastAudio[j-nAudioSamPerBlock];
		}

		return FALSE;
	}
	
	pLastAudio = audiobuffer;

	// Copy the Raw Audio into the FFT Buffer
	for( j=0; j < nAudioSamPerBlock; j++)
	{
		pIn[j] = pIn[j+nAudioSamPerBlock];
	}

	for( j = nAudioSamPerBlock; j < nAudioSamPerBlock*2; j++)
	{
		pIn[j] = (fftw_real)pLastAudio[j-nAudioSamPerBlock];
	}

	// Now we correlate the newly FFT'd block with the each of the Dft blocks for the Events for this state!

	int iWindow;
	int iOffset;

	fftw_real	LazyPwr[this->nWindowFraction][sizeof(fftw_real)*((this->nAudioSamPerBlock/2)+1)];

	for( iWindow = 0, iOffset = 0; iOffset < nAudioSamPerBlock; iOffset += nAudioSamPerBlock/this->nWindowFraction, iWindow++ )
	{
		assert( iWindow < this->nWindowFraction );

		this->MakeFDft(pIn+iOffset, &LazyPwr[iWindow][0]);
	}

	BOOL bMatched = FALSE;

	// setup the siglist as an array.
	char **psiglist = (char **)malloc(sizeof(char *));
	int npsig = 0;

	char *cp, *slp;

	for( slp=siglist, cp=strchr(siglist, ','); cp; cp = strchr(slp, ',') )
	{
		if( npsig > 0 )
			psiglist = (char **)realloc(psiglist, (npsig+1) * sizeof(char *));

		psiglist[npsig++] = strndup(slp, cp-slp);
		slp = cp + 1;
	}
	
	psiglist[npsig++] = strdup(slp);

	if( bDebug && (nDebug >= 1) )
		for( i=0; i<npsig; i++ )
			printf("%d\t%s\n", i, psiglist[i]);

	// Do the difficult work of looking for triggered expect clauses
	//
	for( iWindow = 0; iWindow < this->nWindowFraction; iWindow++ )
	{
		fftw_real fThisCorrel;

		for( pThisSig = this->pFirstSignature; pThisSig; pThisSig = pThisSig->pNext )
		{
			// we check to see that the Signature is in siglist
			char *up = strchr(pThisSig->SigHeader.sTag, '_');
			BOOL bMatched = FALSE;

			for( i=0; i<npsig; i++ )
			{
				if( strncmp(psiglist[i], pThisSig->SigHeader.sTag, up-pThisSig->SigHeader.sTag) == 0 )
				{
					bMatched = TRUE;
					break;
				}
			}

			if( !bMatched )
				continue;

			//
			// Just look for best match with given block
			fThisCorrel = Correlate(&LazyPwr[iWindow][0], pThisSig->pDft, nDftSamPerBlock, fMeanFactor);

			if( fThisCorrel >= fThreshold )
			{
				printf("matched signature %s\n", pThisSig->SigHeader.sTag);

				// once we've matched we fill the rest of the buffer.
				for( j=0; j<npsig; j++ )
					free( psiglist[j] );

				free( psiglist );

				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL	CStaticEngine::CompareAudio(short *abuffer, short *sigbuffer)
{
#ifdef LATER
	CSignature *pThisSig;
	int ic;
	// Fake out the old parameters.
	int	j, k;

	if( pLastAudio == NULL )
	{
		// first call
		pLastAudio = abuffer;

		// copy in the first buffer.
		for( j = nAudioSamPerBlock; j < nAudioSamPerBlock*2; j++)
		{
			pIn[j] = (fftw_real)pLastAudio[j-nAudioSamPerBlock];
		}

		return TRUE;
	}
	
	pLastAudio = abuffer;

	// Copy the Raw Audio into the FFT Buffer
	for( j=0; j < nAudioSamPerBlock; j++)
	{
		pIn[j] = pIn[j+nAudioSamPerBlock];
	}

	for( j = nAudioSamPerBlock; j < nAudioSamPerBlock*2; j++)
	{
		pIn[j] = (fftw_real)pLastAudio[j-nAudioSamPerBlock];
	}

	// Now we correlate the newly FFT'd block with the each of the Dft blocks for the Events for this state!

	int iWindow;
	int iOffset;
	int np = 0;

	fftw_real	LazyPwr[this->nWindowFraction][sizeof(fftw_real)*((this->nAudioSamPerBlock/2)+1)];
	DftStats 	LazyStats[this->nWindowFraction];

	for( iWindow = 0, iOffset = 0; iOffset < nAudioSamPerBlock; 
						iOffset += nAudioSamPerBlock/this->nWindowFraction, iWindow++ )
	{
		assert( iWindow < this->nWindowFraction );

		this->MakeFDft(pIn+iOffset, &LazyPwr[iWindow][0], &LazyStats[iWindow].fTruePower);
	}

	BOOL bMatched = FALSE;
	char thisdigit[10];

	// Do the difficult work of looking for triggered expect clauses
	//
#ifdef OLDCODE
	for( pThisSig = this->pFirstSignature; pThisSig; pThisSig = pThisSig->pNext )
	{
		fftw_real fThisCorrel;

		// wronng, only if -n
		if( bNumeric && !isdigit(pThisSig->SigHeader.sTag[0]) )
			continue;

		for( iWindow = 0; iWindow < this->nWindowFraction; iWindow++ )
		{
			// Just look for best match with given block
			fThisCorrel = Correlate(&LazyPwr[iWindow][0], pThisSig->pDft, nDftSamPerBlock, fMeanFactor);

			if( bDebug && (nDebug >= 0) )
			{
			}
			if( fThisCorrel >= fThreshold )
			{
				// print message
				if( bNumeric )
				{
					sprintf(thisdigit, "%c", pThisSig->SigHeader.sTag[0]);
					if(result)
						strcat(result, thisdigit);
				}
				else
					printf("%s %4.2f\n", pThisSig->SigHeader.sTag, fThisCorrel);
				bMatched = TRUE;
			}
#ifdef NEVER
			else
			{
				if(result && bNumeric)
					strcat(result, ".");
			}
#endif
		}

	}
#else
	for( iWindow = 0; iWindow < this->nWindowFraction; iWindow++ )
	{
		fftw_real fThisCorrel;

		for( pThisSig = this->pFirstSignature; pThisSig; pThisSig = pThisSig->pNext )
		{
			if( bNumeric && !isdigit(pThisSig->SigHeader.sTag[0]) )
				continue;

			// Just look for best match with given block
			fThisCorrel = Correlate(&LazyPwr[iWindow][0], pThisSig->pDft, nDftSamPerBlock, fMeanFactor);

			if( fThisCorrel >= fThreshold )
			{
				bMatched = TRUE;

				// print message
				if( bNumeric )
				{
					sprintf(thisdigit, "%c", pThisSig->SigHeader.sTag[0]);
					if(result)
						strcat(result, thisdigit);
					if( !bOldstyle )
						break; // only first match counts per block
				}
				else
					printf("%s %4.2f\n", pThisSig->SigHeader.sTag, fThisCorrel);
			}
		}

		if(result && bNumeric && !bOldstyle && !bMatched )
			strcat(result, ".");

	}
#endif

#ifdef NEVER
	if( bNumeric )
	{
		if( bMatched && (bInDigit == FALSE) )
		{
			bInDigit = TRUE;
			//printf("%c", cDigit);
			//fflush(stdout);
		}
		else if( !bMatched )
		{
			bInDigit = FALSE;
			if( result )
			{
				strcat(result, ".");
			}
		}
	}
#endif
#endif

	return(TRUE);
}

static int PulseWidth[DFTSAMPERBLOCK];
static fftw_real PulsePower[DFTSAMPERBLOCK];
static fftw_real PulseLastPower[DFTSAMPERBLOCK];
static fftw_real PulsedPowerdT[DFTSAMPERBLOCK];
static int nLastPeaks;

BOOL CStaticEngine::GetDftStats(fftw_real *pDft, DftStats *pStats, fftw_real fMeanFactor)
{
	int i;
	BOOL bPositive = FALSE;
	int _nPeaks = 0;
	fftw_real fLast = 0, fDiff = 0;
	fftw_real smoothdata[this->nDftSamPerBlock];

	pStats->fMax = 0;
	pStats->fSum = 0;
	pStats->nPeaks = 0;
	pStats->nTroughs = 0;
	pStats->fMaxPower = 0;
	pStats->fMeanPower = 0;
	pStats->nTriggers = 0;

	for( i = 1; i < this->nDftSamPerBlock; i++ )
	{
		if( i == 1 )
			smoothdata[i] = (pDft[i] + pDft[i+1])/2;
		else if( i == (this->nDftSamPerBlock-1) )
			smoothdata[i] = (pDft[i] + pDft[i-1])/2;
		else
			smoothdata[i] = (pDft[i-1] + pDft[i] + pDft[i+1])/3;
	}

	if( smoothdata[2] > smoothdata[1] )
		bPositive = TRUE;

	// maxima
	for( i = 1; i < this->nDftSamPerBlock; i++ )
	{
		// Calculate the running Mean
		pStats->fSum += pDft[i];
		if( pDft[i] > pStats->fMax )
			pStats->fMax = pDft[i];	// the maximum power in a peak.

		// Calculate nPeaks
		if( pDft[i] > fMeanFactor )
		{
			fDiff = smoothdata[i] - fLast;
			fLast = smoothdata[i];
			pStats->fMeanPower += pDft[i];

			if( pDft[i] > pStats->fMaxPower )
				pStats->fMaxPower = pDft[i]; // maxium power in a peak.

			if( (fDiff < 0) && (bPositive) )
			{
				bPositive = !bPositive;

				// we only store MAXPEAKS peak frequencies.
				if( pStats->nPeaks < MAXPEAKS )
				{
					pStats->iPeaks[pStats->nPeaks] = 4 * i; // Hz!
					pStats->fPower[pStats->nPeaks++] = pDft[i]; // Hz!
				}
				else
					pStats->nPeaks++;
			}
		}

	}

	int iLast = 0;

	// minima
	for( i = 1; i < this->nDftSamPerBlock; i++ )
	{
		// Calculate nTroughs
		if( pDft[i] <= fMeanFactor )
		{
			fDiff = smoothdata[i] - fLast;
			fLast = smoothdata[i];

			if( (fDiff > 0) && (!bPositive) && ( (i-iLast) == 1) )
			{
				bPositive = !bPositive;

				// we only store MAXPEAKS peak frequencies.
				// a minimum
				if( pStats->nTroughs < MAXPEAKS )
				{
					pStats->iTroughs[pStats->nTroughs] = 4 * i; // Hz!
					pStats->fTroughPower[pStats->nTroughs++] = pDft[i]; // Hz!
				}
				else
					pStats->nTroughs++;
			}
			iLast = i;
		}

	}

	pStats->fTotalPower = pStats->fMeanPower; // total power in peaks.

	pStats->fMeanPower /= pStats->nPeaks;	// mean power per peak

	pStats->fMean = pStats->fSum/this->nDftSamPerBlock;	
	// assert( pStats.fMean == 1);

	pStats->fMedian = pStats->fMax/2;

	nLastPeaks = pStats->nPeaks;

#ifdef LATER
	BOOL bDone = FALSE;
	BOOL bTrigger = FALSE;
	BOOL bFound = FALSE;

#define LOWCUTOFF 320/4
#define HICUTOFF 900/4

	// Now we go through the PulseWidth Table, updating it.
	for( i = LOWCUTOFF; i < HICUTOFF; i++ )
	{
		int j;
		bFound = FALSE;

		for( j=0; j<pStats->nPeaks; j++ )
		{
			// we check to see if there is a matching peak
			if( pStats->iPeaks[j] == (i * 4) )
			{
				PulseWidth[i]++;
				PulsePower[i] += pStats->fPower[j];
				PulsedPowerdT[i] = pStats->fPower[j] - PulseLastPower[i];
				PulseLastPower[i] = pStats->fPower[j];
				bFound = TRUE;
			}
		}

		// Any peaks that dont match get destroyed
		if( (bFound == FALSE) && (PulseWidth[i] > 0) )
		{
			assert( pStats->nTriggers < MAXTRIGGERS );

			if( ( PulseWidth[i] >= ntonewidth ) 
				&& ((4*i) != 180) && ((4*i) != 240)
				&& ((4*i) != 300) && ((4*i) != 480)
				// dtmf spec. frequencies
				&& ((4*i) != 697) && ((4*i) != 770)
				&& ((4*i) != 852) && ((4*i) != 941)
				&& (pStats->fTruePower > 3200000.0)	// arbitrary, depends upon modem gain.
				&& (nLastPeaks <= TONETHRESHOLD)
				&& (pStats->nPeaks <= TONETHRESHOLD)
				&& (pStats->nPeaks >= 3) )
			{
				pStats->iTriggers[pStats->nTriggers++] = i * 4;
				bTrigger = TRUE;

				if( bDebug && (nDebug >= 1) )
				{
					bDone = TRUE;
					printf("%dHz (%d, %4.2f,%8.0f, %d)*\n", i*4, PulseWidth[i], 
									PulsePower[i], pStats->fTruePower, pStats->nPeaks);
					gsLogMessage(logbuf, "ENGN");
				}
			}
			else if( bDebug && (nDebug >= 1) )
			{
				bDone = TRUE;
				printf("%dHz (%d, %4.2f,%8.0f, %d)\n", i*4, PulseWidth[i], 
								PulsePower[i], pStats->fTruePower, pStats->nPeaks);
				gsLogMessage(logbuf, "ENGN");
			}
			
			PulseWidth[i] = 0;
			PulsePower[i] = 0;
			PulseLastPower[i] = 0;
			PulsedPowerdT[i] = 0;
		}
	}

	if( bDone )
	{
		bDone = FALSE;
		if( bDebug && (nDebug >= 1) )
			gsLogMessage("---", "ENGN");
	}

	nLastPeaks = pStats->nPeaks;

	return bTrigger;
#else
	nLastPeaks = pStats->nPeaks;
	return TRUE;
#endif
}

BOOL	CStaticEngine::MakeCepstrum(fftw_real *pDft, fftw_real *pCepstrum)
{
	fftw_real	fSum, fMean;
	int j, k;

	// Copy the Raw Audio into the FFT Buffer
	//	pIn and pOut are static buffers hereabouts.
	//
	// we take the logarithm
	//
	for( j=0; j < this->nDftSamPerBlock; j++)
	{
		if( j == 0 )
			pIn[j] = 0.0;
		else
			pIn[j] = 10*log((fftw_real)pDft[j]);
	}

	// Perform the FFT
	rfftw_one(this->theCPlan, pIn, pOut);

	// Now set up the power spectrum
	pCepstrum[0] = sqrt(pOut[0] * pOut[0]);	// DC component

	for( k = 1; k < nCepSamPerBlock; k++ )
		pCepstrum[k] = sqrt(pOut[k] * pOut[k] + pOut[nCepSamPerBlock-k] * pOut[nCepSamPerBlock-k]);

	// Normalize to the Mean
	fSum = 0.0;
	fMean = 0.0;

	for( k = 0; k < nCepSamPerBlock; k++)
		fSum += pCepstrum[k];

	fMean = fSum/k;

	for( k = 0; k < nCepSamPerBlock; k++)
		pCepstrum[k] = pCepstrum[k]/fMean;

	return(TRUE);
}

BOOL	CStaticEngine::MakeFDft(fftw_real *pAudio, fftw_real *pDft, fftw_real *pfTotalPower)
{
	fftw_real	fSum, fMean;
	int j, k;

	// Perform the FFT
	rfftw_one(this->thePlan, pAudio, pOut);

	// Now set up the power spectrum
	pDft[0] = sqrt(pOut[0] * pOut[0]);	// DC component

	for( k = 1; k < (nAudioSamPerBlock+1)/2; k++ )
		pDft[k] = sqrt(pOut[k] * pOut[k] + pOut[nAudioSamPerBlock-k] * pOut[nAudioSamPerBlock-k]);

	if( nAudioSamPerBlock%2 == 0 )	// N is even
		pDft[nAudioSamPerBlock/2] = sqrt(pOut[nAudioSamPerBlock/2] * pOut[nAudioSamPerBlock/2]);

	// Normalize to the Mean
	fSum = 0.0;
	fMean = 0.0;

	for( k = 0; k < nAudioSamPerBlock/2 + 1; k++)
		fSum += pDft[k];

	fMean = fSum/k;

	for( k = 0; k < nAudioSamPerBlock/2 + 1; k++)
		pDft[k] = pDft[k]/fMean;


	if( pfTotalPower )
		*pfTotalPower = fSum;

	return(TRUE);
}

BOOL	CStaticEngine::MakeDft(short *pAudio, fftw_real *pDft, fftw_real *pfTotalPower)
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
		pDft[k] = sqrt(pOut[k] * pOut[k] + pOut[nAudioSamPerBlock-k] * pOut[nAudioSamPerBlock-k]);

	if( nAudioSamPerBlock%2 == 0 )	// N is even
		pDft[nAudioSamPerBlock/2] = sqrt(pOut[nAudioSamPerBlock/2] * pOut[nAudioSamPerBlock/2]);

	// Normalize to the Mean
	fSum = 0.0;
	fMean = 0.0;

	for( k = 0; k < nAudioSamPerBlock/2 + 1; k++)
		fSum += pDft[k];

	fMean = fSum/k;

	for( k = 0; k < nAudioSamPerBlock/2 + 1; k++)
		pDft[k] = pDft[k]/fMean;


	if( pfTotalPower )
		*pfTotalPower = sqrt(*pfTotalPower/nAudioSamPerBlock);

	return(TRUE);
}

//	LoadSignatures - supercedes LoadWordTable: 
//
//	Loads the Signatures referenced by the given host "hostname"
//	into memory. The newly loaded signatures are attached to this->pSignatures.
//
BOOL CStaticEngine::LoadSignatures(int cc, char **cv, char *libdir)
{
	FILE *sngfp; // File Pointers for signature and host-script respectively.

	// Clear any old signatures
	if( this->pFirstSignature == NULL )
		this->pLastSignature = NULL;
	else
	{
		CSignature *pSig, *pxSig;

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
			if( strstr(dep->d_name, ".sng") == NULL )
				continue;

			CSignature *pNewSig = new CSignature;

			sprintf(libname, "%s/%s", libdir, dep->d_name);

			if( bDebug && (nDebug >= 1) )
				printf("loading signature %s\n", libname);

			if( (sngfp = fopen(libname, "r")) == NULL )
			{
				printf("cant open signature file %s\n", libname);
				delete pNewSig;
				continue;
			}

			// Read Signature File Header
			nr = fread(&(pNewSig->SigHeader), sizeof(struct newsigheader), 1, sngfp);

			if( strncmp(SIGNEWHDRMAGIC, (char *)pNewSig->SigHeader.szMagic, 4 ) != 0 )
			{
				printf("bad signature header %s\n", dep->d_name);
				fclose(sngfp);
				delete pNewSig;
				continue;
			}

			// Read the Dft
#ifdef FFTW_ENABLE_FLOAT
			double dDft[this->nDftSamPerBlock];

			nr = fread(dDft, sizeof(double), this->nDftSamPerBlock, sngfp);

			for( int id = 0; id < this->nDftSamPerBlock; id++ )
				pNewSig->pDft[id] = (fftw_real)dDft[id];
	
			nr = fread(dDft, sizeof(double), this->nDftSamPerBlock/2+1, sngfp);
			if( nr > 0 )
			{
				for( int id = 0; id < this->nDftSamPerBlock/2+1; id++ )
					pNewSig->pZep[id] = (fftw_real)dDft[id];
			}
	
#else
			nr = fread(pNewSig->pDft, sizeof(fftw_real), this->nDftSamPerBlock, sngfp);
			nr = fread(pNewSig->pZep, sizeof(fftw_real), this->nDftSamPerBlock/2+1, sngfp);
#endif

			// Close the Signature File.
			fclose(sngfp);

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
			CSignature *pNewSig = new CSignature;

			if( bDebug && (nDebug >= 0) )
				printf("loading signature %s\n", cv[is]);

			if( (sngfp = fopen(cv[is], "r")) == NULL )
			{
				printf("cant open signature file %s\n", cv[is]);
				delete pNewSig;
				continue;
			}

			// Read Signature File Header
			nr = fread(&(pNewSig->SigHeader), sizeof(struct newsigheader), 1, sngfp);

			if( strncmp(SIGNEWHDRMAGIC, (char *)pNewSig->SigHeader.szMagic, 4 ) != 0 )
			{
				printf("bad signature header %s\n", cv[is]);
				fclose(sngfp);
				delete pNewSig;
				continue;
			}

			// Read the Dft
#ifdef FFTW_ENABLE_FLOAT
			double dDft[this->nDftSamPerBlock];

			nr = fread(dDft, sizeof(double), this->nDftSamPerBlock, sngfp);

			for( int id = 0; id < this->nDftSamPerBlock; id++ )
				pNewSig->pDft[id] = (fftw_real)dDft[id];
	
			nr = fread(dDft, sizeof(double), this->nDftSamPerBlock/2+1, sngfp);
			if( nr > 0 )
			{
				for( int id = 0; id < this->nDftSamPerBlock/2+1; id++ )
					pNewSig->pZep[id] = (fftw_real)dDft[id];
			}
		
#else
			nr = fread(pNewSig->pDft, sizeof(fftw_real), this->nDftSamPerBlock, sngfp);
			nr = fread(pNewSig->pZep, sizeof(fftw_real), this->nDftSamPerBlock/2+1, sngfp);
#endif

			// Close the Signature File.
			fclose(sngfp);

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
// Correlate two blocks of DFT's
//
fftw_real CStaticEngine::Correlate(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor)
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
		if( spread_sig[i] < fMeanFactor )
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
fftw_real CStaticEngine::VCorrelate(fftw_real *pAudio, fftw_real *pSignature, int nSamples, fftw_real fMeanFactor)
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
	for( i=0; i<nSamples; i++ )
	{
		if( spread_sig[i] < fMeanFactor )
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

BOOL	CStaticEngine::MakeRealToComplexFDft(fftw_real *pAudio, fftw_real *pCDft)
{
	rfftw_one(this->theForwardPlan, pAudio, pCDft);

	return TRUE;
}

//
//	side effect: damages pCDft
//
BOOL	CStaticEngine::MakeComplexToRealFDft(fftw_real *pCDft, fftw_real *pAudio)
{
	rfftw_one(this->theReversePlan, pCDft, pAudio);
	return TRUE;
}

//	convert p_in_dft into a Zepstrum, mask everything outside iLo <> iHi-1, then take the 
//	inverse to create the masked DFT.
//
BOOL	CStaticEngine::MaskVoice(fftw_real *p_mask_dft, fftw_real *p_in_dft, int iLo, int iHi)
{
	int i;
	fftw_real f_tmpdft[this->nDftSamPerBlock];

	// Create the Zepstrum
	for( i = 1; i< this->nDftSamPerBlock; i++ )
		p_in_dft[i] = log(p_in_dft[i]);

	rfftw_one(this->theCPlan, &p_in_dft[1], f_tmpdft);

	// mask the zepstrum

	f_tmpdft[0] = 0;

	for( i=1; i< this->nDftSamPerBlock/2; i++ )
	{
		if( (i<iLo) || (i >= iHi) )
		{
			f_tmpdft[i] = 0;
			f_tmpdft[(this->nDftSamPerBlock-1)-i] = 0;
		}
	}

	rfftw_one(this->theReverseCPlan, f_tmpdft, &p_mask_dft[1]);

	p_mask_dft[0] = 0;
	for( i = 1; i< this->nDftSamPerBlock; i++ )
		p_mask_dft[i] = exp(p_mask_dft[i]);


	return TRUE;
}
