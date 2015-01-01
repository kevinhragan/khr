//	etc....
/* quick and dirty RIFF chunk */
//const	BYTE ab_riffchunk[] = {'R','I','F','F',  0,0,0,0, 'W','A','V','E'};
/* quick and dirty format chunk tag */
//const	BYTE ab_formatchunktag[] = {'f','m','t',' ',  0,0,0,0};
/* quick and dirty data chunk header */
//const	BYTE ab_datachunktag[] = {'d','a','t','a',  0,0,0,0};

/* message headers */
//const	WAVEHDR inMessageHeader = {NULL, 0, 0, 0, 0 /* no flags */, 0 /* no loops */,  
//                           NULL, 0};

struct waveheader {
	long	lRiff; 	//	'RIFF'
	long	lFileSize;
	long	lWave;	// 	'WAVE'
	long	lFormat;
	long	lFormatLength;
};

struct	waveformat {
	short	wFormatTag;
	short	nChannels;
	long	nSamplesPerSec;
	long	nAvgBytesPerSec;
	short	nBlockAlign;
	short	wBitsPerSample;
	short	padding[8];
};

/* format of in-message */
#define WAVE_FORMAT_PCM     1

const	waveformat messageFormat = {WAVE_FORMAT_PCM, 1, 8000, 16000, 2, 16};

struct	datahdr {
	long	lType;	// 'data'
	long	lLenData;
};

#define ABUFSIZ	400	// size of audio buffer

extern FILE *gvPlayWavFile(char *filename, int audio_fd);
extern FILE *gvRawToWav(char *rawfile, char *wavfile);
extern FILE *gvWavToRaw(char *rawfile, char *wavfile);
