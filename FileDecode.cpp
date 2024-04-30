#include "FileDecode.h"
#include "MyQtMainWindow.h"
#include <windows.h>
FileDecode::~FileDecode()
{

    if (qtWin)
    {
        qtWin = NULL;
    }
}

int64_t FileDecode::GetPlayingMs()
{
    return curr_playing_ms;
}

int64_t FileDecode::GetFileLenMs()
{
    return file_len_ms;
}

void FileDecode::SetPosition(int position)
{
    position_ms = file_len_ms * position / 1000;
}

int FileDecode::AVOpenFile(std::string filename)
{

#ifdef WRITE_DECODED_PCM_FILE
    outdecodedfile = fopen("decode.pcm", "wb");
    if (!outdecodedfile) {
        std::cout << "open out put file failed";
    }
#endif

#ifdef WRITE_DECODED_YUV_FILE
    outdecodedYUVfile = fopen("decoded_video.yuv", "wb");
    if (!outdecodedYUVfile) {
        std::cout << "open out put YUV file failed";
    }
#endif

	int openInputResult = avformat_open_input(&formatCtx, filename.c_str(), NULL, NULL);
    if (openInputResult != 0) {
        std::cout << "open input failed" << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(formatCtx, NULL) < 0) {
        std::cout << "find stram info faild" << std::endl;
        return -1;
    }

    av_dump_format(formatCtx, 0, filename.c_str(), 0);

    audioStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audioStream < 0) {
        std::cout << "av find best audio stream failed" << std::endl;
        return -1;
    }

    videoStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoStream < 0) {
        std::cout << "av find best video stream failed" << std::endl;
        return -1;
    }

    return 0;
}

int FileDecode::OpenAudioDecode()
{
    audioCodecCtx = formatCtx->streams[audioStream]->codec;

    AVCodec* codec = avcodec_find_decoder(audioCodecCtx->codec_id);
    if (codec == NULL) {
        std::cout << "cannot find audio codec id: " << audioCodecCtx->codec_id << std::endl;
        return -1;
    }

    // Open codec
    AVDictionary* dict = NULL;
    int codecOpenResult = avcodec_open2(audioCodecCtx, codec, &dict);
    if (codecOpenResult < 0) {
        std::cout << "open audio decode faild" << std::endl;
        return -1;
    }
    AVRational pts_base = formatCtx->streams[audioStream]->time_base;
    file_len_ms = formatCtx->streams[audioStream]->duration * av_q2d(pts_base) * 1000;

    return 0;
}

int FileDecode::OpenVideoDecode() {
    videoCodecCtx = formatCtx->streams[videoStream]->codec;

    AVCodec* codec = avcodec_find_decoder(videoCodecCtx->codec_id);
    if (codec == NULL) {
        std::cout << "cannot find video codec id: " << videoCodecCtx->codec_id << std::endl;
        return -1;
    }

    // Open codec
    AVDictionary* dict = NULL;
    int codecOpenResult = avcodec_open2(videoCodecCtx, codec, &dict);
    if (codecOpenResult < 0) {
        std::cout << "open video decode faild" << std::endl;
        return -1;
    }

    int codec_width = videoCodecCtx->coded_width;
    qtWin->initData(videoCodecCtx->width, videoCodecCtx->height, codec_width);

    return 0;
}

int FileDecode::StartRead(std::string fildName)
{
    read_frame_flag = true;
    player_thread_ = new std::thread(&FileDecode::RunFFmpeg, this, fildName);

    return 0;
}

