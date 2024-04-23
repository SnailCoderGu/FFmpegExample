#include "FileDecode.h"


int FileDecode::OpenFile(std::string filename)
{

    outdecodedfile = fopen("decode.pcm", "wb");
    if (!outdecodedfile) {
        std::cout << "open out put file failed";
    }

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

    codec = avcodec_find_decoder(codecCtx->codec_id);
    if (codec == NULL) {
        std::cout << "cannot find codec id: " << codecPara->codec_id << std::endl;
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
    for (int i = 0; i < frame->nb_samples; i++)
        for (int ch = 0; ch < codecCtx->channels; ch++)
            fwrite(frame->data[ch] + data_size * i, 1, data_size, outdecodedfile);
    return 0;
}
