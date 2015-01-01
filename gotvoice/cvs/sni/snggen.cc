#include <stdio.h>
#include <ctype.h>
#include "sni.h"
#include "fftw.h"
#include "rfftw.h"
#include "engine.h"
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
//	snggen -D<n> -o<signature> <wavename>.wav
//	
//	

static char szRcsId[] = "$Id: snggen.cc,v 1.6 2007-03-01 19:57:34 martind Exp $";

BOOL bOldstyle = FALSE;
BOOL bNumeric = FALSE;
BOOL bDebug = FALSE;
int nDebug = 0;
char logbuf[256];

char *btoken = NULL;
char *etoken = NULL;
BOOL be = FALSE;
BOOL bb = FALSE;

// globals
fftw_real fThreshold = 0.47;
fftw_real fMeanFactor = 2.0;
int nQuantum = 10;
char *wavefile = NULL;
char *signature = NULL;
int nAudioSamPerBlock = 2000;

char *options = "?D:n:o:";
// -D<debug>
// -o<signature>=<wavename>
// -n<samplesize>=2000

CStaticEngine *pTheEngine;

BOOL g_save_signature(char *signature, char *filename, int sampleoffset, fftw_real *pDft, char *wavefile = NULL)
{	
	// Real signature file header, used to create file.
	FILE *sngfp;

	assert( signature && pDft && filename );

	printf("creating signature file %s)\n", filename);

	CSignature *pSig = new CSignature;

	// We always set sTag and the filename to m_tokenname
	strcpy( pSig->SigHeader.sHostname, "snggen");
	strcpy( pSig->SigHeader.sTag, signature);
	if( wavefile )
	{
		char *dp = strrchr(wavefile, '/');

		if( dp )
			dp++;
		else
			dp = wavefile;

		sprintf( pSig->SigHeader.sWaveFile, "%s", wavefile);
	}
	pSig->SigHeader.dAudioOffset = sampleoffset;	// exactly halfway
	pSig->SigHeader.nTimeout = 40;
	pSig->SigHeader.fdCorrel = fThreshold;

	sngfp = fopen(filename, "w");

	if( sngfp == NULL )
	{
		fprintf(stderr, "cant create signature file %s\n", filename);
		return FALSE;
	}

	fwrite(&pSig->SigHeader, sizeof(struct newsigheader), 1, sngfp);
	// Write Dft's to File and Close
	fwrite(pDft, sizeof(fftw_real), pTheEngine->nDftSamPerBlock, sngfp);

	fclose(sngfp);

	return TRUE;
}
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
			printf("snggen -n<bsize>=2000 -o<signature>=<wavename> -D<n> <wavename>.wav\n");
			exit(0);
			break;

		case 'n':
			nAudioSamPerBlock = atoi(optarg);
			break;

		case 'o':
			signature = strdup(optarg);
			break;

		case 'D':
			bDebug = TRUE;
			nDebug = atoi(optarg);
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

	if( bDebug && (nDebug <= 0) )
		printf("processing %s\n", wavefile);

	pTheEngine = new CStaticEngine(fMeanFactor, fThreshold, nQuantum, nAudioSamPerBlock);

	struct waveheader whdr;
	struct waveformat wfmt;
	struct datahdr wdata;

	short *bbuffer = NULL;
	int nsamples = 0;

	FILE *bfp = fopen(wavefile, "r");

	if( bfp == NULL )
	{
		fprintf(stderr, "cant open wavefile %s\n", wavefile);
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
							wavefile, wfmt.nChannels, wfmt.nSamplesPerSec, wfmt.wBitsPerSample/8 );
			exit(1);
		}
		else if( bDebug && (nDebug >= 0) )
			printf("scanning %s : channels = %d, Samples/s = %d, Bytes/sample = %d\n",
							wavefile, wfmt.nChannels, wfmt.nSamplesPerSec, wfmt.wBitsPerSample/8 );

		fread(&wdata, 1, sizeof(struct datahdr), bfp);

		nsamples = wdata.lLenData/sizeof(short);

		if( nsamples < nAudioSamPerBlock )
		{
			printf("wave file too small (needed %d, found %d)\n", nAudioSamPerBlock, nsamples);
			fclose(bfp);
			exit(1);
		}

		// read all the data in one shot.
		bbuffer = (short *)malloc(sizeof(short) * nsamples);
		fread(bbuffer, nsamples, sizeof(short), bfp);
		fclose(bfp);

		// Now we pre-compute the dft's for file b.
		if( bDebug && (nDebug >= 0) )
			printf("loaded %d samples (%d blocks)\n", nsamples, nsamples/pTheEngine->nAudioSamPerBlock);
	}

	// now we pre-compute the unknown DFT's
	int ix;
	fftw_real *pDft;

	ix = nsamples/2 - nAudioSamPerBlock/2;
	assert( ix+nAudioSamPerBlock <= nsamples );

	pDft = (fftw_real *)malloc(sizeof(fftw_real)*pTheEngine->nDftSamPerBlock);

	pTheEngine->MakeDft(bbuffer+ix, pDft, NULL);

	char sigfile[256];
	char *dp, *sp;

	if( signature == NULL )
	{
		signature = strdup(wavefile);

		dp = strrchr(signature, '/');
		sp = strrchr(signature, '.');

		if( sp ) *sp = '\0';
		if( dp )
			dp++;
		else
			dp = signature;

		char *newsig = strdup(dp);
		free(signature); signature = newsig;
	}

	char *fnp = strdup(wavefile);
	dp = strrchr(fnp, '/');

	if( dp )
	{
		*dp = '\0';
		sprintf(sigfile, "%s/%s.sng", fnp, signature);
	}
	else
		sprintf(sigfile, "%s.sng", signature);

	
	if( bDebug && (nDebug >= 0) )
	{
		printf("saving(%s, %s, %d, %x, %s)\n", signature, sigfile, ix, pDft, wavefile);
	}
	g_save_signature(signature, sigfile, ix, pDft, wavefile);

	free(bbuffer);
	free(pDft);
}
