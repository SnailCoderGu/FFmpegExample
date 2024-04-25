#include "FileDecode.h"

FileDecode::~FileDecode()
{
    if (swrResample != NULL)
    {
        delete swrResample;
        swrResample = NULL;
    }
}

int FileDecode::AVOpenFile(std::string filename)
{

#ifdef WRITE_DECODED_PCM_FILE
    outdecodedfile = fopen("decode.pcm", "wb");
    if (!outdecodedfile) {
        std::cout << "open out put file failed";
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
        std::cout << "av find best stream failed" << std::endl;
        return -1;
    }

    return 0;
}

int FileDecode::OpenAudioDecode()
{
    codecCtx = formatCtx->streams[audioStream]->codec;

    AVCodec* codec = avcodec_find_decoder(codecCtx->codec_id);
    if (codec == NULL) {
        std::cout << "cannot find codec id: " << codecCtx->codec_id << std::endl;
        return -1;
    }

    // Open codec
    AVDictionary* dict = NULL;
    int codecOpenResult = avcodec_open2(codecCtx, codec, &dict);
    if (codecOpenResult < 0) {
        std::cout << "open decode faild" << std::endl;
        return -1;
    }

    return 0;
}

int FileDecode::Decode()
{
    
    AVPacket avpkt;
    do {
        
        av_init_packet(&avpkt);
        if (av_read_frame(formatCtx, &avpkt) < 0) {

            //没有读到数据，说明结束了
            return 0;
        }
        if (avpkt.stream_index == audioStream)
        {
            //std::cout << "read one audio frame" << std::endl;
            DecodeAudio(&avpkt);
            av_packet_unref(&avpkt);
        }
        else {
            //暂时不处理其他针
            av_packet_unref(&avpkt);
            continue;
        }
    } while (avpkt.data == NULL);
}

void FileDecode::Close()
{
    if (swrResample) {
        swrResample->Close();
    }

#ifdef  WRITE_DECODED_PCM_FILE
    fclose(outdecodedfile);
#endif //  WRITE_DECODED_PCM_FILE



    avformat_close_input(&formatCtx);
    avcodec_free_context(&codecCtx);
}

int FileDecode::DecodeAudio(AVPacket* originalPacket)
{
    int ret = avcodec_send_packet(codecCtx, originalPacket);
    if (ret < 0)
    {
        return -1;
    }
    AVFrame* frame = av_frame_alloc();
    ret = avcodec_receive_frame(codecCtx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
        return -2;
    }else if (ret < 0) {
        std::cout << "error decoding";
        return -1;
    }

    int data_size = av_get_bytes_per_sample(codecCtx->sample_fmt);
    if (data_size < 0) {
        /* This should not occur, checking just for paranoia */
        std::cout << "Failed to calculate data size\n";
        return -1;
    }

#ifdef WRITE_DECODED_PCM_FILE
    for (int i = 0; i < frame->nb_samples; i++)
        for (int ch = 0; ch < codecCtx->channels; ch++)
            fwrite(frame->data[ch] + data_size * i, 1, data_size, outdecodedfile);
#endif

    // 把AVFrame里面的数据拷贝到，预备的src_data里面
    if (swrResample == NULL)
    {
        swrResample = new SwrResample();

        //创建重采样信息
        int src_ch_layout = codecCtx->channel_layout;
        int src_rate = codecCtx->sample_rate;
        enum AVSampleFormat src_sample_fmt = codecCtx->sample_fmt;

        int dst_ch_layout = AV_CH_LAYOUT_STEREO;
        int dst_rate = 44100;
        enum AVSampleFormat dst_sample_fmt = codecCtx->sample_fmt;

        //aac编码一般是这个,实际这个值只能从解码后的数据里面获取，所有这个初始化过程可以放在解码出第一帧的时候
        int src_nb_samples = frame->nb_samples;

        swrResample->Init(src_ch_layout, dst_ch_layout,
            src_rate, dst_rate,
            src_sample_fmt, dst_sample_fmt,
            src_nb_samples);
    }
   
    ret = swrResample->WriteInput(frame);

    int res = swrResample->SwrConvert();
   
    av_frame_free(&frame);
    return 0;
}
