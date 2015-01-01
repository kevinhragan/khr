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
//	sniview -D<n> <signature>
//	
//	

static char szRcsId[] = "$Id: sniview.cc,v 1.21 2005-04-08 20:55:35 martind Exp $";

BOOL bDebug = FALSE;
int nDebug = 0;
char logbuf[256];

// globals
int nQuantum = 5;
char *snifile = NULL;
char *tag = NULL;
int nAudioSamPerBlock = 400;
int nst = 200;
char *routing = NULL;
char *callerid = NULL;
char *mid = NULL;
BOOL bInPlace = FALSE;
BOOL bInteractive = FALSE;
BOOL bVisual = FALSE;
BOOL bLog = FALSE;
int ilifter = 0;

char *n_routing = NULL;
char *n_cid = NULL;
char *n_tag = NULL;
char *n_author = NULL;
BOOL bWrite = FALSE;
BOOL bOneCom = FALSE;
char *newfile;
char cline[256] = "";
char oline[256];

char *options = "?D:i::vI:R:t:o::A:l";
// -D<debug>
// -R<routing>
// -I<callerid>
// -t<tag>
// -o<newfile>
// -A<author>
// -l	(log)

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
			printf("sniview <signature>.{sni,sng} -D<n> -i [-v]\n");
			exit(0);
			break;

		case 'D':
			bDebug = TRUE;
			nDebug = atoi(optarg);
			break;

		case 'l':
			bLog = TRUE;
			break;

		case 'A':
			n_author = strdup(optarg);
			break;

		case 'i':
			if( optarg )
			{
				bOneCom = TRUE;
				strcpy(cline, optarg);
			}
			else
				bInteractive = TRUE;

			break;

		case 'v':
			bVisual = TRUE;
			break;

		case 'o':
			if( optarg )
				newfile = strdup(optarg);
			else
				newfile = NULL;
			bWrite = TRUE;
			break;

		case 'I':
			n_cid = strdup(optarg);
			break;

		case 't':
			n_tag = strdup(optarg);
			break;

		case 'R':
			n_routing = strdup(optarg);
			break;
		}
	} 

	// next param is snifile, must be present
	if( optind >= ac )
	{
		fprintf(stderr, "no sni file specified\n");
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

	snifile = cv[0];
	if( bWrite && (newfile == NULL) )
		newfile = strdup(snifile);

	pTheEngine = new CSniEngine();
	// we don't use the fft engine at all. we use LoadSignature only.
	// we don't use the fft engine at all. we use LoadSignature only.

	CSniSig *pSig = pTheEngine->LoadSignature(snifile);
	if( pSig == NULL )
	{
		exit(1);
	}

	if( !bInteractive && !bOneCom )
		pTheEngine->PrintSignatureHeader(pSig, snifile);

	if( bWrite )
	{
		printf("creating %s\n", newfile);

		time(&pSig->header.tCreate);

		if( n_tag )
			strcpy(pSig->header.sTag, n_tag);

		if( n_cid )
			strcpy(pSig->header.sCallerId, n_cid);

		if( n_routing )
			strcpy(pSig->header.sRouting, n_routing);

		if( n_author )
			strcpy(pSig->header.sAuthor, n_author);

		pTheEngine->PrintSignatureHeader(pSig, newfile);
		pTheEngine->SaveSignature(pSig, newfile);
		exit(0);
	}

	if( !bInteractive && !bOneCom )
		exit(0);

	// enter interactive mode.
	int iblock = 0;
	int idft = 0;
	int iqdft = 0;
	int nblocks = pSig->header.nDftBlocks;
	sni_block *pb =  pSig->pFirstBlock;

	// this is for live calculations.
	pTheEngine->Initialize(pb->block.nAudioSamPerBlock, pb->block.nQuantum);

	int i;

	if( bInteractive )
	{
		printf("-%% ");
		gets(cline);
	}

	do
	{
		if( strncmp(cline, ".", 1) == 0)
		{
			char bullbuf[256];
			strcpy(cline, oline);
			printf("-%% %s");
			gets(bullbuf);
		}

		if( (strcmp(cline, "quit") == 0) || (cline[0] =='q') )
			break;
		else if( strcmp(cline, "help") == 0)
		{
			printf("b [<n>]	- sets the dft pointer to <n>\n");
			printf("p [<n>]	- print the <n>'th dft, or the next (p alone)\n");
			printf("h	- print the sni header\n");
			printf("v	- set visual mode.\nn	- set numeric mode\n");
		}
		else if( strncmp(cline, "b", 1) == 0)
		{
			// set up dft pointer (default = 0)
			int interim = idft;

			if( (strlen(cline) > 2) && isdigit(cline[2]) )
				interim = atoi(&cline[2]);

			if( (interim >= 0) && (interim < nblocks) )
				iblock = interim;
			else
				printf("dft index out of range\n");

			interim = iblock;

			for( pb = pSig->pFirstBlock; (interim > 0) && pb; pb = pb->pnext, interim--);

			// this is for live calculations.
			pTheEngine->Initialize(pb->block.nAudioSamPerBlock, pb->block.nQuantum);

			printf("iblock = %d\n", iblock);
			printf("dft %d:\tblocks=%dx%d", iblock,  pb->block.nDfts, pb->block.nQuantum);
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
			printf(",%0x , aoffset=%-6d (%4.2f)", pb->block.iFlags, pb->block.iAudioOffset, (float)(pb->block.iAudioOffset)/8000.00);
			printf(", foffset=%d\n", pb->block.iFileOffset);
			idft = 0;
			iqdft = 0;
		}
		else if( strncmp(cline, "0", 1) == 0)
			iqdft = 0;
		else if( strncmp(cline, "+", 1) == 0)
		{
			if( pb && (iqdft >= 0) && (iqdft<(pb->block.nQuantum-1)))
			{
				iqdft++; 
				printf("iqdft = %d\n", iqdft);
			}
			else
				printf("iqdft out of range\n");
		}
		else if( strncmp(cline, "-", 1) == 0)
		{
			if( pb && (iqdft > 0) && (iqdft<pb->block.nQuantum))
			{
				iqdft--; 
				printf("iqdft = %d\n", iqdft);
			}
			else
				printf("iqdft out of range\n");
		}
		else if( strncmp(cline, "k", 1) == 0)
		{
			// calculate and display the peaks
			// as freqencies
			int id, iq, is;
			fftw_real *summary = (fftw_real *)calloc(pb->block.nDftSamPerBlock, sizeof(fftw_real));

			fftw_real ftotal = 0;
			fftw_real fmax = 0;

			for( id=0; id<pb->block.nDfts; id++ )
			{
				for( iq=0; iq<pb->block.nQuantum; iq++)
				{
					int ibase = (id*pb->block.nDftSamPerBlock*pb->block.nQuantum)+(pb->block.nDftSamPerBlock*iq);

					for( is=1; is<pb->block.nDftSamPerBlock; is++)
					{
						if( pb->block.iType == SNIT_PHASE )
						{
							fftw_complex *pc = (fftw_complex *)pb->pdata;

							if( pc[ibase+is].re > fmax )fmax = pc[ibase+is].re;
							summary[is] += pc[ibase+is].re;
							ftotal += pc[ibase+is].re;
						}
						else
						{
							if( pb->pdata[ibase+is] > fmax )fmax = pb->pdata[ibase+is];
							summary[is] += pb->pdata[ibase+is];
							ftotal += pb->pdata[ibase+is];
						}
					}
				}
			}


			ftotal /= pb->block.nDfts*pb->block.nQuantum;
			fftw_real fmean = ftotal/100;

			fftw_real smooth[100];
			char graph[20][100];

			int row, col;

			for( col=0; col<100; col++)
				smooth[col] = 0;

                        fftw_real lastf = 0, thisf;
                        fftw_real lastdelta = 0;

                        for(is = 1; is < pb->block.nDftSamPerBlock; is++ )
                        {
                                int index = 100*(is-1)/(pb->block.nDftSamPerBlock-1);
                                fftw_real thisdelta;

                                thisf = summary[is]/(pb->block.nDfts*pb->block.nQuantum);
                                thisdelta = thisf - lastf;

                                if( (thisdelta<=0) && (lastdelta>=0) && (lastf>1.0) )
                                        printf("+[%d,%4.2f] ", (is*4000)/pb->block.nDftSamPerBlock, lastf);

				if( (thisdelta>=0) && (lastdelta<=0) && (lastf>1.0) )
                                        printf("-[%d,%4.2f] ", (is*4000)/pb->block.nDftSamPerBlock, lastf);

                                smooth[index] += thisf/fmean;

				lastf = thisf;
				lastdelta = thisdelta;
                        }

			printf("\n");

			for( col=0; col<100; col++)
			{
				for( row=0; row<20; row++)
				{
					int val;

					val = (int)((2*fmean*smooth[col]));

					if( row < val )
						graph[row][col] = '#';
					else
						graph[row][col] = ' ';
				}
			}

			for( row=19; row>=0; row--)
			{
				for( col=0; col<100; col++)
					printf("%c", graph[row][col]);
				printf("\n");
			}

			float fhi, flo;

			fhi = 8000.00/2;
			flo = 8000/(2*(float)(pb->block.nDftSamPerBlock));
			printf("%-4.0f                    |                        |                        |%25.0f\n", flo, fhi);
			printf("ftotal=%4.1f fmax=%4.1f fmean=%4.1f nDSPB = %d\n", ftotal, fmax, fmean,  pb->block.nDftSamPerBlock);

			free(summary);
		}
		else if( strncmp(cline, "s", 1) == 0)
		{
			// summary
			int id, iq, is;
			fftw_real smooth[100];
			char graph[20][100];

			int row, col;

			for( col=0; col<100; col++)
				smooth[col] = 0;

			fftw_real ftotal = 0;
			fftw_real fmax = 0;
			fftw_real fmean = 0;

			for( id=0; id<pb->block.nDfts; id++ )
			{
				for( iq=0; iq<pb->block.nQuantum; iq++)
				{
					int ibase = (id*pb->block.nDftSamPerBlock*pb->block.nQuantum)+(pb->block.nDftSamPerBlock*iq);

					for( is=1; is<pb->block.nDftSamPerBlock; is++)
					{
						int index = 100*(is-1)/(pb->block.nDftSamPerBlock-1);
						if( pb->block.iType == SNIT_PHASE )
						{
							fftw_complex *pc = (fftw_complex *)pb->pdata;

							if( pc[ibase+is].re > fmax )fmax = pc[ibase+is].re;
							smooth[index] += pc[ibase+is].re;
							ftotal += pc[ibase+is].re;
						}
						else
						{
							if( pb->pdata[ibase+is] > fmax )fmax = pb->pdata[ibase+is];
							smooth[index] += pb->pdata[ibase+is];
							ftotal += pb->pdata[ibase+is];
						}
					}
				}
			}


			ftotal /= pb->block.nDfts*pb->block.nQuantum;
			fmean = ftotal/100;

			printf("ftotal = %4.1f(%4.1f, %4.1f) nDSPB = %d\n", ftotal, fmax, fmean,  pb->block.nDftSamPerBlock);

			for( col=0; col<100; col++)
			{
				for( row=0; row<20; row++)
				{
					int val;

					val = (int)(20*(smooth[col]/(pb->block.nDfts*pb->block.nQuantum))/(fmean*5));

					if( bLog )
					{
						if( row-5 < 5*log(val) )
							graph[row][col] = '#';
						else
							graph[row][col] = ' ';
					}
					else
					{
						if( row < val )
							graph[row][col] = '#';
						else
							graph[row][col] = ' ';
					}
				}
			}

			for( row=19; row>=0; row--)
			{
				for( col=0; col<100; col++)
					printf("%c", graph[row][col]);
				printf("\n");
			}

			float fhi, flo;

			fhi = 8000.00/2;
			flo = 8000/(2*(float)(pb->block.nDftSamPerBlock));
			printf("%-4.0f                    |                        |                        |%25.0f\n", flo, fhi);
		}
		else if( strncmp(cline, "p", 1) == 0)
		{
			int interim = idft;

			if( cline[1] == '+' )
				interim = idft+1;
			else if( cline[1] == '-' )
				interim = idft-1;
			else if( (strlen(cline) > 2) && isdigit(cline[2]) )
				interim = atoi(&cline[2]);

			if( (interim >= 0) && pb && (interim < pb->block.nDfts) )
				idft = interim;
			else
				printf("dft index out of range\n");

			printf("[%d, %d, %d]\n", iblock, idft, iqdft);

			int ibase = (idft*pb->block.nDftSamPerBlock*pb->block.nQuantum)+(pb->block.nDftSamPerBlock*iqdft);
			printf("dftindex = %d\n", ibase);

			if( bVisual )
			{
				char graph[20][100];

				int row, col;

				fftw_real smooth[100];

				for( col=0; col<100; col++)
					smooth[col] = 0;

				fftw_real ftotal = 0;

				for( i=1; i<pb->block.nDftSamPerBlock; i++)
				{
					int index = 100*(i-1)/(pb->block.nDftSamPerBlock-1);

					if( pb->block.iType == SNIT_PHASE )
					{
						fftw_complex *pc = (fftw_complex *)pb->pdata;

						smooth[index] += pc[ibase+i].re;
						ftotal += pc[ibase+i].re;
					}
					else
					{
						smooth[index] += pb->pdata[ibase+i];
						ftotal += pb->pdata[ibase+i];
					}
				}

				printf("ftotal = %4.1f nDSPB = %d\n", ftotal, pb->block.nDftSamPerBlock);

				for( col=0; col<100; col++)
					for( row=0; row<20; row++)
					{
						if( bLog )
						{
							if( row-7 <= 5*log(smooth[col]*(1001/pb->block.nDftSamPerBlock)/2) )
								graph[row][col] = '#';
							else
								graph[row][col] = ' ';
						}
						else
						{
							if( row <= smooth[col]*(1001/pb->block.nDftSamPerBlock)/2 )
								graph[row][col] = '#';
							else
								graph[row][col] = ' ';
						}
					}

				for( row=19; row>=0; row--)
				{
					for( col=0; col<100; col++)
						printf("%c", graph[row][col]);
					printf("\n");
				}

				float fhi, flo;

				fhi = 8000.00/2;
				flo = 8000/(2*(float)(pb->block.nAudioSamPerBlock));
				printf("%-4.0f                    |                        |                        |%25.0f\n", flo, fhi);
						
			}
			else
			{
				int col;
				// numeric
				fftw_real smooth[100];

				for( col=0; col<100; col++)
					smooth[col] = 0;

				for( i=1; i<pb->block.nDftSamPerBlock; i++)
				{
					int index = 100*(i-1)/(pb->block.nDftSamPerBlock-1);
					if( pb->block.iType == SNIT_PHASE )
					{
						fftw_complex *pc = (fftw_complex *)pb->pdata;

						smooth[index] += pc[ibase+i].re;
					}
					else
						smooth[index] += pb->pdata[ibase+i];
				}

				if( pb->block.iType == SNIT_PHASE )
				{
					fftw_complex *pc = (fftw_complex *)pb->pdata;

					printf("-- raw...\n");
					for( col=0; col<50; col++)
					{
						printf("[%-10f %4.2f] ", pc[col+ibase].re, pc[col+ibase].im);
						if( ((col+1)%5 ==0) )
							printf("\n");
					}
					printf("-- smoothed...\n");

					for( col=0; col<100; col++)
					{
						printf("%-10f", smooth[col]);
						if( ((col+1)%10 ==0) )
							printf("\n");
					}
				}
				else
				{
					printf("-- raw...\n");
					for( col=0; col<100; col++)
					{
						printf("%-10f", pb->pdata[col+ibase]);
						if( ((col+1)%10 ==0) )
							printf("\n");
					}
					printf("-- smoothed...\n");

					for( col=0; col<100; col++)
					{
						printf("%-10f", smooth[col]);
						if( ((col+1)%10 ==0) )
							printf("\n");
					}
				}
			}
		}
		else if( strcmp(cline, "v") == 0)
			bVisual = TRUE;
		else if( (strncmp(cline, "i", 1) == 0) && isdigit(cline[2]) )
			ilifter = atoi(&cline[2]);
		else if( (strncmp(cline, "I", 1) == 0) && isdigit(cline[2]) )
			ilifter = -atoi(&cline[2]);
		else if( strcmp(cline, "n") == 0)
			bVisual = FALSE;
		else if( strcmp(cline, "l") == 0)
		{
			if( bLog )
			{
				printf("logdisplay off\n");
				bLog = FALSE;
			}
			else
			{
				printf("logdisplay on\n");
				bLog = TRUE;
			}
		}
		else if( strncmp(cline, "c", 1) == 0)
		{
			// we calculate a visualize the cepstrum of the current spectrum
			int interim = idft;

			if( cline[1] == '+' )
				interim = idft+1;
			else if( cline[1] == '-' )
				interim = idft-1;
			else if( (strlen(cline) > 2) && isdigit(cline[2]) )
				interim = atoi(&cline[2]);

			if( (interim >= 0) && pb && (interim < pb->block.nDfts) )
				idft = interim;
			else
				printf("dft index out of range\n");

			printf("[%d, %d, %d]\n", iblock, idft, iqdft);

			int ibase = (idft*pb->block.nDftSamPerBlock*pb->block.nQuantum)+(pb->block.nDftSamPerBlock*iqdft);
			printf("dftindex = %d\n", ibase);

			// everything is half complex.
			fftw_real *pData = (fftw_real *)calloc(pb->block.nAudioSamPerBlock+1, sizeof(fftw_real));
			fftw_real *pOut = (fftw_real *)calloc(pb->block.nAudioSamPerBlock+1, sizeof(fftw_real));
			fftw_real *pCep = (fftw_real *)calloc(pb->block.nAudioSamPerBlock+1, sizeof(fftw_real));

			pData[0] = 0;
			pData[pb->block.nDftSamPerBlock/2] = 0;

			// now we set up the data. as a half-complex array.
			for( i=1; i<pb->block.nDftSamPerBlock; i++)
			{
				if( pb->block.iType == SNIT_PHASE )
				{
					fftw_complex *pc = (fftw_complex *)pb->pdata;

					// in the phase data format the data in already in |z|, arg(z) format!
					// log(z) = log(|z|) + j*arg(z)
					pData[i] = log(pc[ibase+i].re);
					pData[pb->block.nAudioSamPerBlock-i] = 2*M_PI*(pc[ibase+i].im/360);
				}
				else
				{
					pData[i] = log(pb->pdata[ibase+i]);
					pData[pb->block.nAudioSamPerBlock-i] = 0;
				}
			}

			if( bDebug && (nDebug >= 0) )
			{
				printf("log(S(w))");
				for( i=1; i<=10; i++ )
					printf("[%4.2f,%4.2f]", pData[i], pData[pb->block.nAudioSamPerBlock-i]);
				printf("\n");
			}

			// compute the cepstrum, we end-up with pb->block.nAudioSamPerBlock 'real' samples.
			rfftw_one(pTheEngine->theReversePlan, pData, pCep);

			int nspec = pb->block.nAudioSamPerBlock;

			char graph[20][100];

			int row, col;

			fftw_real smooth[100];

			for( col=0; col<100; col++)
				smooth[col] = 0;

			fftw_real ftotal = 0, fmean;

			for( i=1; i<nspec; i++)
			{
				int index = 100*(i-1)/(nspec-1);

				smooth[index] += sqrt(pCep[i]*pCep[i]);
				ftotal += sqrt(pCep[i]*pCep[i]);
			}

			fmean = ftotal/100;

			printf("ftotal = %4.1f nCepDSPB = %d\n", ftotal, nspec);

			for( col=0; col<100; col++)
			{
				for( row=0; row<20; row++)
				{
					if( bLog )
					{
						if( (smooth[col] > 0) && ( row-5 <= 5*log(smooth[col]) ) )
							graph[row][col] = '#';
						else
							graph[row][col] = ' ';
					}
					else
					{
						if( row/4 <= 100*(smooth[col]/ftotal) )
							graph[row][col] = '#';
						else
							graph[row][col] = ' ';
					}
				}
			}

			for( row=19; row>=0; row--)
			{
				for( col=0; col<100; col++)
					printf("%c", graph[row][col]);
				printf("\n");
			}

			float fhi, flo;

			fhi = 8000.00/2;
			flo = 8000/(2*(float)(pb->block.nAudioSamPerBlock));
			printf("%-4.0f                    |                        |                        |%25.0f\n", flo, fhi);
			free(pData); free(pOut); free(pCep);
		}
		else if( strncmp(cline, "V", 1) == 0)
		{
			CFeatureVector *pfv;
			int ib, iv = 0;
			int plot[101];

			for( iv=0; iv<101; iv++) plot[iv] = 0;

			// print out all the feature vectors
			for( iv=0, pfv = pSig->pFirstFV; pfv; pfv = pfv->pnext )
			{
				if( !bVisual )
					printf("f%d %-3d %4.0f ", iv, pfv->iSampleOffset, pfv->fEnergy);
				for( i=0; i<pfv->nFormant; i++ )
				{
					if( !bVisual )
						printf("[%4.0f %4.2f]", pfv->pFormant[i].fcenter, pfv->pFormant[i].fpower);
					plot[(int)(pfv->pFormant[i].fcenter)/40]++;
				}

				if( !bVisual )
					printf("\np%d %-3d %4.0f ", iv, pfv->iSampleOffset, pfv->fEnergy);

				for( i=0; i<pfv->nPitch; i++ )
					if( !bVisual )
						printf("{%4.0f %4.2f}", pfv->pPitch[i].fcenter, pfv->pPitch[i].fpower);

				if( !bVisual )
					printf("\n");
				iv++;
			}

			if( bVisual )
			{
				int row, col;

				for( row=0; row<20; row++)
				{
					for( col=0; col<100; col++ )
						if( (20-row) <= plot[col] )
							printf("|");
						else
							printf(" ");

					printf("\n");
				}

				float fhi, flo;

				fhi = 8000.00/2;
				flo = 8000/(2*(float)(pb->block.nDftSamPerBlock));
				printf("%-4.0f                    |                        |                        |%25.0f\n", flo, fhi);
			}
		}
		else if( strncmp(cline, "Q", 1) == 0)
		{
			CFeatureVector *pfv;
			int ib, iv = 0;
			int bigplot[25][25];
			int bigtotal = 0;
			int nfreq = 0;

			for( iv=0; iv<25; iv++)
				for( ib=0; ib<25; ib++) 
					bigplot[iv][ib] = 0;

			// print out all the feature vectors
			for( iv=0, pfv = pSig->pFirstFV; pfv; pfv = pfv->pnext )
			{
				for( i=0; i<pfv->nFormant; i++ )
				{
					nfreq++;
					for( ib=0; ib<pfv->nFormant; ib++)
					{
						if( ib == i ) continue;
						bigplot[(int)(pfv->pFormant[i].fcenter)/160][(int)(pfv->pFormant[ib].fcenter)/160]++;
						bigtotal++;
					}
				}
			}

			printf("formant p-matrix for %d formant frequencies\n", nfreq);
			for( iv=0; iv<25; iv++) 
			{
				int ntotal = 0;

				for( ib=0; ib<25; ib++) 
					ntotal += bigplot[iv][ib];

				for( ib=0; ib<25; ib++) 
					if( ntotal <= 0 )
						printf("%03d ", bigplot[iv][ib]);
					else
						printf("%03d ", (1000*bigplot[iv][ib])/ntotal);
				printf("| %03d\n", (1000*ntotal)/bigtotal);	
			}

		}
		else if( strncmp(cline, "X", 1) == 0)
		{
			CFeatureVector *pfv;
			int ib, iv = 0;
			int bigplot[25][25];
			int bigtotal = 0;

			for( iv=0; iv<25; iv++)
				for( ib=0; ib<25; ib++) 
					bigplot[iv][ib] = 0;

			// print out all the feature vectors
			for( iv=0, pfv = pSig->pFirstFV; pfv; pfv = pfv->pnext )
			{
				for( i=0; i<pfv->nPitch; i++ )
				{
					for( ib=0; ib<pfv->nPitch; ib++)
					{
						if( ib == i ) continue;
						bigplot[(int)(pfv->pPitch[i].fcenter)/160][(int)(pfv->pPitch[ib].fcenter)/160]++;
						bigtotal++;
					}
				}
			}

			printf("pitch p-matrix\n");
			for( iv=0; iv<25; iv++) 
			{
				int ntotal = 0;

				for( ib=0; ib<25; ib++) 
					ntotal += bigplot[iv][ib];

				for( ib=0; ib<25; ib++) 
					if( ntotal <= 0 )
						printf("%03d ", bigplot[iv][ib]);
					else
						printf("%03d ", (1000*bigplot[iv][ib])/ntotal);
				printf("| %03d\n", (1000*ntotal)/bigtotal);	
			}

		}
		else if( strncmp(cline, "P", 1) == 0)
		{
			CFeatureVector *pfv;
			int iv = 0;
			int plot[101];

			for( iv=0; iv<101; iv++) plot[iv] = 0;

			// print out all the feature vectors
			for( iv=0, pfv = pSig->pFirstFV; pfv; pfv = pfv->pnext )
			{
				for( i=0; i<pfv->nPitch; i++ )
					plot[(int)(pfv->pPitch[i].fcenter)/40]++;

				iv++;
			}

			for( iv=0; iv<100; iv++) 
				printf("%d ", plot[iv]);
			printf("\n");	

			int row, col;

			for( row=0; row<20; row++)
			{
				for( col=0; col<100; col++ )
					if( (20-row) <= plot[col] )
						printf("|");
					else
						printf(" ");

				printf("\n");
			}

			float fhi, flo;

			fhi = 8000.00/2;
			flo = 8000/(2*(float)(pb->block.nDftSamPerBlock));
			printf("%-4.0f                    |                        |                        |%25.0f\n", flo, fhi);
		}
		else if( strncmp(cline, "F", 1) == 0)
		{
			// we calculate a visualize the cepstrum of the current spectrum
			int interim = idft;

			if( cline[1] == '+' )
				interim = idft+1;
			else if( cline[1] == '-' )
				interim = idft-1;
			else if( (strlen(cline) > 2) && isdigit(cline[2]) )
				interim = atoi(&cline[2]);

			if( (interim >= 0) && pb && (interim < pb->block.nDfts) )
				idft = interim;
			else
				printf("dft index out of range\n");

			printf("[%d, %d, %d]\n", iblock, idft, iqdft);
			int ibase = (idft*pb->block.nDftSamPerBlock*pb->block.nQuantum)+(pb->block.nDftSamPerBlock*iqdft);
			printf("dftindex = %d\n", ibase);

			fftw_complex *pc = (fftw_complex *)pb->pdata;

			// CFeatureVector *presult = pTheEngine->ComputeFeatureVector(&pc[ibase], 20);
			CFeatureVector *presult = pTheEngine->ComputeDftPeaks(&pc[ibase]);

			if( presult->nFormant > 0 )
			{
				// calculate and printout the formant table
				int ifm, jfm;

				printf("multiples:\n  x  ");
				for( ifm=0; ifm<presult->nFormant; ifm++ )
					printf("  F%d  ", ifm);
				
				printf("\n     ");

				for( ifm=0; ifm<presult->nFormant; ifm++ )
					printf(" %-4.0f ", presult->pFormant[ifm].fcenter);
				
				printf("\n     ");

				for( ifm=0; ifm<presult->nFormant; ifm++ )
					printf(" %-4.1f ", presult->pFormant[ifm].fpower);

				fftw_real fundemental[presult->nFormant], likelyfund;
				int period[presult->nFormant];
				int nfund = 0;
				
				for( ifm=0; ifm<presult->nFormant; ifm++ )
				{
					int periodicity = 0;
					BOOL bFund = FALSE;

					printf("\nF%02d  ", ifm);

					for( jfm=0; jfm<presult->nFormant; jfm++ )
					{
						fftw_real fratio = presult->pFormant[jfm].fcenter/presult->pFormant[ifm].fcenter;

						if( (fratio > 1.5) && (ifm != jfm) )
						{
							fftw_real above = ceil(fratio);
							fftw_real below = floor(fratio);
							fftw_real fmin, fdiff;

							if( fabs(fratio-above) > fabs(fratio-below) )
							{
								fmin = fabs(fratio-below);
								fdiff = fmin/below;
							}
							else
							{
								fmin = fabs(fratio-above);
								fdiff = fmin/above;
							}

							if( fmin <= 0.085 )
							{
								fundemental[nfund] = presult->pFormant[ifm].fcenter;
								periodicity++;
								bFund = TRUE;
							}
						}

						// if fratio is nearly an integer, we add it.
						printf(" %4.2f ", fratio);
					}
					printf("%-2d %3.2f", periodicity, (float)(periodicity)/(presult->nFormant-(ifm+1)));
					if( periodicity>0 )
						printf(" %4.0f", fundemental[nfund]);
					if( bFund )
						period[nfund++] = periodicity;

				}

				int fundper;

				switch( nfund )
				{
				case 0:
					printf("\n");
					break;

				case 1:
					if( period[0] > 1 )
						printf("\n%4.0f %d %3.2f\n", fundemental[0], period[0], 
										(float)(fundper)/(presult->nFormant-(ifm+1)));
					else
						printf("\n");
					break;

				case 2:
				default:
					if( (period[0] > 1) &&  (period[1] > 1) && isperiodic(fundemental[0], fundemental[1]) )
					{
						fundper = period[0];
						likelyfund = fundemental[0];
						// print fundemental[0] 
					}
					else
					{
						fundper = period[1];
						likelyfund = fundemental[1];
						// print fundemental[1]
					}
					printf("\n%4.0f %d %3.2f\n", likelyfund, fundper, (float)(fundper)/(presult->nFormant-(ifm+1)));
					break;
				}
			}

			for( i=0; i<presult->nPitch; i++ )
				printf("P%d %4.1f %4.2fdB| ", i+1, presult->pPitch[i].fcenter, presult->pPitch[i].fpower);

			printf("\n");

		}
		else if( strncmp(cline, "f", 1) == 0)
		{
			// we calculate a visualize the cepstrum of the current spectrum
			int interim = idft;

			if( cline[1] == '+' )
				interim = idft+1;
			else if( cline[1] == '-' )
				interim = idft-1;
			else if( (strlen(cline) > 2) && isdigit(cline[2]) )
				interim = atoi(&cline[2]);

			if( (interim >= 0) && pb && (interim < pb->block.nDfts) )
				idft = interim;
			else
				printf("dft index out of range\n");

			printf("[%d, %d, %d]\n", iblock, idft, iqdft);

			int ibase = (idft*pb->block.nDftSamPerBlock*pb->block.nQuantum)+(pb->block.nDftSamPerBlock*iqdft);
			printf("dftindex = %d\n", ibase);

			// everything is half complex.
			fftw_real *pData = (fftw_real *)calloc(pb->block.nAudioSamPerBlock+1, sizeof(fftw_real));
			fftw_real *pOut = (fftw_real *)calloc(pb->block.nAudioSamPerBlock+1, sizeof(fftw_real));
			fftw_real *pCep = (fftw_real *)calloc(pb->block.nAudioSamPerBlock+1, sizeof(fftw_real));

			pData[0] = 0;
			pData[pb->block.nDftSamPerBlock/2] = 0;

			// now we set up the data. as a half-complex array.
			for( i=1; i<pb->block.nDftSamPerBlock; i++)
			{
				if( pb->block.iType == SNIT_PHASE )
				{
					fftw_complex *pc = (fftw_complex *)pb->pdata;

					// in the phase data format the data in already in |z|, arg(z) format!
					// log(z) = log(|z|) + j*arg(z)
					pData[i] = log(pc[ibase+i].re);
					pData[pb->block.nAudioSamPerBlock-i] = 2*M_PI*(pc[ibase+i].im/360);
				}
				else
				{
					pData[i] = log(pb->pdata[ibase+i]);
					pData[pb->block.nAudioSamPerBlock-i] = 0;
				}
			}

			if( bDebug && (nDebug >= 0) )
			{
				printf("log(S(w))");
				for( i=1; i<=10; i++ )
					printf("[%4.2f,%4.2f]", pData[i], pData[pb->block.nAudioSamPerBlock-i]);
				printf("\n");
			}

			// compute the cepstrum, we end-up with pb->block.nAudioSamPerBlock 'real' samples.
			rfftw_one(pTheEngine->theReversePlan, pData, pCep);

			int nspec = pb->block.nAudioSamPerBlock;
			sni_formant formants[nspec/2];	// overkill

			// if we get here, we perform a homo-morphic decomposition (liftering) of the 
			// excitation and the formants.
			if( bDebug && (nDebug >= 0) )
			{
				printf("F*(log(S(w)))");
				for( i=1; i<=10; i++ )
					printf("[%4.2f]", pCep[i]);
				printf("\n");
			}
			// apply liftering;
			if( ilifter != 0 )
			{
				BOOL bReverse = FALSE;

				if( ilifter < 0 )
				{
					bReverse = TRUE;
					ilifter = -ilifter;
				}

				if( ilifter >= pb->block.nDftSamPerBlock )
				{
					printf("ilifter out of range (%d >= %d)\n", ilifter, pb->block.nDftSamPerBlock);
				}
				else
				{
					printf("applying %s lifter = %d to %d\n", 
							bReverse?"reverse":"forward", ilifter, pb->block.nDftSamPerBlock);
				}

				// pCep now contains the complex cepstrum (real data), symmetric about the middle
				// of the buffer. 
				//
				// we provide two different masks, which can be applied to the cepstrum to achieve
				// de-convolution of U(w) (the excitation), from V(w)the formant envelope.
				// 
				if( bReverse )
				{
					// mask the V(w) to reveal U(w).
					for( i=1; i<ilifter; i++ )
						pCep[i] = pCep[pb->block.nAudioSamPerBlock-i] = 0;

					ilifter = -ilifter; 	// back again
				}
				else
				{
					// mask U(w) to reveal V(w)
					for( i=ilifter; i<pb->block.nDftSamPerBlock; i++ )
						pCep[i] = pCep[pb->block.nAudioSamPerBlock-i] = 0;
				}

			}

			// now reverse the process
			rfftw_one(pTheEngine->theForwardPlan, pCep, pData);

			pData[0] /= pb->block.nAudioSamPerBlock;
			for( i=1; i<pb->block.nDftSamPerBlock; i++ )
			{
				pData[i] /= pb->block.nAudioSamPerBlock;
				pData[pb->block.nAudioSamPerBlock-i] /= pb->block.nAudioSamPerBlock;
			}

			if( bDebug && (nDebug >= 0) )
			{
				printf("F(F*(log(S(w))))");
				for( i=1; i<=10; i++ )
					printf("[%4.2f,%4.2f]", pData[i], pData[pb->block.nAudioSamPerBlock-i]);
				printf("\n");
			}

			// now we display the liftered log spectrum, which is now in half-complex form.
			if( ilifter != 0 )
			{
				fftw_real dldt = 0.0;
				fftw_real flast = pData[1];
				BOOL bInPeak = FALSE;
				fftw_real fpower, fmax;
				int istart, iend, imax;
				int iformant = 0;

				// find the formants, by looking at the log(|z|) values. In the first half
				// of the half-complex array (see fftw 2.15 docs).
				//
				for( i=3; i<nspec/2-2; i++)
				{
					fftw_real fthis = (pData[i]+pData[i-1]+pData[i+1]+pData[i-2]+pData[i+2])/5;
					fftw_real fdelta = fthis - flast;

					if( bDebug && (nDebug >= 1) )
						printf("%.3f:%.3f, ", fthis, fdelta);

					if( bInPeak )
					{
						fpower += fthis;
						if( fthis >= fmax )
						{
							fmax = fthis;
							imax = i;
						}
					}

					if( bDebug && (nDebug >= 0) )
						printf("%c",  fdelta>0?'+':'-');

					if( bInPeak && (fdelta < 0) && (dldt < 0) )
					{
						bInPeak = FALSE;
						iend = i;

						fftw_real flo, fhi, fcenter;
						fftw_real fstep = 4000.0/(nspec/2); // Hz per data index

						flo = fstep*istart;
						fhi = fstep*iend;
						fcenter = flo + (fhi-flo)/2;

						if( (fmax > 0) && (ilifter >= 0) )// just passed maximum
						{
							formants[iformant].fcenter = fstep*imax;
							formants[iformant].fpower = fmax;

							printf("F%d[%.1fHz %4.2fdB]\n", iformant, fstep*imax, fmax);
							iformant++;
						}
						else if( fmax > 0 ) // remember its a log.
						{
							formants[iformant].fcenter = fstep*imax;
							formants[iformant].fpower = fmax;

							printf("F%d[%.1fHz %4.2fdB]\n", iformant, fstep*imax, fmax);
							iformant++;
						}

					}
					else if( (fdelta < 0) && (dldt > 0) && (i>2) )
					{
						if( bInPeak == FALSE )
						{
							fmax = fpower = flast;
							imax = istart = i-1;
							if( bDebug && (nDebug >= 0) )
								printf("|");
							bInPeak = TRUE;
						}
					}

					flast = fthis;
					dldt = fdelta;
				}
				if( bDebug && (nDebug >= 0) )
					printf("\n");

				if( iformant > 0 )
				{
					// calculate and printout the formant table
					int ifm, jfm;

					printf("multiples:\n  x  ");
					for( ifm=0; ifm<iformant; ifm++ )
						printf("  F%d  ", ifm);
					
					printf("\n     ");

					for( ifm=0; ifm<iformant; ifm++ )
						printf(" %-4.0f ", formants[ifm].fcenter);
					
					for( ifm=0; ifm<iformant; ifm++ )
					{
						printf("\nF%02d  ", ifm);

						for( jfm=0; jfm<iformant; jfm++ )
						{
							printf(" %4.2f ", formants[jfm].fcenter/formants[ifm].fcenter);
						}
					}
					printf("\n");
				}
			}

			if( bVisual )
			{
				int row, col;
				fftw_real smooth[100];
				char graph[20][100];

				for( col=0; col<100; col++)
					smooth[col] = 0;

				for( i=1; i<nspec/2; i++)
				{
					int index = (100*(i-1))/(nspec/2);

					smooth[index] += pData[i];
				}


				fftw_real top, bot;

				for( col=0; col<100; col++)
				{
					smooth[col] /= (float)(nspec/2)/100.0;
					if( !bLog )
						smooth[col] = exp(smooth[col]);
				}

				printf("nCepDSPB = %d\n", nspec);

				top = bot = smooth[0];
				for( col=0; col<100; col++)
				{
					if( smooth[col] < bot )
						bot = smooth[col];

					if( smooth[col] > top )
						top = smooth[col];
				}

				// we must divide 20 by the range.
				double fstep = (top-bot)/20;

				if( bDebug && (nDebug >= 0) )
					printf("top=%f, bot=%f, fstep=%f\n", top, bot, fstep);

				for( col=0; col<100; col++)
				{
					for( row=0; row<20; row++)
					{
						double somuch = row*fstep + bot;

						//printf("somuch=%f, smooth=%f\n", somuch, smooth[col]);

						if( somuch <= smooth[col] )
							graph[row][col] = '#';
						else
							graph[row][col] = ' ';
					}
				}

				for( row=19; row>=0; row--)
				{
					for( col=0; col<100; col++)
						printf("%c", graph[row][col]);
					printf("\n");
				}
	
				float fhi, flo;

				fhi = 8000.00/2;
				flo = 8000/(2*(float)(nspec));
				printf("%-4.0f                    |                        |                        |%25.0f\n", flo, fhi);

				if( bDebug && (nDebug >= 0) )
					for( col=0; col<100; col++)
					{
						printf("%-10f", smooth[col]);
						if( ((col+1)%10 ==0) )
							printf("\n");
					}

			}

			free(pData); free(pOut); free(pCep);
		}
		else if( strcmp(cline, "h") == 0)
			pTheEngine->PrintSignatureHeader(pSig, snifile);
		else
			printf("would do '%s'\n", cline);

		if( bOneCom )
			break;

		printf("-%% ");
		strcpy(oline, cline);
	}
	while( gets(cline) );

}
