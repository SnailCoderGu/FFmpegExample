#pragma once
#include <iostream>

extern "C" {
    #include <libavutil/opt.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/samplefmt.h>
	#include <libswresample/swresample.h>
}

#define WRITE_RESAMPLE_PCM_FILE

class SwrResample
{
public:
	int Init(int64_t src_ch_layout, int64_t dst_ch_layout,
		int src_rate, int dst_rate,
		enum AVSampleFormat src_sample_fmt, enum AVSampleFormat dst_sample_fmt,
		int src_nb_samples);

	int WriteInput(AVFrame* frame);

	int SwrConvert();

	void Close();

private:
	struct SwrContext* swr_ctx;

	uint8_t** src_data_;
	uint8_t** dst_data_;

	int src_nb_channels, dst_nb_channels;
	int src_linesize, dst_linesize;
	int src_nb_samples_, dst_nb_samples_;

	enum AVSampleFormat dst_sample_fmt_;

	enum AVSampleFormat src_sample_fmt_;

	

#ifdef WRITE_RESAMPLE_PCM_FILE
	FILE* outdecodedswffile;
#endif

};

