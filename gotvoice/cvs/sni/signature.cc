#include <stdio.h>
#include <time.h>
#include "sni.h"
#include "engine.h"
#include <errno.h>
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
#include "wave.h"
#include <signal.h>
#include <ctype.h>

extern CStaticEngine *pTheEngine;
static char szRcsId[] = "$Id: signature.cc,v 1.2 2005-03-02 01:19:26 martind Exp $";

//
// Signature.cpp: implementation of the CSignature class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSignature::CSignature()
{
	// Set up the Signature
	strncpy(&this->SigHeader.szMagic[0], SIGNEWHDRMAGIC, 4);
	SigHeader.nSamplesPerBlock = pTheEngine->nDftSamPerBlock;	// Must be 201
	SigHeader.nDftBlocks = 1;		// Must be 1.
	SigHeader.fdCorrel = 0.0;		// default Correlation Coefficient
	SigHeader.dAudioOffset = 0;	// Offset of DCT block in the audio, resolution SNM_NUM_MMBUFMS ms.

	// Allocate Dft Buffer
	this->pDft = (fftw_real *)malloc(sizeof(fftw_real)*pTheEngine->nDftSamPerBlock);
	this->pMaskDft = (fftw_real *)malloc(sizeof(fftw_real)*pTheEngine->nDftSamPerBlock);
	this->pZep = (fftw_real *)malloc(sizeof(fftw_real)*pTheEngine->nDftSamPerBlock/2+1);
	memset(this->pDft, 0 ,sizeof(fftw_real)*pTheEngine->nDftSamPerBlock);
	memset(this->pMaskDft, 0 ,sizeof(fftw_real)*pTheEngine->nDftSamPerBlock);
	memset(this->pZep, 0 ,sizeof(fftw_real)*pTheEngine->nDftSamPerBlock/2+1);
	pNext = NULL;
	pLast = NULL;
	bReady = FALSE;
	SigHeader.nTimeout = 40;	// default timeout

	SigHeader.sHostname[0] = '\0';
	SigHeader.sTag[0] = '\0';
	SigHeader.sDTMFTones[0] = '\0';
	strcpy(SigHeader.sWaveFile, "(null)");
	bLoadedWave = FALSE;
}

CSignature::~CSignature()
{
	if( pDft )free(pDft);
	if( pMaskDft )free(pMaskDft);
	if( pZep )free(pZep);
}
