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

static char szRcsId[] = "$Id: sngcid.cc,v 1.10 2007-04-27 00:53:57 build Exp $";

//	
//	Basic strategy is simple:
//	
//		we move through the wavefile, using <quantum> as our stepping stone. We create a DFT from every buffer
//		and correlate it with all the signatures specified. If the correlation exceeds the threshold specified, 
//		then diagnostic output is produced.
//
//	there are three ways to run sngcid: default (also called numeric), raw and time. In the default numeric mode 
//	the heuristics are tuned for extraction of phone numbers, in raw mode (-r) only the names of the signatures are printed out,
//	no attempt is made to apply heuristics. In time mode (-T) the heuristics applied are optimzed to date/time extraction.
//
//	sngcid [-l[<basis>]] [-h<chop>=15] [-g<gap>=4] [-D<n>] <wavefile> [<signatures>]
//	sngcid -r [-q<quantum>=10] [-C<correl>=0.62] [-l[<basis>]] [-h<chop>=15] [-g<gap>=4] [-D<n>] <wavefile> [<signatures>]
//	sngcid -T [-l[<basis>]] [-h<chop>=15] [-g<gap>=4] [-D<n>] <wavefile> [<signatures>]
//	
//	-b<token> -e<token>,<token>
//	-X	(extract time/date)
//

extern char lasttag[];
extern int nseen;

int Xlevel = 0;

// globals
int nTimeCut = 15;
double fThreshold = 0.62; // dont change this!
double fMeanFactor = 2.0; // dont change this!
int nQuantum = 10; // dont change this!
BOOL bNumeric = TRUE;
BOOL bTimestamp = FALSE;
BOOL bOldstyle = FALSE;
BOOL bInDigit = FALSE;
BOOL bDebug = FALSE;
int nDebug = 1;
char *lib = "/var/gv/lib";
BOOL bLibrary = FALSE;
int nDigitGap = 4;	// How wide the gap between digits is.

char *wavefile = NULL;
char *signature = "gvscan.sng";

char *options = "gB:E:X::C:D:rl::h:q:H:";

// -H<provider_prefix>

BOOL bHack = FALSE;
char *hack_provider = NULL;
char hack_buffer[1024];

CStaticEngine *pTheEngine;

char *btoken = NULL;
char *etoken = NULL;
BOOL bb = TRUE;
BOOL be = FALSE;

char digitbuffer[50000];
char digitbit[5000];

// parameter gathering
int cc = 0;
char **cv = NULL;

