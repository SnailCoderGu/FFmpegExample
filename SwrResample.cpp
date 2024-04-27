#include "SwrResample.h"

int  SwrResample::Init(int64_t src_ch_layout, int64_t dst_ch_layout,
    int src_rate, int dst_rate,
    enum AVSampleFormat src_sample_fmt, enum AVSampleFormat dst_sample_fmt,
    int src_nb_samples)
{
#ifdef WRITE_RESAMPLE_PCM_FILE
    outdecodedswffile = fopen("decode_resample.pcm", "wb");
    if (!outdecodedswffile) {
        std::cout << "open out put swr file failed";
    }
#endif // WRITE_RESAMPLE_PCM_FILE

    src_sample_fmt_ = src_sample_fmt;
    dst_sample_fmt_ = dst_sample_fmt;

    int ret;
    /* create resampler context */
    swr_ctx = swr_alloc();
    if (!swr_ctx) {
        std::cout << "Could not allocate resampler context" << std::endl;
        ret = AVERROR(ENOMEM);
        return ret;
    }

    /* set options */
    av_opt_set_int(swr_ctx, "in_channel_layout", src_ch_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", src_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", src_sample_fmt, 0);

    av_opt_set_int(swr_ctx, "out_channel_layout", dst_ch_layout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", dst_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", dst_sample_fmt, 0);

    /* initialize the resampling context */
    if ((ret = swr_init(swr_ctx)) < 0) {
        std::cout << "Failed to initialize the resampling context" << std::endl;
        return -1;
    }

    //配置输入的参数
    /*
    * src_nb_samples: 描述一整的采样个数 比如这里就是 1024
    * src_linesize: 描述一行采样字节长度 
    *   当是planr 结构 LLLLLRRRRRR 的时候 比如 一帧1024个采样，32位表示。那就是 1024*4 = 4096 
    *   当是非palner 结构的时候 LRLRLR 比如一帧1024采样 32位表示 双通道   1024*4*2 = 8196 要乘以通道
    * src_nb_channels : 可以根据布局获得音频的通道
    * ret 返回输入数据的长度 比如这里 1024 * 4 * 2 = 8196 (32bit，双声道，1024个采样)
    */
    src_nb_channels = av_get_channel_layout_nb_channels(src_ch_layout);
    
    ret = av_samples_alloc_array_and_samples(&src_data_, &src_linesize, src_nb_channels,
        src_nb_samples, src_sample_fmt, 0);
    if (ret < 0) {
        std::cout << "Could not allocate source samples\n" << std::endl;
        return -1;
    }
    src_nb_samples_ = src_nb_samples;

    //配置输出的参数
    int max_dst_nb_samples = dst_nb_samples_ =
        av_rescale_rnd(src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);

    dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
    
    ret = av_samples_alloc_array_and_samples(&dst_data_, &dst_linesize, dst_nb_channels,
        dst_nb_samples_, dst_sample_fmt, 0);
    if (ret < 0) {
        std::cout << "Could not allocate destination samples" << std::endl;
        return -1;
    }

    int data_size = av_get_bytes_per_sample(dst_sample_fmt_);
    audioPlayer.SetFormat(dst_nb_samples_, dst_rate, data_size*8, dst_nb_channels);
}

int SwrResample::WriteInput(AVFrame* frame)
{
    int planar = av_sample_fmt_is_planar(src_sample_fmt_);
    int data_size = av_get_bytes_per_sample(src_sample_fmt_);
    if (planar)
    {

        //src是planar类型的话，src_data里面数据是LLLLLLLRRRRR 结构，src_data_[0] 指向全部的L，src_data_[1] 指向全部R
        // src_data_ 里面其实一个长 uint8_t *buf，src_data_[0] 指向L开始的位置，src_data_[1]指向R的位置
        // linesize 是 b_samples * sample_size 就是比如 48000*4
        for (int ch = 0; ch < src_nb_channels; ch++) {
            memcpy(src_data_[ch], frame->data[ch], data_size * frame->nb_samples);
        }
    }
    else
    {
        //src是非planar类型的话，src_data里面数据是LRLRLRLR 结构，src_data_[0] 指向全部数据 没有src_data[1]
        // linesize 是nb_samples * sample_size * nb_channels 比如 48000*4*2
        for (int i = 0; i < frame->nb_samples; i++){
            for (int ch = 0; ch < src_nb_channels; ch++)
            {
                memcpy(src_data_[0], frame->data[ch]+data_size*i, data_size);
            }
        }
    }
    return 0;
}


int SwrResample::SwrConvert()
{
    int ret = swr_convert(swr_ctx, dst_data_, dst_nb_samples_, (const uint8_t**)src_data_, src_nb_samples_);
    if (ret < 0) {
        fprintf(stderr, "Error while converting\n");
        exit(1);
    }

    int  dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
        ret, dst_sample_fmt_, 1);

   int planar = av_sample_fmt_is_planar(dst_sample_fmt_);
   if (planar)
   {
       int data_size = av_get_bytes_per_sample(dst_sample_fmt_);
       for (int i = 0; i < dst_nb_samples_; i++) {
           for (int ch = 0; ch < dst_nb_channels; ch++)
           {
               fwrite(dst_data_[ch]+i*data_size, 1, data_size, outdecodedswffile);
           }
       }
   }
   else {
       //非planr结构，dst_data_[0] 里面存在着全部数据
       fwrite(dst_data_[0], 1, dst_bufsize, outdecodedswffile);
       audioPlayer.writeData((const char*)(dst_data_[0]), dst_bufsize);
   }

    return dst_bufsize;
}

void SwrResample::Close()
{

#ifdef WRITE_RESAMPLE_PCM_FILE
    fclose(outdecodedswffile);
#endif

    if (src_data_)
        av_freep(&src_data_[0]);

    av_freep(&src_data_);

    if (dst_data_)
        av_freep(&dst_data_[0]);

    av_freep(&dst_data_);

    swr_free(&swr_ctx);

    audioPlayer.Close();
}
