#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include "sni.h"
#include "fftw.h"
#include "rfftw.h"
#include "sni_engine.h"
#include "wave.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <assert.h>
#include <math.h>

//
//	snicmp -D<n> -X <known> <unknown>
//	
//	
int hitcompare(const void *a, const void *b)
{
	hitpoint fa, fb;

	fa = *(hitpoint*)a;
	fb = *(hitpoint*)b;

	if( fa.nhits > fb.nhits )
		return 1;
	else if( fb.nhits > fa.nhits )
		return -1;
	else
		return 0;
}

int chaincompare(const void *a, const void *b)
{
	hitpoint fa, fb;

	fa = *(hitpoint*)a;
	fb = *(hitpoint*)b;

	if( fa.nchains > fb.nchains )
		return 1;
	else if( fb.nchains > fa.nchains )
		return -1;
	else
		return 0;
}

int scorecompare(const void *a, const void *b)
{
	fftw_real fa, fb;

	fa = *(fftw_real*)a;
	fb = *(fftw_real*)b;

	if( fa > fb )
		return 1;
	else if( fb > fa )
		return -1;
	else
		return 0;
}

static char szRcsId[] = "$Id: snicmp.cc,v 1.28 2005-06-21 00:06:23 martind Exp $";

BOOL bDebug = FALSE;
int nDebug = 0;
char logbuf[256];

// globals
int nQuantum = 8;
int nAudioSamPerBlock = 400;
fftw_real fThreshold = 0.50;

BOOL bThreshold = TRUE;
BOOL bVisual = FALSE;
char *output = NULL;
int ixclude = -1;
BOOL bTransition = FALSE;

char *options = "?D:SC::vo:X:T";
// -D<debug>
// -C[<correl>=0.70]
// -S (sumamry)
// -v (visual)
// -o<outputfile>
// -X<excludeblock>
// -T (transition table)
CSniEngine *pTheEngine;

BOOL bSummary = FALSE;
int density[100];

// parameter gathering
int cc = 0;
char **cv = NULL;