main(int ac, char **av)
{
	int oc;
	digitbuffer[0] = '\0';

	while( (oc = getopt(ac, av, options)) > 0 )
	{
		switch( oc )
		{
		default:
		case '?':
			printf("sngcid -q<quantum>=10 -C<correl>=0.62 -r -l[<basis>] -h<chop>=15 -g<gap>=4 -D<n> <wavefile> [<signatures>]\n");
			printf("sngcid [-B<begin tokens>] [-E<end tokens.] <wavefile> [<signatures>]\n");
			exit(0);
			break;

		case 'q':
			nQuantum = atoi(optarg);
			break;

		case 'B':
			btoken = strdup(optarg);
			bb = FALSE;
			break;

		case 'E':
			etoken = strdup(optarg);
			break;

		case 'H':
			bHack = TRUE;
			hack_provider = optarg;
			break;

		case 'g':	// digit gap in ticks
			if( optarg )
				nDigitGap = atoi(optarg);
			break;

		case 'h':
			nTimeCut = atoi(optarg);
			break;

		case 'C':
			fThreshold = atof(optarg);
			break;
		
		case 'l':
			if( optarg )
				lib = strdup(optarg);

			bLibrary = TRUE;
			break;

		case 'X':
			bNumeric = FALSE;
			bTimestamp = TRUE;

			if( optarg )
				Xlevel = atoi(optarg);

			break;

		case 'r':
			bNumeric = FALSE;
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
	wavefile = strdup(av[optind]);

	cc = 0;
	cv = NULL;

	int io = optind + 1;
	if( io < ac )
	{
		// allocate memory for pointers
		cv = (char **)malloc(sizeof(char *)*(ac-io));
		assert(cv);
	}

	while( io < ac )
	{
		// setup the parameters
		cv[cc++] = av[io++];
	}

	// there must be a library of signatures specified. If the -l option is not used, the -H<provider> must be, in the absence of sng files.
	if( (cc <= 0) && (bLibrary == FALSE) && (bHack == FALSE) )
	{
		fprintf(stderr, "must specify a signature file, library, or the correct hack parameters\n");
		exit(1);
	}

	if( bDebug && (nDebug >= 0) && bHack )
	{
		printf("bHack = %s, hack_provider = %s\n", bHack?"true":"false", hack_provider);
	}

	if( bDebug && (nDebug >= 1) )
	{
		printf("fThreshold = %4.2f, fMeanFactor = %4.2f, nQuantum = %d\n", fThreshold, fMeanFactor, nQuantum);
		printf("wavefile = %s, signature = %s, Debug = %s, %d\n", wavefile, signature, bDebug?"on":"off", nDebug);
		printf("lib = %s, bLibrary = %s, nCutoff = %d\n", lib, bLibrary?"true":"false", nTimeCut);
		// now all the signature files are in cv.
		if( bDebug && (nDebug >= 1) )
			for(io = 0; io < cc; io++)
				fprintf(stderr, "%d %s\n", io, cv[io]);
	}

	pTheEngine = new CStaticEngine(fMeanFactor, fThreshold, nQuantum);

	// Load the Signatures
	if( bLibrary && !bHack )
	{
		if( pTheEngine->LoadSignatures(cc, cv, lib) == FALSE )
		{
			fprintf(stderr, "library '%s' not found, exiting\n", lib);
			exit(1);	// this library doesn't exist
		}
	}
	else if( bHack )
	{
		if( ( strcmp( hack_provider, "attres_LA2" ) == 0 )
		|| ( strcmp( hack_provider, "attres_LA" ) == 0 )
		|| ( strcmp( hack_provider, "attwgsm" ) == 0 )
		|| ( strcmp( hack_provider, "cingular" ) == 0 )
		|| ( strcmp( hack_provider, "cing_vzwla" ) == 0 )
		|| ( strcmp( hack_provider, "cingular_fl" ) == 0 )
		|| ( strcmp( hack_provider, "cingular_79" ) == 0 )
		|| ( strcmp( hack_provider, "tmobile" ) == 0 )
		|| ( strcmp( hack_provider, "tmobile_east" ) == 0 ) )
		{
			nTimeCut = 0;
		}

		sprintf(hack_buffer, "%s/%s", lib, hack_provider);

		if( pTheEngine->LoadSignatures(cc, cv, hack_buffer) == FALSE )
		{
			fprintf(stderr, "library '%s' not found, exiting\n", hack_buffer);
			exit(1);	// this library doesn't exist
		}
	}
	else if( !bLibrary )
	{
		pTheEngine->LoadSignatures(cc, cv);
	}

	//
	//	loop through the audio, comparing with the signatures.
	//
	FILE *ifp = fopen(wavefile, "r");

	if( ifp == NULL )
	{
		fprintf(stderr, "cant open wavefile %s\n", wavefile);
		exit(1);
	}

	struct waveheader whdr;
	struct waveformat wfmt;
	struct datahdr wdata;
	int i;

	fread(&whdr, 1, sizeof(struct waveheader), ifp);

	if( strncmp( (char *)&whdr.lRiff, "RIFF", 4) != 0 )
	{
		fprintf(stderr, "dont support raw files\n");
		exit(1); 
	}
	else
	{
		short sbuffer[pTheEngine->nAudioSamPerBlock];

		// wave file
		fread(&wfmt, 1, whdr.lFormatLength, ifp);

		if( (wfmt.nChannels != 1) || (wfmt.nSamplesPerSec != 8000) || (wfmt.wBitsPerSample != 16) )
		{
			fprintf(stderr, "unsupported wav format in %s : channels = %d, Samples/s = %d, Bytes/sample = %d\n",
							wavefile, wfmt.nChannels, wfmt.nSamplesPerSec, wfmt.wBitsPerSample/8 );
			exit(1);
		}
		else if( bDebug && (nDebug >= 0) )
			printf("scanning %s : channels = %d, Samples/s = %d, Bytes/sample = %d\n",
							wavefile, wfmt.nChannels, wfmt.nSamplesPerSec, wfmt.wBitsPerSample/8 );

		fread(&wdata, 1, sizeof(struct datahdr), ifp);

		int nCutoff = 4*nTimeCut;	// magic number

		while( !be && ((i = fread(sbuffer, sizeof(short), pTheEngine->nAudioSamPerBlock, ifp)) == pTheEngine->nAudioSamPerBlock) )
		{
			digitbit[0] = '\0';
			if( bTimestamp )
				pTheEngine->ScanReport(sbuffer, digitbit, Xlevel);
			else
				pTheEngine->SignatureReport(sbuffer, digitbit);

			if( bNumeric )
			{
				if( bDebug && (nDebug >= 0) )
				{
					printf("%s|", digitbit);
					fflush(stdout);
				}
				strcat(digitbuffer, digitbit);
			}
			else if( bTimestamp )
			{
				printf("%s", digitbit);
			}
			else
			{
				if( strlen(digitbit) > 0 )
					printf("%s\n", digitbit);
				fflush(stdout);
				strcat(digitbuffer, digitbit);
			}

			if( (bTimestamp || bNumeric) && ( --nCutoff <= 0 ) && (nTimeCut > 0) )
				break;
		}

		if( bTimestamp && !be )
		{
			if( nseen > 0 )
				if( Xlevel > 0 )
					printf("%s(%d)\n", lasttag, nseen);
				else
					printf("%s\n", lasttag);
		}
		else if( !bNumeric )
			printf("\n");
	}

	fclose(ifp);

	if( !bNumeric )
		exit(0);

	// Now we walk down digitbuffer and generate the phone number
	char *dcp;

	bInDigit = FALSE;
	int nDigits = 0;
	int nDividers = 0;
	int nWidth = 0;
	char cDigit = '\0';
	char output[64];
	int nid = 0;	// cid length
	int onid = 0;
	int ntotalloops = 0;

	while( (nDigitGap > 0) && (nDigitGap <= 8) && (ntotalloops++ < 8) )
	{
	output[0] = '\0';
	nDividers = 0; nDigits = 0;

	dcp = strrchr(digitbuffer, 'f');

	if( dcp == NULL )
		dcp = digitbuffer;
	else
		dcp++;

	if( bDebug && (nDebug >= 2) )
		printf("dcp = %s\n", dcp);

	for( ; *dcp && (*dcp != 'r'); dcp++ )
	{
		if( bInDigit == FALSE )
		{
			if(isdigit(*dcp))
			{
				bInDigit = TRUE;
				strncat(output, dcp, 1);
				if( bDebug && (nDebug >= 0) )
					printf("%c-", *dcp); fflush(stdout);
				cDigit = *dcp;
				nDividers = 0;
				nDigits++;
				nWidth = 0;
				if( (nDigits == 3) || (nDigits == 6) )
				{
					//printf("-"); fflush(stdout);
				}
			}
		}
		else
		{
			nWidth++;

			// bInDigit == TRUE;
			if( *dcp == '.' )
			{
				if( ++nDividers >= nDigitGap )
					bInDigit = FALSE;
			}
			else if( isdigit(*dcp) && (*dcp != cDigit) )
			{
				bInDigit = TRUE;
				strncat(output, dcp, 1);
				if( bDebug && (nDebug >= 0) )
					printf("%c+", *dcp); fflush(stdout);
				cDigit = *dcp;
				nDividers = 0;
				nDigits++;
				nWidth = 0;
			}

		}
	}

	nid = strlen(output);
	if( bDebug && (nDebug >= 0) )
		printf("%s %d %d\n", output, nid, nDigitGap); 

	if( nid < 10 )
		nDigitGap--;
	else if( nid > 10 )
		nDigitGap++;

	if( (nid == 10) || (!strrchr(digitbuffer, 'f') && (nid == 7)) || (strrchr(digitbuffer, 'f') && (nid == 4)) )
		break;

	}

	if( bNumeric )
	{
		if( bHack )
		{
			hack_buffer[0] = '\0';
			strncat( hack_buffer, output, 10 );
			printf("%s\n", hack_buffer);
		}
		else
		{
			printf("%s\n", output);
		}
		fflush(stdout);
	}

	exit(0);
}
