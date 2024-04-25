#pragma once
#include <string>
#include <iostream>
#include "SwrResample.h"

#define WRITE_DECODED_PCM_FILE
#define WRITE_DECODED_YUV_FILE

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
	int OpenVideoDecode();
	int Decode();
	void Close();
private:
	int DecodeAudio(AVPacket* originalPacket);
	int DecodeVideo(AVPacket* originalPacket);

	bool is_planar_yuv(enum AVPixelFormat pix_fmt);
private:
	
	AVFormatContext* formatCtx = NULL;
	AVCodecContext* audioCodecCtx = NULL;
	AVCodecContext* videoCodecCtx = NULL;

#ifdef WRITE_DECODED_PCM_FILE
	FILE* outdecodedfile;
#endif 

#ifdef WRITE_DECODED_YUV_FILE
	FILE* outdecodedYUVfile;
#endif 

	int audioStream;
	int videoStream;

private:
	SwrResample* swrResample = NULL;
};