int FileDecode::InnerStartRead()
{
	audio_packet_buffer = std::make_unique<AVJitterBuffer>(10);
	video_packet_buffer = std::make_unique<AVJitterBuffer>(10);

  
    videoDecodeThreadFlag = true;
    videoDecodeThread = new std::thread(&FileDecode::VideoDecodeFun, this);  //启动解码线程

	audioDecodeThreadFlag = true;
	audioDecodeThread = new std::thread(&FileDecode::AudioDecodeFun, this);  //启动解码线程


	AVRational video_pts_base = formatCtx->streams[videoStream]->time_base;
    AVRational audio_pts_base = formatCtx->streams[audioStream]->time_base;
	int64_t audio_pts_begin = formatCtx->streams[audioStream]->start_time;
    int64_t video_pts_begin = formatCtx->streams[videoStream]->start_time;

	/*  int audio_dur_ms = formatCtx->streams[audioStream]->duration * av_q2d(pts_base) * 1000;
	  int video_dur_ms = formatCtx->streams[videoStream]->duration * av_q2d(pts_base) * 1000;*/

    StartSysClockMs();

    int result = 0;
    read_frame_flag = true;
    do {
        std::unique_lock<std::mutex> lock(read_mutex_);
        if (!pause_read_flag)
        {
            Sleep(2);
            lock.unlock();
            continue;
        }
       

        if (position_ms != -1)
        {
            double rr = av_q2d(audio_pts_base);
            int64_t base_position = (double)position_ms / (av_q2d(audio_pts_base) * 1000);
            int seek_flag = (position_ms <= curr_playing_ms) ? AVSEEK_FLAG_BACKWARD : AVSEEK_FLAG_FRAME;
			int ret = av_seek_frame(formatCtx, audioStream, base_position, seek_flag);
            qDebug() << "seek frame ms:" << position_ms;
 

            //清空缓冲区
            ClearJitterBuf();

            qDebug() << "audio queue buffer ms:" << audio_packet_buffer->size();
            qDebug() << "video queue buffer ms:" << video_packet_buffer->size();

            //发生seek的时候，重置时钟到seek的位置
            ClockReset(position_ms);

            //player_start_time_ms = GetNowMs() - position_ms;

            position_ms = -1;

        }

		AVPacket* avpkt = av_packet_alloc();
		av_init_packet(avpkt);

        int expand_size = 0;
       
        int read_ret = av_read_frame(formatCtx, avpkt);
        if (read_ret < 0) {
            char errmsg[AV_ERROR_MAX_STRING_SIZE];
            av_make_error_string(errmsg,AV_ERROR_MAX_STRING_SIZE, read_ret);
            if (read_ret == AVERROR_EOF)
			{
				qDebug() << "Reached end of file" << errmsg;
				read_frame_flag = false;
				av_packet_unref(avpkt);
				result = -1;
				break;

            }
            else
            {
               
				qDebug() << "Error while reading frames" << errmsg;
				continue;
            }
            
        }

        if (avpkt->stream_index == audioStream) {
            int64_t pkt_dur = avpkt->duration * av_q2d(audio_pts_base) * 1000;
            if (pkt_dur != 0 && pkt_dur != audio_frame_dur)
            {
                //jitterbuf 里面存放1秒的数据，这个Resize方法要注意死锁
                expand_size = 1000 / pkt_dur;
                audio_frame_dur = pkt_dur;
            }
            int64_t read_time = (avpkt->pts - audio_pts_begin) * av_q2d(audio_pts_base) * 1000;
            qDebug() << "push read audio frame ms: " << read_time << ":" << audio_packet_buffer->size();;
            audio_packet_buffer->Push(avpkt, expand_size);
            qDebug() << "push read audio frame end ms: " << read_time << ":" << audio_packet_buffer->size();;
            int buffer_time = audio_packet_buffer->size() * pkt_dur;
            //if (buffer_time > 200)
            //{
            //    Sleep(180);
            //}
        }
        else if(avpkt->stream_index == videoStream)
        {
			int64_t pkt_dur = avpkt->duration * av_q2d(video_pts_base) * 1000;
			if (pkt_dur != 0 && pkt_dur != video_frame_dur)
			{
				//jitterbuf 里面存放1秒的数据，这个Resize方法要注意死锁
				expand_size = 1000 / pkt_dur;
                audio_frame_dur = pkt_dur;
			}
			int64_t read_time = (avpkt->pts - video_pts_begin) * av_q2d(video_pts_base) * 1000;
			qDebug() << "push read video frame ms: " << read_time << ":"<< video_packet_buffer->size();
            video_packet_buffer->Push(avpkt, expand_size);
            qDebug() << "push read video frame end ms: " << read_time;
        }

        lock.unlock();
    } while (read_frame_flag);

    return result;
}

int FileDecode::VideoDecodeFun()
{
    AVRational pts_base = formatCtx->streams[videoStream]->time_base;
    int64_t pts_begin = formatCtx->streams[videoStream]->start_time;

    do {

        if (pauseFlag)  //停止了渲染，由于jitter的push阻塞机制，自动会阻塞读包线程
        {
            Sleep(2);
            continue;
        }

        int64_t sys_ms = GetSysClockMs();
        if (video_stream_time > GetSysClockMs())
        {
            //qDebug() << "VideoSleep" << video_stream_time<<":"<< sys_ms;
            Sleep(2);
            continue;
        }
        qDebug() << "video pop before: ";
        AVPacket* avpkt = video_packet_buffer->Pop(true);
        if (avpkt == nullptr)
        {
            if (!read_frame_flag)
            { 
                //如果已经读到文件尾部了，再pop不出数据，说明播放结束了
                break;
            }

            Sleep(2);
          
            continue;
        }
        //qDebug() << "video pop pts: " << (avpkt->pts - pts_begin) * av_q2d(pts_base) * 1000;

        if (avpkt->stream_index == videoStream) {

            DecodeVideo(avpkt);

            video_stream_time = (avpkt->pts - pts_begin) * av_q2d(pts_base) * 1000;
            //qDebug() << "render video frame ms: " << video_stream_time << ":" << sys_ms;

            av_packet_unref(avpkt);
            av_free_packet(avpkt);

        }

    } while (videoDecodeThreadFlag);

    return 0;


}

