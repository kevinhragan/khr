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
//	snigen -D<n> -o<tag> <wavename>.wav
//	
//	

static char szRcsId[] = "$Id: snigen.cc,v 1.22 2005-06-21 00:06:23 martind Exp $";

BOOL bDebug = FALSE;
int nDebug = 0;
char logbuf[256];

// globals
int nQuantum = 5;
char *wavefile = NULL;
char *tag = NULL;
int nAudioSamPerBlock = 400;
int nst = 200;
char *routing = NULL;
char *callerid = NULL;
char *mid = NULL;
BOOL bInPlace = FALSE;
BOOL bComplex = TRUE;
BOOL bMultisig = FALSE;
char *author = "gotvoice";
char *output = NULL;
int nblank = 0;
int ntail = 0;
BOOL bDft = TRUE;
BOOL bAutomax = FALSE;
fftw_real fAutoFactor = 2.0;

char *options = "a::B:b:c?D:n:q:t:R:I::is:A:o:XV";
// -D<debug>
// -n<samplesize>=400
// -q<n>
// -t<tag>=<wavename> - set the tag
// -R<routing>
// -I<callerid>
// -i	- inplace (.sni in same directory as wav)
// -s<silence>=200
// -c	- turn off, save complex dft.
// -o outputfile
// -b<secs>	- blank out <secs> at start of message
// -B<secs>	- blank out <secs> at end of message
// -X 		- multiple blocks, by default put all audio in one sigblock.
// -V		- vectors only stored.
// -a[<factor>=2.0]		- only capture audio > max/2.

CSniEngine *pTheEngine;

// parameter gathering
int cc = 0;
char **cv = NULL;

