#include <stdio.h>
#include <ctype.h>
#include "fftw.h"
#include "rfftw.h"
#include "sni.h"
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
//	sniutt
//
//	simply break a wavefile, based on power, into mini wavefiles, caller utterances 
//	
//	sniutt [-D<n>] -n<blocksize>=400 -t<silence threshold> <wavefile>
//
//	chops the wavefile on silence and puts each piece of audio in a file.
//	<wavefile>.000.wav.
//
//	Then you can throw away what's not relevant.
//	

static char szRcsId[] = "$Id: snicut.cc,v 1.8 2007-03-06 01:59:42 martind Exp $";

BOOL bDebug = FALSE;
int nDebug = 0;

CStaticEngine *pTheEngine;

char logbuf[256];
char uttbuf[256];

double fThreshold = 0.62; // dont change this!
double fMeanFactor = 2.0; // dont change this!
int nQuantum = 10; // dont change this!
BOOL bNumeric = TRUE;
BOOL bTimestamp = FALSE;
BOOL bOldstyle = FALSE;
BOOL bInDigit = FALSE;

char *suffix = NULL;
char *wavefile = NULL;
int nAudioSamPerBlock = 200;
int nPad = 2000;
int nst = 20;	// silence threshold
int nHead = 80000; // 10 secs.
BOOL bHead = FALSE;
int nblank = 0;
int ntail = 0;
BOOL bMultiwav = FALSE;
BOOL bInPlace = FALSE;
BOOL bInvert = FALSE;
char *btoken = NULL;
char *etoken = NULL;
BOOL bb = FALSE, be = FALSE;

char *lib = "."; 