int FileDecode::AudioDecodeFun()
{
	AVRational pts_base = formatCtx->streams[audioStream]->time_base;
	int64_t pts_begin = formatCtx->streams[audioStream]->start_time;

	do {

        if (pauseFlag)
        {
            Sleep(2);
            continue;
        }
        int64_t sys_ms = GetSysClockMs();
		if (audio_stream_time > GetSysClockMs()) 
		{

            //qDebug() << "AudioSleep" << audio_stream_time << ":" << sys_ms;
            Sleep(2);
            continue;
		}
        
        AVPacket* avpkt = audio_packet_buffer->Pop(true);
        if (avpkt == nullptr)
        {
			if (!read_frame_flag)
			{
				//如果已经读到文件尾部了，再pop不出数据，说明播放结束了
				break;
			}
            Sleep(2);
           // qDebug() << "AudioSleep pop null";
            continue;
        }
        
       
		if (avpkt->stream_index == audioStream) {
			
			DecodeAudio(avpkt);
			
            audio_stream_time = (avpkt->pts - pts_begin) * av_q2d(pts_base) * 1000;
            curr_playing_ms = audio_stream_time;
            //qDebug() << "render audio frame ms: " << curr_playing_ms << ": " << sys_ms;

			av_packet_unref(avpkt);
			av_free_packet(avpkt);
		}

	} while (audioDecodeThreadFlag);

	return 0;
}


void FileDecode::Close()
{
    read_frame_flag = false;
    if (player_thread_ && player_thread_->joinable())
    {
        player_thread_->join();
        player_thread_ = nullptr;
    }

    videoDecodeThreadFlag = false;
    if (videoDecodeThread && videoDecodeThread->joinable())
    {
        videoDecodeThread->join();
        videoDecodeThread = nullptr;
    }

    audioDecodeThreadFlag = false;
    if (audioDecodeThread && audioDecodeThread->joinable())
    {
        audioDecodeThread->join();
        audioDecodeThread = nullptr;
    }



    if (swrResample) {
        swrResample->Close();
    }

#ifdef  WRITE_DECODED_PCM_FILE
    if (outdecodedfile) {
        fclose(outdecodedfile);
        outdecodedfile = NULL;
    }
#endif //  WRITE_DECODED_PCM_FILE

#ifdef  WRITE_DECODED_YUV_FILE
    if (outdecodedYUVfile) {
        fclose(outdecodedYUVfile);
        outdecodedYUVfile = NULL;
    }
#endif //  WRITE_DECODED_PCM_FILE

    if (audioCodecCtx) {
        avcodec_close(audioCodecCtx); // 注意这里要用关闭，不要用下面free，会不够彻底，导致avformat_close_input崩溃
        //avcodec_free_context(&audioCodecCtx);
        audioCodecCtx = nullptr;
    }
    if (videoCodecCtx) {
        avcodec_close(videoCodecCtx);
        //avcodec_free_context(&videoCodecCtx);
        videoCodecCtx = nullptr;
    }

    if (formatCtx) {
        avformat_close_input(&formatCtx);
        formatCtx = nullptr;
    }

    ClearJitterBuf();

    qtWin = nullptr;
    
}

void FileDecode::SetMyWindow(MyQtMainWindow* mywindow)
{
    this->qtWin = mywindow;
}

void FileDecode::PauseRender()
{
    pauseFlag = true;
}

void FileDecode::ResumeRender()
{
    pauseFlag = false;
}

void FileDecode::PauseRead()
{
    std::unique_lock<std::mutex> lock(read_mutex_);
    pause_read_flag = false;
}


void FileDecode::ResumeRead()
{
    std::unique_lock<std::mutex> lock(read_mutex_);
    pause_read_flag = true;
}

