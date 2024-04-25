#pragma once
#include <string>
#include <iostream>
#include "SwrResample.h"

#define WRITE_DECODED_PCM_FILE

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswresample/swresample.h>
	#include <libavutil/avutil.h>
}

class FileDecode
{
public:
	~FileDecode();
	int AVOpenFile(std::string filename);
	int OpenAudioDecode();
	int Decode();
	void Close();
private:
	int DecodeAudio(AVPacket* originalPacket);
private:
	
	AVFormatContext* formatCtx = NULL;
	AVCodecContext* codecCtx = NULL;

#ifdef WRITE_DECODED_PCM_FILE
	FILE* outdecodedfile;
#endif 

	int audioStream;

private:
	SwrResample* swrResample = NULL;
};

