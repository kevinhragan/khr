#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include "sni.h"
#include "fftw.h"
#include "rfftw.h"
#include "sni_engine.h"
#include "sni_cluster.h"
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
//	snicluster [-o<sncfile>] <snifile> <snifile> ...
//
//	Calculate the sum total of the cross-distances between all the snifile combinations
//	and then find and output the cluster centers.
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

static char szRcsId[] = "$Id: snicluster.cc,v 1.16 2005-12-30 21:18:55 martind Exp $";

BOOL bDebug = FALSE;
int nDebug = 0;
char logbuf[256];

// globals
int nQuantum = 5;
int nAudioSamPerBlock = 400;
fftw_real fThreshold = 0.70;
char *outputfile = NULL;

BOOL bThreshold = TRUE;
BOOL bExclude = TRUE;

char *options = "?ID:C::o:";
// -D<debug>
// -C[<correl>=0.50]
// -o<outputfile>
// -I		// include self comparisons.

CSniEngine *pTheEngine;

// parameter gathering
int cc = 0;
char **cv = NULL;

//
//	snicluster
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
			printf("snicmp -D<x> [-X] [-d<min>=0.40] [-o<outoutfile>] [-C[<correl>=0.70]] <unknown> <known1> <known2> ...\n");
			exit(0);
			break;

		case 'D':
			bDebug = TRUE;
			nDebug = atoi(optarg);
			break;

		case 'C':
			bThreshold = TRUE;
			if( optarg )
				fThreshold = atof(optarg);
			break;

		case 'o':
			outputfile = strdup(optarg);
			break;

		case 'I':
			bExclude = FALSE;
			break;
		}
	} 

	// next param is snifile, must be present
	if( optind >= ac )
	{
		printf("no sni file specified\n");
		printf("snicluster [-D<n>] [-o<sncfile>] <snifile> <snifile> ...\n");
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

	if( cc < 1 )
	{
		printf("need at least one signature file parameter\n\n");
		printf("snicluster [-D<n>] [-o<sncfile>] <snifile> <snifile> ...\n");
		exit(1);
	}

	pTheEngine = new CSniEngine();
	pTheEngine->Initialize(nAudioSamPerBlock, nQuantum);
	// we don't use the fft engine at all. we use LoadSignature only.

	int iff, icc;

	sni_result *clusters[cc][cc];
	CSniSig *signatures[cc];

	int ntotaldfts = 0;

	for(iff=0; iff<cc; iff++)
	{
		signatures[iff] = pTheEngine->LoadSignature(cv[iff]);

		if( signatures[iff] == NULL )
		{
			printf("cant load %s, exiting\n", cv[iff]);
			exit(1);
		}

		if( signatures[iff]->pFirstBlock == NULL )
		{
			// no data, so we fix up a block;
			sni_block *pb = (sni_block *)calloc(1, sizeof(sni_block));

			CSniSig *ps = signatures[iff];

			ps->pFirstBlock = ps->pLastBlock = pb;
			pb->block.nDfts = pb->block.nQuantum = 0;
			pb->block.iType = SNIT_PHASE;
		}
		if( signatures[iff] )
		{
			ntotaldfts += signatures[iff]->pFirstBlock->block.nDfts*signatures[iff]->pFirstBlock->block.nQuantum;
		}

		for(icc=0; icc<cc; icc++)
			clusters[iff][icc] = NULL;
	}

	if( ntotaldfts == 0 )
	{
		printf("no dfts to process\n");
		exit(1);
	}

	printf("processing a total of %d dfts, %d calculations in %d files\n", ntotaldfts, (ntotaldfts*ntotaldfts/2) - ntotaldfts, cc);
	assert( signatures[0]->pFirstBlock->block.iType == SNIT_PHASE);

	int idft = 0;
	int nfreq = 0;

	for(iff=0; iff<cc; iff++)
	{
		CSniSig *psU = signatures[iff];;

		if( psU == NULL )
		{
			printf("can't load signature %s\n", cv[iff]);
			continue;
		}

		for(icc=0; icc<cc; icc++)
		{
			fftw_real ftotal = 0;
			int ntotal = 0;

			// if( bDebug && (nDebug >= 0))
				printf("comparing %s and %s\n", cv[iff], cv[icc]);

			CSniSig *psK = signatures[icc];

			if( psK == NULL )
			{
				printf("can't load signature %s\n", cv[icc]);
				continue;
			}

			sni_block *pbU, *pbK;
			int nU = 0, nK = 0;

			for( pbU=psU->pFirstBlock; pbU; pbU = pbU->pnext )
			{
				nK = 0;
				for( pbK = psK->pFirstBlock; pbK; pbK = pbK->pnext )
				{
					if( bDebug && (nDebug >= 0))
						printf("comparing %s[%d] with %s[%d]\n", psU->header.sTag, nU, psK->header.sTag, nK);

					sni_result *pr;

					if( (icc == iff) && (cc > 1) && bExclude )
						pr = NULL;
					else if( (pbU->block.nDfts > 0) && (pbK->block.nDfts > 0) )
					{
						pr = pTheEngine->CompareBlock(pbU, pbK);

						if( pr == NULL )
						{
							printf("cannot compare %s[%d] with %s[%d]\n", psU->header.sTag, nU, psK->header.sTag, nK);
						}
					}
					else
						pr = NULL;

					// free(pr->presult);
					// free(pr);
					clusters[iff][icc] = pr;

					nK++;
				}
				nU++;
			}
		}
	}

	// to make things easy we transcribe the data
	fftw_real *distances;
	assert( signatures[0]->pFirstBlock->block.iType == SNIT_PHASE);

	distances = (fftw_real *)calloc(ntotaldfts*ntotaldfts, sizeof(fftw_real));

	int idftr=0;

	for( iff=0; iff<cc; iff++ )
	{
		int idftc = 0;
		int wr;

		wr = signatures[iff]->pFirstBlock->block.nDfts*signatures[iff]->pFirstBlock->block.nQuantum;
	
		if( bDebug && (nDebug >= 0))
			printf("collecting data for %s\n", cv[iff]);

		for( icc=0; icc<cc; icc++ )
		{
			int row, col;
			int wc;

			wc = signatures[icc]->pFirstBlock->block.nDfts*signatures[icc]->pFirstBlock->block.nQuantum;

			sni_result *sr = clusters[iff][icc];

			for(row=0; row<wr; row++)
			{
				for( col=0; col<wc; col++)
				{
					if(sr)
					{
						distances[(idftr+row)*ntotaldfts + idftc+col] = sr->presult[(row*wc)+col];

						//distances[idftr+row][idftc+col] = sr->presult[(row*wc)+col];
						if( bDebug && (nDebug >= 1))
						{
							printf("d[%d][%d]=p[%d]=%4.1f |", idftr+row, idftc+col, (row*wc)+col, sr->presult[(row*wc)+col]);
							if( col%5 == 0 )
								printf("\n");
						}
					}
					else
						distances[(idftr+row)*ntotaldfts + idftc+col] = 0;
						// distances[idftr+row][idftc+col] = 0;
				}
				if( bDebug && (nDebug >= 1))
					printf("\n");
			}

			idftc += wc;
		}

		idftr += wr;
	}

	assert( idftr == ntotaldfts );
	assert( signatures[0]->pFirstBlock->block.iType == SNIT_PHASE);

	hitpoint ipart[ntotaldfts];
	int ii, ij;
	int k = 0;

	// now we go through every block pair. For each block we record
	// the number of hits (cells > fThreshold), the number of chains of length >= 2
	// and the length of the maximum chain.
	fftw_real ftt = fThreshold;

	while( k <= 0 )
	{
		for( ii=0; ii<ntotaldfts; ii++ )
		{
			int nhits = 0;

			if( bDebug && (nDebug >= 0))
				printf("%-4d of %-4d\r", ii, ntotaldfts);

			for( ij=0; ij<ntotaldfts; ij++ )
			{
				if( (distances[ii*ntotaldfts+ij] >= ftt) || (!bThreshold) )
				{
					nhits++;
				}
			}

			if( nhits > 0 )
			{
				ipart[k].nhits = nhits;
				ipart[k++].dftindex = ii;
			}
		}

		ftt -= 0.025;
		if( ftt <= 0.4 )
			break;
	}

	if( bDebug && (nDebug >= 0) )
		printf("\nfound %d/%d blocks, threshold used = %4.2f\n", k, ntotaldfts, ftt);

	if( outputfile == NULL )
		exit(0);

	// 
	//	this is very complicated code to create a new signature file from
	//	our best dfts. They are written out into a specific new single block signature
	//	file called 'sTag[0].snc', but using the standard sni format.
	//
	CSniSig *pSig = new CSniSig;
	sni_block *pBlock = NULL;

	// setup the Header
	strcpy(pSig->header.szmagic, SNIHDRMAGIC);
	time(&pSig->header.tCreate);
	strcpy(pSig->header.sRouting, signatures[0]->header.sRouting);
	strcpy(pSig->header.sCallerId, signatures[0]->header.sCallerId);
	strcpy(pSig->header.sAuthor, signatures[0]->header.sAuthor);
	strcpy(pSig->header.sTag, signatures[0]->header.sTag);
	strcpy(pSig->header.sMessageId, outputfile);

	// we start a new block
	pBlock = (sni_block *)calloc(sizeof(sni_block), 1);
	pSig->pFirstBlock = pSig->pLastBlock = pBlock;
	pSig->header.nDftBlocks = 1;

	pBlock->block.magic = 0xFFFFFFFF;
	pBlock->block.iAudioOffset = 0;
	pBlock->block.nAudioSamPerBlock = signatures[0]->pFirstBlock->block.nAudioSamPerBlock;
	pBlock->block.nQuantum = 1;
	pBlock->block.nDftSamPerBlock = signatures[0]->pFirstBlock->block.nDftSamPerBlock;
	assert( signatures[0]->pFirstBlock->block.iType == SNIT_PHASE);
	pBlock->block.iType = SNIT_PHASE;
	pBlock->block.iSizeof = sizeof(fftw_complex);
	pBlock->block.iFlags = signatures[0]->pFirstBlock->block.iFlags;
	pBlock->block.nDfts = k;

	pBlock->pdata = (fftw_real *)malloc(pBlock->block.iSizeof*pBlock->block.nDftSamPerBlock*k);
	fftw_complex *pb = (fftw_complex *)(pBlock->pdata);

	// now we have to find the blocks in memory and copy them.

	for( iff=0; iff<k; iff++ )
	{
		// step through the files
		int istep = 0;

		for( icc=0; icc<cc; icc++ )
		{
			CSniSig *ps = signatures[icc];

			sni_block *pd;
			int nsigdfts = 0;

			for( pd = ps->pFirstBlock; pd; pd = pd->pnext )
				nsigdfts += pd->block.nDfts*pd->block.nQuantum;

			if( (istep+nsigdfts) < ipart[iff].dftindex )
			{
				istep += nsigdfts;
				continue;
			}

			// now we have selected the correct signature, we have to find the right block.

			int idex = ipart[iff].dftindex - istep; 

			for( pd = ps->pFirstBlock; pd && (istep+(pd->block.nDfts*pd->block.nQuantum) < ipart[iff].dftindex); pd = pd->pnext )
			{
				istep += pd->block.nDfts*pd->block.nQuantum;
			}

			// we can now step into the sigblock.
			fftw_complex *pi = (fftw_complex *)pd->pdata;
			fftw_complex *pc = pi+(pd->block.nDftSamPerBlock*(ipart[iff].dftindex-istep));

			for(ij=0; ij<pBlock->block.nDftSamPerBlock; ij++)
				pb[ij] = pc[ij];

			break;
		}

		pb += pBlock->block.nDftSamPerBlock;
	}

	if( bDebug && (nDebug >= 0) )
		pTheEngine->PrintSignatureHeader(pSig, outputfile);
	pTheEngine->SaveSignature(pSig, outputfile);
}