char *options = "?l:B:E:D:n:s:p:h::b:e:Xxt:I";
// -D<debug>
// -n<samplesize>=800
// -s<silence threshold>=100	// minimum power
// -p<padding>=2000
// -h[<head>=10]		- stop processing after <head> seconds of audio.
// -b<secs>|-B<token>,<token>	- blank out <secs> at start of message
// -e<secs>|-E<token<,<token>..	- blank out <secs> at end of message
// -I				- invert bs and be
// -X 		- multiple blocks, by default put all audio in one sigblock.
// -x (in place)
// -l<libdir>	- specify location of signatures.
// -t<suffix>

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
		case '?':
		default:
			printf("snicut -D<n> -s<silence>=200 -n<bsize>=2000 -p<padding>=2000 -h[<head>=10] <wavefile>\n");
			exit(0);

		case 't':
			suffix = strdup(optarg);
			break;

		case 'l':
			lib = strdup(optarg);
			break;

		case 'x':
			bInPlace = TRUE;
			break;

		case 'X':
			bMultiwav = TRUE;
			break;

		case 'I':
			bInvert = TRUE;
			break;

		case 'B':
			btoken = strdup(optarg);
			break;

		case 'E':
			etoken = strdup(optarg);
			break;

		case 'b':
			nblank = (int)(8000.00*atof(optarg));
			break;

		case 'e':
			ntail = (int)(8000.00*atof(optarg));
			break;

		case 'n':
			nAudioSamPerBlock = atoi(optarg);
			break;

		case 's':
			nst = atoi(optarg);
			break;

		case 'p':
			nPad = atoi(optarg);
			break;

		case 'h':
			bHead = TRUE;
			if( optarg )
				nHead = 8000 * atoi(optarg);
			break;

		case 'D':
			bDebug = TRUE;
			nDebug = atoi(optarg);
			break;
		}
	} 

	if( btoken || etoken )
		nAudioSamPerBlock = 2000;

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

	if( cc < 1 )
	{
		fprintf(stderr, "must specify a wave file\n");
		exit(1);
	} 

	if( bDebug && (nDebug >= 1) )
	{
		printf("nst=%d, spb=%d, bDebug = %s, %d\n", nst, nAudioSamPerBlock, bDebug?"on":"off", nDebug);
		// now all the signature files are in cv.
		for(io = 0; io < cc; io++)
			printf("%d %s\n", io, cv[io]);
	}

	pTheEngine = new CStaticEngine(fMeanFactor, fThreshold, nQuantum);

	if( pTheEngine->LoadSignatures(cc, cv, lib) == FALSE )
	{
		fprintf(stderr, "library '%s' not found, exiting\n", lib);
		exit(1);	// this library doesn't exist
	}

	int icc;

	for( icc=0; icc<cc; icc++ )
	{
		struct waveheader whdr;
		struct waveformat wfmt;
		struct datahdr wdata;

		short *bbuffer = NULL;
		int nsamples = 0;
		int ndfts = 0;

		wavefile = cv[icc];

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
			fclose(bfp);
			continue;
		}
		else
		{
			// wave file
			fread(&wfmt, 1, whdr.lFormatLength, bfp);

			if( (wfmt.nChannels != 1) || (wfmt.nSamplesPerSec != 8000) || (wfmt.wBitsPerSample != 16) )
			{
				fprintf(stderr, "unsupported wav format in %s : channels = %d, Samples/s = %d, Bytes/sample = %d\n",
								wavefile, wfmt.nChannels, wfmt.nSamplesPerSec, wfmt.wBitsPerSample/8 );
				fclose(bfp);
				continue;
			}
			else if( bDebug && (nDebug >= 1) )
				printf("scanning %s : channels = %d, Samples/s = %d, Bytes/sample = %d\n",
								wavefile, wfmt.nChannels, wfmt.nSamplesPerSec, wfmt.wBitsPerSample/8 );

			fread(&wdata, 1, sizeof(struct datahdr), bfp);
			nsamples = wdata.lLenData/sizeof(short);

			if( (nsamples-nblank-ntail) < nAudioSamPerBlock )
			{
				printf("wave file too small (too short by %3.1fs)\n", (float)(-(nsamples-nblank-ntail))/8000.0);
				fclose(bfp);
				continue;
			}

			if( nblank > 0 )
			{
				printf("skipping first %3.1fs.\n", (float)(nblank)/8000.00);

				nsamples -= nblank;
				fseek(bfp, nblank*sizeof(short), SEEK_CUR);	// skip over blanked data
			}

			if( ntail > 0 )
			{
				printf("dropping last %3.1fs.\n", (float)(ntail)/8000.00);

				nsamples -= ntail;
			}

			// read all the data in one shot.
			bbuffer = (short *)calloc(nsamples+nPad, sizeof(short));
			fread(bbuffer, nsamples, sizeof(short), bfp);
			fclose(bfp);

			// Now we pre-compute the dft's for file b.
			if( bDebug && (nDebug >= 1) )
				printf("preloaded %d samples (%d blocks)\n", nsamples, nsamples/nAudioSamPerBlock);

			ndfts = nsamples/nAudioSamPerBlock;
			if( nsamples%nAudioSamPerBlock == 0 )
				ndfts--;
		}

		// now we pre-compute the unknown DFT's
		int nblocks = 0;
		int ix;
		BOOL bInUtt = FALSE;
		int nutt = 0;
		int nws = 0;
		FILE *wavfp;
		BOOL bStarted = FALSE;
		BOOL bEnded = FALSE;

		for( ix = 0; ix+nAudioSamPerBlock < nsamples; ix += nAudioSamPerBlock )
		{
			if( bHead && (ix >= nHead) )
				break;

			fftw_real fPower;
			int j;

			for( j=0; j<nAudioSamPerBlock; j++ )
			{
				fPower += pow((fftw_real)(bbuffer[ix+j]),2);
			}

			fPower = sqrt(fPower/nAudioSamPerBlock);

			nblocks++;

			if( fPower >= nst )
			{
				if( bInUtt == FALSE )
				{

					if( bInPlace )
						sprintf(uttbuf, "%s", wavefile);
					else
					{
						char *slp = strrchr(wavefile, '/');

						if( slp )
							sprintf(uttbuf, "%s", slp+1);
						else
							sprintf(uttbuf, "%s", wavefile);
					}

					if( suffix )
						sprintf(uttbuf + strlen(uttbuf)-4, "_%03d_%s.wav", nutt++, suffix);
					else
						sprintf(uttbuf + strlen(uttbuf)-4, "_%03d.wav", nutt++);
					printf("creating %s\n", uttbuf);
					
					nws = 0;
					wavfp = fopen(uttbuf, "w");
					assert(wavfp);

					char *cp;

					cp = (char *)&whdr.lRiff;
					sprintf(cp, "RIFF");
					cp = (char *)&whdr.lWave;
					sprintf(cp, "WAVE");
					cp = (char *)&whdr.lFormat;
					sprintf(cp, "fmt ");

					cp = (char *)&wdata.lType;
					sprintf(cp, "data");

					fwrite(&whdr, 1, sizeof(whdr), wavfp);
					fwrite(&messageFormat, 1, sizeof(messageFormat), wavfp);
					fwrite(&wdata, 1, sizeof(wdata), wavfp);

				}

				// there a number of situations
				//
				//	-B<start>		keep everything after <token>, including token
				//	-E<end>			keep everything before <token>, including token
				//	-B<start> -E<end>	keep everything from <start> to <end>, inclusive,
				//
				//	-I -B<start>		discard everything after <token>, including <start>
				//	-I -E<end>		discard everything before <end>, excluding <end>
				//	-I -B<start> -E<end>	discard everything between <start> and <end> (inclusive)

				if( bInvert )
				{
					if( pTheEngine->MatchesSignature(bbuffer+ix, btoken) )
						bStarted = TRUE;

					if( pTheEngine->MatchesSignature(bbuffer+ix, etoken) )
					{
						if( bEnded == FALSE )
						{
							// if we are at bEnded we skip two buffers to ensure no match.
							if( ix + nAudioSamPerBlock < nsamples ) ix += nAudioSamPerBlock;
							if( ix + nAudioSamPerBlock < nsamples ) ix += nAudioSamPerBlock;
						}
						bEnded = TRUE;
					}

					if( ((btoken && !bStarted) || (btoken == NULL))
						|| ((etoken && bEnded) || (etoken == NULL)) )
					{
						fwrite(bbuffer+ix, sizeof(short), nAudioSamPerBlock, wavfp);
						nws += nAudioSamPerBlock;

						if( bDebug && (nDebug >= 1) )
							printf("+%-3d [%-5d] ", ix/nAudioSamPerBlock, (int) fPower);
					}
				}
				else
				{
					if( pTheEngine->MatchesSignature(bbuffer+ix, btoken) )
						bStarted = TRUE;

					if( pTheEngine->MatchesSignature(bbuffer+ix, etoken) )
					{
						if( bEnded == FALSE )
						{
							fwrite(bbuffer+ix, sizeof(short), nAudioSamPerBlock, wavfp);
							nws += nAudioSamPerBlock;

							if( bDebug && (nDebug >= 1) )
								printf("+%-3d [%-5d] ", ix/nAudioSamPerBlock, (int) fPower);

							if( ix + nAudioSamPerBlock < nsamples )
							{
								ix += nAudioSamPerBlock;

								fwrite(bbuffer+ix, sizeof(short), nAudioSamPerBlock, wavfp);
								nws += nAudioSamPerBlock;

								if( bDebug && (nDebug >= 1) )
									printf("+%-3d [%-5d] ", ix/nAudioSamPerBlock, (int) fPower);
							}
						}
						bEnded = TRUE;
					}

					if( ((btoken && bStarted) || (btoken == NULL))
						&& ((etoken && !bEnded) || (etoken == NULL)) )
					{
						fwrite(bbuffer+ix, sizeof(short), nAudioSamPerBlock, wavfp);
						nws += nAudioSamPerBlock;

						if( bDebug && (nDebug >= 1) )
							printf("+%-3d [%-5d] ", ix/nAudioSamPerBlock, (int) fPower);
					}
				}

				bInUtt = TRUE;
			}
			else
			{
				if( bInUtt && bMultiwav )
				{
					// now we pad out with following audio to nPad.
					int padding = (nPad - nws%nPad)%nPad; // number of samples we need to pad;

					fwrite(bbuffer+ix, sizeof(short), padding, wavfp);
					if( bDebug && (nDebug >= 1) )
					{
						printf("written %d samples, padded to %d (+%d)\n", nws, nPad, padding);
					}
					nws += padding;	

					// close the wave file
					fseek(wavfp, 0, SEEK_SET);
					wdata.lLenData = nws*sizeof(short);
					whdr.lFileSize = sizeof(whdr) + sizeof(messageFormat) + sizeof(wdata) + (nws*sizeof(short));
					whdr.lFormatLength = sizeof(messageFormat);

					fwrite(&whdr, 1, sizeof(whdr), wavfp);
					fwrite(&messageFormat, 1, sizeof(messageFormat), wavfp);
					fwrite(&wdata, 1, sizeof(wdata), wavfp);
					fclose(wavfp);

					bInUtt = FALSE;
					if( bDebug && (nDebug >= 1) )
						printf("\n");
				}
			}
		}

		if( bInUtt == TRUE )
		{
			// now we pad out with following audio to nPad.
			int padding = (nPad - nws%nPad)%nPad; // number of samples we need to pad;

			if( (nws + padding + ix) < nsamples )
			{
				fwrite(bbuffer+ix, sizeof(short), padding, wavfp);
			}
			else
			{
				short *padbuf = (short *)calloc(padding, sizeof(short));

				fwrite(padbuf, sizeof(short), padding, wavfp);
				free(padbuf);
			}

			if( bDebug && (nDebug >= 1) )
			{
				printf("eof written %d samples, padded to %d (+%d)\n", nws, nPad, padding);
			}

			nws += padding;	

			// close the wave file
			fseek(wavfp, 0, SEEK_SET);
			wdata.lLenData = nws*sizeof(short);
			whdr.lFileSize = sizeof(whdr) + sizeof(messageFormat) + sizeof(wdata) + (nws*sizeof(short));
			whdr.lFormatLength = sizeof(messageFormat);

			fwrite(&whdr, 1, sizeof(whdr), wavfp);
			fwrite(&messageFormat, 1, sizeof(messageFormat), wavfp);
			fwrite(&wdata, 1, sizeof(wdata), wavfp);
			fclose(wavfp);

			bInUtt = FALSE;
			if( bDebug && (nDebug >= 1) )
				printf("\n");
		}

		free(bbuffer);
	}
}