int FileDecode::DecodeAudio(AVPacket* originalPacket)
{
    int ret = avcodec_send_packet(audioCodecCtx, originalPacket);
    if (ret < 0)
    {
        return -1;
    }
    AVFrame* frame = av_frame_alloc();
    ret = avcodec_receive_frame(audioCodecCtx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
        return -2;
    }else if (ret < 0) {
        std::cout << "error decoding";
        return -1;
    }

    int data_size = av_get_bytes_per_sample(audioCodecCtx->sample_fmt);
    if (data_size < 0) {
        /* This should not occur, checking just for paranoia */
        std::cout << "Failed to calculate data size\n";
        return -1;
    }

#ifdef WRITE_DECODED_PCM_FILE
    for (int i = 0; i < frame->nb_samples; i++)
        for (int ch = 0; ch < audioCodecCtx->channels; ch++)
            fwrite(frame->data[ch] + data_size * i, 1, data_size, outdecodedfile);
#endif

    ResampleAudio(frame);
   
    av_frame_free(&frame);
    return 0;
}

int FileDecode::ResampleAudio(AVFrame* frame)
{
	// 把AVFrame里面的数据拷贝到，预备的src_data里面
	if (!swrResample)
	{
		swrResample = std::make_unique<SwrResample>();

		//创建重采样信息
		int src_ch_layout = audioCodecCtx->channel_layout;
		int src_rate = audioCodecCtx->sample_rate;
		enum AVSampleFormat src_sample_fmt = audioCodecCtx->sample_fmt;

		int dst_ch_layout = AV_CH_LAYOUT_STEREO;
		int dst_rate = 44100;
		enum AVSampleFormat dst_sample_fmt = AV_SAMPLE_FMT_S16;

		//aac编码一般是这个,实际这个值只能从解码后的数据里面获取，所有这个初始化过程可以放在解码出第一帧的时候
		int src_nb_samples = frame->nb_samples;

		swrResample->Init(src_ch_layout, dst_ch_layout,
			src_rate, dst_rate,
			src_sample_fmt, dst_sample_fmt,
			src_nb_samples);
	}

	swrResample->WriteInput(frame);

	int res = swrResample->SwrConvert();

    return res;
}


int FileDecode::DecodeVideo(AVPacket* originalPacket)
{
    
    int ret = avcodec_send_packet(videoCodecCtx, originalPacket);
    if (ret < 0)
    {
        return -1;
    }
    AVFrame* frame = av_frame_alloc();
    ret = avcodec_receive_frame(videoCodecCtx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return -2;
    }
    else if (ret < 0) {
        std::cout << "error decoding";
        return -1;
    }
#ifdef  WRITE_DECODED_YUV_FILE


    enum AVPixelFormat pix_fmt = videoCodecCtx->pix_fmt;
    bool isPlanar = is_planar_yuv(pix_fmt);

    int wrapy = frame->linesize[0];
    int wrapu = frame->linesize[1];
    int wrapv = frame->linesize[2];
    int xsize = frame->width;
    int ysize = frame->height;

    if (pix_fmt == AV_PIX_FMT_YUV420P) {
        fwrite(frame->data[0], 1, wrapy * ysize, outdecodedYUVfile); // Y
        fwrite(frame->data[1], 1, wrapu * ysize / 2, outdecodedYUVfile); // U
        fwrite(frame->data[2], 1, wrapv * ysize / 2, outdecodedYUVfile); // V
    }


#endif //  WRITE_DECODED_PCM_FILE

    qtWin->updateYuv(frame->data[0], frame->data[1], frame->data[2]);

    av_frame_free(&frame);

    return 0;
   
}

bool FileDecode::is_planar_yuv(enum AVPixelFormat pix_fmt) {
   
    // Check if the pixel format corresponds to planar layout
    switch (pix_fmt) {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUV444P:
        // Add more planar pixel formats here if needed
        return 1; // Planar layout
    default:
        return 0; // Non-planar layout
    }
}

void FileDecode::RunFFmpeg(std::string url)
{
    

    // 注册所有的编解码器、格式和协议
    av_register_all();


    int ret = AVOpenFile(url);
    if (ret != 0)
    {
        std::cout << "AVOpenFile Faild";
    }
    
    ret = OpenVideoDecode();
    ret = OpenAudioDecode();

    ret = InnerStartRead();
    if (ret == -1)
    {
        std::cout << "读到文件尾部了,并且都渲染完了" << std::endl;

        std::ostringstream oss;
        oss << std::this_thread::get_id();
        qDebug() << oss.str().c_str();
        QMetaObject::invokeMethod(qtWin, &MyQtMainWindow::ClosePlayer);
    }
}



void FileDecode::ClearJitterBuf()
{
    if (audio_packet_buffer) {
        qDebug() << "start complete clear buffer";
        std::function<void(AVPacket* pkt)> func = &FileDecode::AVPacketFreeBind;
        audio_packet_buffer->Clear(func);
        video_packet_buffer->Clear(func);

        qDebug() << "complete clear buffer" << audio_packet_buffer->size() << " : " << video_packet_buffer->size();
    }
}