//
//	a little needs to be said about magic numbers.
//
//	as of 3/15/05, I was trying the following:
//
//		snigen -n160 -b<timestamp> -B2 -q1 foo.wav
//		snigen -n160 -b<timestamp> -B2 -q1 bar.wav
//
//	then,
//		snicmp -p -C0.90 foo.sni bar.sni
//
main(int ac, char **av)
{
	int oc;

	while( (oc = getopt(ac, av, options)) > 0 )
	{
		switch( oc )
		{
		default:
		case '?':
			printf("snicmp -D<x> [-X] [-S] [-p] [-v] [-C[<correl>=0.70]] <unknown> <known1> <known2> ...\n");
			exit(0);
			break;

		case 'T':
			bTransition = TRUE;
			bSummary = TRUE;
			break;

		case 'X':
			ixclude = atoi(optarg);
			break;

		case 'o':
			output = strdup(optarg);
			break;

		case 'v':
			bVisual = TRUE;
			break;

		case 'D':
			bDebug = TRUE;
			nDebug = atoi(optarg);
			break;

		case 'S':
			bSummary = TRUE;
			break;

		case 'C':
			bThreshold = TRUE;
			if( optarg )
				fThreshold = atof(optarg);
			break;

		}
	} 

	// next param is snifile, must be present
	if( optind >= ac )
	{
		printf("no sni file specified\n");
		printf("snicmp -D<x> [-X] [-S] [-p] [-v] [-C[<correl>=0.70]] <unknown> <known1> <known2> ...\n");
		exit(1);
	}

	cc = 0;
	cv = NULL;
	int io;

	io = optind;

	if( io < ac )
	{
		// allocate memory for pointers
		cv = (char **)malloc(sizeof(char *)*(1+(ac-io)));
		assert(cv);
		cv[ac-io] = '\0';
	}

	while( io < ac )
	{
		// setup the parameters
		cv[cc++] = av[io++];
	}

	if( cc < 2 )
	{
		printf("need at least, two signature file parameters\n\n");
		printf("snicmp -D<x> [-X] [-S] [-p] [-v] [-C[<correl>=0.70]] <unknown> <known1> <known2> ...\n");
		exit(1);
	}
	
	if( (cc != 2) && bTransition )
	{
		printf("-T option only allows two file paramters\n");
		printf("snicmp -S -T unknown.sni known_kluster.sni\n");
		exit(1);
	}

	pTheEngine = new CSniEngine();
	pTheEngine->Initialize(nAudioSamPerBlock, nQuantum);
	// we don't use the fft engine at all. we use LoadSignature only.

	CSniSig *psU = pTheEngine->LoadSignature(cv[0]);

	if( psU == NULL )
	{
		printf("can't load signature %s\n", cv[0]);
		exit(1);
	}

	int nk = 0;

	if( bTransition )
	{
		CSniSig *ps = pTheEngine->LoadSignature(cv[1]);

		if( ps == NULL )
		{
			printf("can't load signature %s\n", cv[1]);
			exit(1);
		}

		sni_block *pbK = ps->pFirstBlock;

		if( (pbK == NULL) || (pbK->block.nDfts == 0) )
		{
			printf("no dfts in signature file %s\n", cv[1]);
			exit(1);
		}

		nk = pbK->block.nDfts;
		delete ps;
	}

	int transitions[nk][nk];
	int icc, iff;
	int nformants = 0;
	fftw_real ftscore = 0;
	fftw_real *scores = NULL; 
	int ncols = 0;
	int nrows = 0;

	for( icc=0; icc<nk; icc++ )
		for( iff=0; iff<nk; iff++ )
			transitions[icc][iff] = 0;

	for(icc=1; icc<cc; icc++)
	{
		fftw_real ftotal = 0;
		int ntotal = 0;

		 if( bDebug && (nDebug >= 0))
			printf("comparing %s and %s\n", cv[0], cv[icc]);

		CSniSig *psK = pTheEngine->LoadSignature(cv[icc]);

		if( psK == NULL )
		{
			printf("can't load signature %s\n", cv[icc]);
			exit(1);
		}

		sni_block *pbU, *pbK;
		int nU = 0, nK = 0;

		for( pbU=psU->pFirstBlock; pbU; pbU = pbU->pnext )
		{
			nrows = 0;
			BOOL *pbRows = (BOOL *)calloc(pbU->block.nDfts*pbU->block.nQuantum, sizeof(BOOL));
			nK = 0;
			for( pbK = psK->pFirstBlock; pbK; pbK = pbK->pnext )
			{
				if( bDebug && (nDebug >= 0))
					printf("comparing %s[%d] with %s[%d]\n", psU->header.sTag, nU, psK->header.sTag, nK);

				// for each block pair, we get a correlation matrix (pr)
				sni_result *pr = pTheEngine->CompareBlock(pbU, pbK);

				if( pr == NULL )
				{
					printf("cannot compare %s[%d] with %s[%d]\n", psU->header.sTag, nU, psK->header.sTag, nK);
					continue;
				}

				int ii, ij;
				fftw_real *psummary = NULL;
				int *pnsum = NULL;

				int hits[pr->rows/pbU->block.nQuantum];
				hitpoint allhits[pr->rows];
				int ntotalhits = 0;
				int ntotalchains = 0;
				fftw_real fmeanhits = 0;

				for( ii=0; ii<pr->rows/pbU->block.nQuantum; ii++)
					hits[ii] = 0;

				// used to count the number of columns involved
				BOOL *pbCols = (BOOL *)calloc(pr->cols, sizeof(BOOL));
				ncols = 0;

				// now we go through every block pair. For each block we record
				// the number of hits (cells > fThreshold), the number of chains of length >= 2
				// and the length of the maximum chain.
				for( ii=0; ii<pr->rows; ii++ )
				{
					int nhits = 0;
					int nchains = 0;
					int maxchain = 0;
					int clength = 0;
					BOOL bInChain = FALSE;

					for( ij=0; ij<pr->cols; ij++ )
					{
						if( (pr->presult[ii*pr->cols+ij] >= fThreshold) || (!bThreshold) )
						{
							if( pbCols[ij] == FALSE )
							{
								pbCols[ij] = TRUE;
								ncols++;
							}
							if( pbRows[ii] == FALSE )
							{
								pbRows[ii] = TRUE;
								nrows++;
							}
							nhits++;

							if( bInChain == FALSE )
							{
								bInChain = TRUE;
								clength = 0;
							}
							else
							{
								clength++;
								if( clength > maxchain )
									maxchain = clength;
							}
						}
						else
						{
							if( (bInChain == TRUE) && (clength >= 1) )
								nchains++;

							if( (bInChain == TRUE) && (clength >= 2) )
								ntotalchains++;

							clength = 0;
							bInChain = FALSE;
						}
					}

					if( bDebug && (nDebug >= 1))
						printf("%d = %d\n", ii, nhits);

					hits[ii/pbU->block.nQuantum] += nhits;
					ntotalhits += nhits;
					allhits[ii].nhits = nhits;
					allhits[ii].dftindex = ii;
					allhits[ii].nchains = nchains;
					allhits[ii].maxchain = maxchain;
				}

				free(pbCols);
				free(pbRows);

				fmeanhits = (float)(ntotalhits)/pr->rows;

				if( bSummary )
				{
					psummary = (fftw_real *)calloc(pbU->block.nDfts*pbK->block.nDfts, sizeof(fftw_real));
					pnsum = (int *)calloc(pbU->block.nDfts*pbK->block.nDfts, sizeof(int));
					
					int nt = 0, ntt = 0;
					int id=0, nd=0;		// index of densest row, number of hits for that row
					int nc = 0;
					fftw_real ft=0, fc=0;
					int *irows = (int *)calloc(pr->rows, sizeof(int));

					for( ii=0; ii<pr->rows; ii++ )
						for( ij=0; ij<pr->cols; ij++ )
						{
							if( (pr->presult[ii*pr->cols+ij] >= fThreshold) || (!bThreshold) )
							{
								if( irows[ii] == 0 )
								{
									nt++;
								}
								irows[ii]++;
								ntt++;

								// find the densts matching dft
								if( irows[ii] > nd )
								{
									nd = irows[ii];
									id = ii;
								}

								ft += pr->presult[ii*pr->cols+ij]*pr->presult[ii*pr->cols+ij];
							}

#ifdef NEVER
							if( isnan(pr->presult[ii*pr->cols+ij]) )
							{
								printf("pr[%d][%d] = %f\n", ii, ij, pr->presult[ii*pr->cols+ij]);
								pr->presult[ii*pr->cols+ij] = 0;
							}
#endif

							fc += pr->presult[ii*pr->cols+ij]*pr->presult[ii*pr->cols+ij];
							nc++;

							assert((int)fc <= nc);

							if( pr->presult[ii*pr->cols+ij] > 
								psummary[(ii/pbU->block.nQuantum)*pbK->block.nDfts+ij/pbK->block.nQuantum] )
							{
								psummary[(ii/pbU->block.nQuantum)*pbK->block.nDfts+ij/pbK->block.nQuantum] 
														= pr->presult[ii*pr->cols+ij];
							}

							pnsum[(ii/pbU->block.nQuantum)*pbK->block.nDfts+ij/pbK->block.nQuantum]++; 
						}

					if( bDebug && (nDebug >= 1) )
						printf("ft = %f, fc = %f\n", ft, fc);

					int ilaststate = -1;

					for( ii=0; ii<pbU->block.nDfts; ii++ )
					{
						int ibest = 0;
						fftw_real fBest = 0.0;
						fftw_real fMean = 0.0;

						if( !bTransition )
							printf("%d %d |", ii, hits[ii]);
						for( ij=0; ij<pbK->block.nDfts; ij++ )
						{
							if( !bTransition )
							{
								if( (psummary[ii*pbK->block.nDfts+ij] >= fThreshold) || (!bThreshold) )
									printf("%2.0f", 100*psummary[ii*pbK->block.nDfts+ij]); 
								else
									printf("--");
							}

							fMean += psummary[ii*pbK->block.nDfts+ij];

							if( psummary[ii*pbK->block.nDfts+ij] > fBest )
							{
								if( ixclude != ij )
								{
									ibest = ij;
									fBest = psummary[ii*pbK->block.nDfts+ij];
								}
							}
						}
						fMean /= pbK->block.nDfts;
						if( !bTransition )
							printf("| %d %3.2f %3.2f %3.3f\n", ibest, fBest, fMean, fabs(fMean-fBest));
						else
						{
							// we only change state if the delta from the mean distance is big enough
							if( (ilaststate >= 0) && (fabs(fMean-fBest) >= 0.15) )
								transitions[ilaststate][ibest]++;

							if( (fabs(fMean-fBest) >= 0.15) )
								ilaststate = ibest;
						}
					}

					free(psummary);
					free(pnsum);

					free(irows);
				}
				else if( bDebug && (nDebug >= 1) )
				{
					for( ii=0; ii<pr->rows; ii++ )
					{
						if( ((ii%pbU->block.nQuantum) == 0) && (ii>0) )
						{
							for( ij=0; ij<pr->cols*2+(pr->cols/pbK->block.nQuantum); ij++ )
								printf("-");
							printf("\n");
						}

						for( ij=0; ij<pr->cols; ij++ )
						{
							if( ij%pbK->block.nQuantum == 0 )printf("|");
							if( (pr->presult[ii*pr->cols+ij] >= fThreshold) || (!bThreshold) )
								printf("%2.0f", 100*pr->presult[ii*pr->cols+ij]); 
							else
								printf("--");

						}
						printf("\n");
					}
				}

				int done = 0;

				for( ii=0; ii<pr->rows; ii++ )
				{
					if( (allhits[ii].nhits > 0) && (allhits[ii].nchains > 1) && (allhits[ii].maxchain > 1) )
					{
						if( ((cc==2) && strcmp(cv[0], cv[1]) == 0) && (allhits[ii].nhits < fmeanhits) )
							continue;

						if( bDebug && (nDebug >= 0) )
							printf("(%d %d = %d, %d, %d)", 
								allhits[ii].dftindex/pbU->block.nQuantum,
								allhits[ii].dftindex%pbU->block.nQuantum, 
								allhits[ii].nhits, allhits[ii].nchains, allhits[ii].maxchain);
						done++;
						ntotalhits += allhits[ii].nchains*allhits[ii].maxchain;

						if( done%8 == 0 )
							if( bDebug && (nDebug >= 0) )
								printf("\n");
					}
				}
				if( bDebug && (nDebug >= 0) )
					printf("\n");

				if( !bTransition )
					printf("%s %s | %d / %d %4.2f | %d / %d %4.2f\n", cv[0], cv[icc], 
									ncols, pr->cols, (float)(ncols*100)/(pr->cols), 
									nrows, pr->rows, (float)(nrows*100)/(pr->rows)); 

				if( pr->presult )
					free(pr->presult);

				if(pr)
					free(pr);

				nK++;
			}
			nU++;
		}


		delete psK;
	}

	if( bTransition && (nk > 0) )
	{
		fftw_real sums[nk];
		fftw_real ftotalsum = 0;

		for( icc=0; icc<nk; icc++ )
			sums[icc] = 0;

		for( icc=0; icc<nk; icc++ )
		{
			for( iff=0; iff<nk; iff++ )
				sums[icc] += transitions[icc][iff];

			ftotalsum += sums[icc];
		}

		for( icc=0; icc<nk; icc++ )
		{
			printf("%-2d %-4.1f | ", icc, 100.0*sums[icc]/ftotalsum); 
			for( iff=0; iff<nk; iff++ )
			{
				if( sums[icc] == 0 )
					printf(" 0 ");
				else
					printf("%2.0f ", (float)(100*transitions[icc][iff])/sums[icc]);
			}
			printf("\n");
		}
	}
}