main(int ac, char **av)
{
	int oc;

	while( (oc = getopt(ac, av, options)) > 0 )
	{
		switch( oc )
		{
		default:
		case '?':
			printf("snigen -D<n> [-a] [-b<in>] [-B<out>] [-i] -q<n>=5 -t<tag>=<wavename> -s<silence>=200 -I<cid> -R<routing> -n<bsize>=400 <wavename>.wav\n");
			printf("e.g.\n\tsnigen -q1 -n2000 -tEnterPassword -s0 foo.wav\n");
			printf("\n\tsnigen -q5 -n400 -ttinky -R8086962267 -I8085675674 0134050010.wav\n\n");
			exit(0);
			break;

		case 'a':
			bAutomax = TRUE;
	
			if( optarg )
				fAutoFactor = atof(optarg);
			break;

		case 'B':
			ntail = (int)(8000.00*atof(optarg));
			break;

		case 'b':
			nblank = (int)(8000.00*atof(optarg));
			break;

		case 'V':
			bDft = FALSE;
			break;

		case 'o':
			output = strdup(optarg);
			break;

		case 'A':
			author = strdup(optarg);
			break;

		case 'i':
			bInPlace = TRUE;
			break;

		case 'n':
			nAudioSamPerBlock = atoi(optarg);
			break;

		case 't':
			tag = strdup(optarg);
			break;

		case 'D':
			bDebug = TRUE;
			nDebug = atoi(optarg);
			break;

		case 'q':
			nQuantum = atoi(optarg);
			break;

		case 's':
			nst = atoi(optarg);
			break;

		case 'R':
			routing = strdup(optarg);
			break;

		case 'I':
			if( optarg )
				callerid = strdup(optarg);
			break;

		case 'c':
			bComplex = FALSE;
			break;

		case 'X':
			bMultisig = TRUE;
			break;
		}
	} 

	// next param is wavefile, must be present
	if( optind >= ac )
	{
		fprintf(stderr, "no wave file specified\n");
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

	wavefile = cv[0];

	// must specify output file if cc > 1
	if( (cc > 1) && (output == NULL) )
	{
		printf("with more than one wave file, you must name the output -o<filename>\n");
		exit(1);
	}

	int icc;
	int ndfts = 0;
	int nblocks = 0;

	pTheEngine = new CSniEngine();
	pTheEngine->Initialize(nAudioSamPerBlock, nQuantum);
	// we don't use the fft engine at all. we use LoadSignature only.

	// setup the Header
	CSniSig *pSig = new CSniSig;
	strcpy(pSig->header.szmagic, SNIHDRMAGIC);
	pSig->header.nDftBlocks = 0;
	time(&pSig->header.tCreate);

	if( routing )
		strcpy(pSig->header.sRouting, routing);

	if( callerid )
		strcpy(pSig->header.sCallerId, callerid);

	if( author )
		strcpy(pSig->header.sAuthor, author);

	char *dp, *sp;

	if( output )
		mid = strdup(output);
	else
		mid = strdup(wavefile);

	dp = strrchr(mid, '/');
	sp = strrchr(mid, '.');

	if( sp ) *sp = '\0';
	if( dp )
		mid = ++dp;

	strcpy(pSig->header.sMessageId, mid);
 
	if( tag )
		strcpy(pSig->header.sTag, tag);
	else
		strcpy(pSig->header.sTag, mid);

	for( icc = 0; icc< cc; icc++ )
	{

		if( bDebug && (nDebug <= 0) )
			printf("processing %s\n", cv[icc]);

		struct waveheader whdr;
		struct waveformat wfmt;
		struct datahdr wdata;

		short *bbuffer = NULL;
		int nsamples = 0;

		FILE *bfp = fopen(cv[icc], "r");

		if( bfp == NULL )
		{
			fprintf(stderr, "cant open wavefile %s\n", cv[icc]);
			exit(1);
		}

		fread(&whdr, 1, sizeof(struct waveheader), bfp);

		if( strncmp( (char *)&whdr.lRiff, "RIFF", 4) != 0 )
		{
			fprintf(stderr, "dont support non-wave files\n");
			exit(1); 
		}
		else
		{
			// wave file
			fread(&wfmt, 1, whdr.lFormatLength, bfp);

			if( (wfmt.nChannels != 1) || (wfmt.nSamplesPerSec != 8000) || (wfmt.wBitsPerSample != 16) )
			{
				fprintf(stderr, "unsupported wav format in %s : channels = %d, Samples/s = %d, Bytes/sample = %d\n",
								cv[icc], wfmt.nChannels, wfmt.nSamplesPerSec, wfmt.wBitsPerSample/8 );
				exit(1);
			}
			else if( bDebug && (nDebug >= 1) )
				printf("scanning %s : channels = %d, Samples/s = %d, Bytes/sample = %d\n",
								cv[icc], wfmt.nChannels, wfmt.nSamplesPerSec, wfmt.wBitsPerSample/8 );

			fread(&wdata, 1, sizeof(struct datahdr), bfp);

			nsamples = wdata.lLenData/sizeof(short);
			nsamples = (nsamples/nAudioSamPerBlock)*nAudioSamPerBlock;

			if( nsamples-nblank-ntail < nAudioSamPerBlock )
			{
				printf("wave file too small (needed %d, too short by %d)\n", nAudioSamPerBlock, -(nsamples-nblank-ntail));
				fclose(bfp);
				exit(1);
			}

			if( nblank > 0 )
			{
				printf("skipping %3.1fs.\n", (float)(nblank)/8000.00);

				nsamples -= nblank;
				fseek(bfp, nblank*sizeof(short), SEEK_CUR);	// skip over blanked data
			}

			if( ntail > 0 )
			{
				printf("trimming %3.1fs.\n", (float)(ntail)/8000.00);

				nsamples -= ntail;
			}

			// read all the data in one shot.
			bbuffer = (short *)malloc(sizeof(short) * nsamples);
			fread(bbuffer, nsamples, sizeof(short), bfp);
			fclose(bfp);

			// Now we pre-compute the dft's for file b.
			if( bDebug && (nDebug >= 0) )
				printf("loaded %d samples (%d blocks)\n", nsamples, nsamples/pTheEngine->nAudioSamPerBlock);
		}

		assert(nAudioSamPerBlock%nQuantum == 0);

		// create the tag
		int ix = 0;
		sni_block *pBlock = NULL;
		BOOL bInBlock = FALSE;
		BOOL bInSequence = FALSE;
		int iseq = 0;
		// build the dft blocks
		fftw_real fMaxPower = 0;

		for( ix=0; (ix+nAudioSamPerBlock) <= nsamples; ix += nAudioSamPerBlock)
		{
			fftw_real fPower;
			int j;

			if( (nQuantum>1) && ((ix+nAudioSamPerBlock*2) > nsamples ) )
				break;

			for( j=0; j<nAudioSamPerBlock; j++ )
			{
				fPower += pow((fftw_real)(bbuffer[ix+j]),2);
			}

			fPower = sqrt(fPower/nAudioSamPerBlock);
			if( fPower > fMaxPower )
				fMaxPower = fPower;
		}

		if( bAutomax )
		{
			nst = (int)(fMaxPower/fAutoFactor);
		}
		printf("max power = %d, nst = %d, factor = %4.2f\n", (int)fMaxPower, nst, fAutoFactor);

		// by breaking the file into utterances, we create one sni_block per utterance.
		//
		for( ix=0; (ix+nAudioSamPerBlock) <= nsamples; ix += nAudioSamPerBlock)
		{
			fftw_real fPower;
			int j;

			if( (nQuantum>1) && ((ix+nAudioSamPerBlock*2) > nsamples ) )
				break;

			for( j=0; j<nAudioSamPerBlock; j++ )
			{
				fPower += pow((fftw_real)(bbuffer[ix+j]),2);
			}

			fPower = sqrt(fPower/nAudioSamPerBlock);

			if( (fPower >= nst) && (bInSequence == FALSE) )
			{
				iseq = 0;
				bInSequence = TRUE;
			}
				
			if( (fPower >= nst) && (bInBlock == FALSE) )
			{
				nblocks++;
				// we start a new block
				pBlock = (sni_block *)calloc(sizeof(sni_block), 1);

				pBlock->block.magic = 0xFFFFFFFF;
				pBlock->block.iAudioOffset = ix+nblank;
				pBlock->block.nAudioSamPerBlock = pTheEngine->nAudioSamPerBlock;
				pBlock->block.nQuantum = nQuantum;

				if( bComplex )
				{
					pBlock->block.nDftSamPerBlock = pTheEngine->nAudioSamPerBlock/2;
					pBlock->block.iType = SNIT_PHASE;
					pBlock->block.iSizeof = sizeof(fftw_complex);
					pBlock->block.iFlags = 0;
				}
				else
				{
					pBlock->block.nDftSamPerBlock = pTheEngine->nDftSamPerBlock;
					pBlock->block.iType = SNIT_POWER;
					pBlock->block.iSizeof = sizeof(fftw_real);
					pBlock->block.iFlags = 0;
				}
				pBlock->block.nDfts = 0;
				bInBlock = TRUE;

				if( bDebug && (nDebug >= 0) )
				{
					printf("newblock ao=%4.2f, aspb=%d, dspb=%d, q=%d, type=%s\n", (float)pBlock->block.iAudioOffset/8000.00,
						pBlock->block.nAudioSamPerBlock, pBlock->block.nDftSamPerBlock, pBlock->block.nQuantum,
						(pBlock->block.iType == SNIT_POWER)?"power":"phase");
				}

				pBlock->pdata = NULL;
			}

			if( bInBlock && (fPower >= nst) )
			{
				if( pBlock->pdata )
					pBlock->pdata = (fftw_real *)realloc(pBlock->pdata, 
							(pBlock->block.nDfts+1)*pBlock->block.iSizeof*pBlock->block.nDftSamPerBlock*nQuantum);
				else
					pBlock->pdata = (fftw_real *)malloc(pBlock->block.iSizeof*pBlock->block.nDftSamPerBlock*nQuantum);

				// we compute one dft per window.
				int astep = nAudioSamPerBlock/nQuantum;
				int idft = pBlock->block.nDftSamPerBlock*nQuantum*pBlock->block.nDfts;

				for( j=0; j<nQuantum; j++)
				{
					if( bComplex )
					{
						CFeatureVector *pFV;
						fftw_complex *pc = (fftw_complex *)pBlock->pdata;

						// now we compute the dft for this cycle and save it.
						pTheEngine->MakePhaseDft(bbuffer+ix+(j*astep), &pc[idft+(j*pBlock->block.nDftSamPerBlock)], NULL);

#ifdef NEVER
						pFV = pTheEngine->ComputeFeatureVector(&pc[idft+(j*pBlock->block.nDftSamPerBlock)], 20);

						if( pFV )
						{
							pFV->fEnergy = fPower;
							pFV->iSampleOffset = iseq++;
							pSig->AddFeatureVector(pFV);
						}
#endif
					}
					else
					{
						// now we compute the dft for this cycle and save it.
						pTheEngine->MakeDft(bbuffer+ix+(j*astep), &pBlock->pdata[idft+(j*pBlock->block.nDftSamPerBlock)], NULL);
					}
					ndfts++;
				}

				if( bDebug && (nDebug >= 1) )
				{
					printf("\tcreated quantum block %d, dftindex = %d\n", pBlock->block.nDfts, idft);
					int i;

					if( bComplex )
					{
						fftw_complex *pc = (fftw_complex *)pBlock->pdata;
						for(i=0; i<5; i++)
							printf("[%-10.2f %-10.2f] ", pc[idft+i].re, pc[idft+i].im);
					}
					else
					{
						for(i=0; i<10; i++)
							printf("%-10f", pBlock->pdata[idft+i]);
					}

					printf("\n");
				}
				// link onto the sig_blocks chain 
				pBlock->block.nDfts++;
			}

			if( (fPower < nst) && bInSequence )
				bInSequence = FALSE;

			if( (fPower < nst) && bInBlock && bMultisig )
			{
				// end of block
				if( pSig->pFirstBlock == NULL )
				{
					pSig->pFirstBlock = pSig->pLastBlock = pBlock;
					pSig->header.nDftBlocks++;
				}
				else
				{
					pSig->pLastBlock->pnext = pBlock;
					pBlock->plast = pSig->pLastBlock;
					pSig->pLastBlock = pBlock;
					pSig->header.nDftBlocks++;
				}

				if( bDebug && (nDebug >= 0) )
					printf("block ended, dftblocks = %d\n", pSig->header.nDftBlocks);
					
				bInBlock = FALSE;
			}
		}

		
		if( bInBlock )
		{
			// end of block
			if( pSig->pFirstBlock == NULL )
			{
				pSig->pFirstBlock = pSig->pLastBlock = pBlock;
				pSig->header.nDftBlocks++;
			}
			else
			{
				pSig->pLastBlock->pnext = pBlock;
				pBlock->plast = pSig->pLastBlock;
				pSig->pLastBlock = pBlock;
				pSig->header.nDftBlocks++;
			}

			if( bDebug && (nDebug >= 0) )
				printf("block ended, dftblocks = %d\n", pSig->header.nDftBlocks);
					
		}

		free(bbuffer);
	}


	char sigfile[256];

	if( output == NULL )
	{
		char *fnp = strdup(wavefile);
		char *dp = strrchr(fnp, '/');

		if( dp && bInPlace )
		{
			*dp = '\0';
			sprintf(sigfile, "%s/%s.sni", fnp, mid);
		}
		else
			sprintf(sigfile, "%s.sni", mid);
	}

	
	if( bDebug && (nDebug >= 0) )
	{
		printf("creating %s by '%s' [ '%s' '%s' '%s' '%s' nb=%d dfts=%d ]\n", output?output:sigfile, 
				pSig->header.sAuthor, pSig->header.sTag, pSig->header.sRouting, 
				pSig->header.sCallerId, pSig->header.sMessageId, pSig->header.nDftBlocks, ndfts); 
		if( pSig->header.nVectors > 0 )
			printf("with %d feature vectors\n", pSig->header.nVectors);
	}

	if( output )
		pTheEngine->SaveSignature(pSig, output, bDft);
	else
		pTheEngine->SaveSignature(pSig, sigfile, bDft);
}
