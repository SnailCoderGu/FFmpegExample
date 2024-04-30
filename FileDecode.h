#pragma once
#include <string>
#include <iostream>
#include "SwrResample.h"
#include <thread>
#include "JitterBuffer.h"
#include <memory>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <atomic>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
}

//#define WRITE_DECODED_PCM_FILE
//#define WRITE_DECODED_YUV_FILE

class MyQtMainWindow;

#define AVJitterBuffer JitterBuffer<AVPacket*> 

class FileDecode
{
public:

	FileDecode():audio_packet_buffer(nullptr),video_packet_buffer(nullptr), swrResample(nullptr)
	{
		
	}

	~FileDecode();
	int AVOpenFile(std::string filename);
	int OpenAudioDecode();
	int OpenVideoDecode();
	int StartRead(std::string);
	int InnerStartRead();

	void SetPosition(int position);
	
	void Close();
	
	void SetMyWindow(MyQtMainWindow* mywindow);

	void PauseRender();
	void ResumeRender();

	void PauseRead();
	void ResumeRead();

	void ClearJitterBuf();

	void static AVPacketFreeBind(AVPacket* pkt)
	{
		av_free_packet(pkt);
		av_packet_unref(pkt);
	}

	std::string getCurrentTimeAsString() {
		// 获取当前时间点
		auto now = std::chrono::system_clock::now();

		// 将当前时间点转换为 time_t 类型
		auto now_c = std::chrono::system_clock::to_time_t(now);

		// 获取毫秒部分
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		// 格式化输出
		std::stringstream ss;
		ss << " ["<<std::put_time(std::localtime(&now_c), "%T") // 时分秒
			<< '.' << std::setfill('0') << std::setw(3) << ms.count()<<"]:"; // 毫秒

		return ss.str();
	}

	int64_t GetPlayingMs();
	int64_t GetFileLenMs();

private:

	int64_t player_start_time_ms = 0;
	/*
	* 自定义的一个系统时间，这个时间的起点是渲染起点 player_start_time_ms
	* 当发生移动视频位置的时候，需要修正时间，不然渲染判断就有问题
	*/
	inline void StartSysClockMs() // 记录启动时间，这个里面只会执行一次
	{
		if (player_start_time_ms == 0)
		{
			player_start_time_ms = now_ms();
		}
	}

	inline int64_t now_ms()
	{
		auto now = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	}

	inline int64_t GetSysClockMs()
	{
		if (player_start_time_ms != 0)
		{
			
			return  now_ms() - player_start_time_ms;
		}
		return 0;
	}

	int64_t video_stream_time = 0; //当前视频流渲染时间
	int64_t audio_stream_time = 0; //当前音频流渲染时间

	//在seek的时候，所有始终重置一次
	inline void ClockReset(int64_t seek_time)
	{
		audio_stream_time = seek_time;
		curr_playing_ms = audio_stream_time;
		video_stream_time = seek_time;

		//经过这翻操作，取GetSysClockMs()出来的就是 seek_time
		// now_ms() - (now_ms() - seek_time)

		player_start_time_ms = now_ms() - seek_time;

		// 这里面其实有个几毫秒的差别，问题不大，整体没有那么脆弱
	}


	int VideoDecodeFun();
	int AudioDecodeFun();
	void RunFFmpeg(std::string url);

	int DecodeAudio(AVPacket* originalPacket);
	int DecodeVideo(AVPacket* originalPacket);
	int ResampleAudio(AVFrame* frame);

	bool is_planar_yuv(enum AVPixelFormat pix_fmt);


private:
	
	AVFormatContext* formatCtx = NULL;
	AVCodecContext* audioCodecCtx = NULL;
	AVCodecContext* videoCodecCtx = NULL;

#ifdef WRITE_DECODED_PCM_FILE
	FILE* outdecodedfile = NULL;
#endif 

#ifdef WRITE_DECODED_YUV_FILE
	FILE* outdecodedYUVfile = NULL;
#endif 

	int audioStream;  //AVFormat 音频流索引
	int videoStream;  //AVFormat 视频流索引

	std::unique_ptr<SwrResample> swrResample; // 音频重采器

	MyQtMainWindow* qtWin = NULL;

	bool videoDecodeThreadFlag = true;
	std::thread* videoDecodeThread = nullptr;;  // 视频解码渲染线程

	bool audioDecodeThreadFlag = true;
	std::thread* audioDecodeThread = nullptr;;  //音频解码渲染线程

	
	bool read_frame_flag = true; //av_read_frame线程的标识
	std::thread* player_thread_ = nullptr;
	

	std::unique_ptr<AVJitterBuffer> audio_packet_buffer; //音频待解码渲染jitter buffer
	std::unique_ptr<AVJitterBuffer> video_packet_buffer; //视频待解码渲染jitter buffer

	int64_t audio_frame_dur = 0; //一帧音频需要经过的时间
	int64_t video_frame_dur= 0; // 一帧视频需要经过的时间

	int64_t file_len_ms; //视频总长度
	int64_t curr_playing_ms; //当前播放长度,取的音频播放时间戳

	bool pauseFlag =false;

	

	int64_t position_ms =  -1; //seek 时候，指定位置的毫秒数

	std::mutex read_mutex_;  //读文件锁

	bool pause_read_flag = true; //暂停读帧的标识
};

