#pragma once
#include <string>
#include <iostream>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswresample/swresample.h>
	#include <libavutil/avutil.h>
}

class FileDecode
{
public:
	int OpenFile(std::string filename);
	int OpenAudioDecode();
	int Decode();
private:
	int DecodeAudio(AVPacket* originalPacket);
private:
	AVCodec* codec = NULL;
	AVFormatContext* formatCtx = NULL;
	AVCodecParameters* codecPara =NULL;
	AVCodecContext* codecCtx = NULL;

	FILE* outdecodedfile;

	int audioStream;
};

