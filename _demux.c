/*
 * Copyright (c) 2018 Tee Jung
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "_demux.h"
#include "_yuv2rgb.h"

static int open_codec_ctx(int *stream_idx,
                          AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx,
                          enum AVMediaType type);

static int decode_packet(demux_ctx_t* ctx, AVFrame **frame, AVPacket* pkt);
static int decode_video_packet(demux_ctx_t* ctx, AVFrame **frame, AVPacket* pkt);

#ifdef SUPPORT_AUDIO
static int decode_audio_packet(demux_ctx_t* ctx, AVFrame **frame, AVPacket* pkt);
#endif

static int _demux_initialized = 0;

demux_ctx_t* demux_open(char* filename)
{
    demux_ctx_t* ctx;

    AVFormatContext* fmt_ctx = NULL;
    AVCodecContext *dec_ctx = NULL;

    int ret;
    int stream_idx;

    if (!_demux_initialized)
    {
        /* register all formats and codecs */
        av_register_all();

        _demux_initialized = 1;
    }

    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", filename);
        return NULL;
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return NULL;
    }

    /* alloc demux context */
    ctx = (demux_ctx_t*)malloc(sizeof(demux_ctx_t));
    if (ctx == NULL)
    {
        fprintf(stderr, "Could not allocate context memory\n");
        return NULL;
    }
    else
    {
        memset(ctx, 0, sizeof(demux_ctx_t));
        ctx->fmt_ctx = fmt_ctx;
    }

    ret = open_codec_ctx(&stream_idx, &dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO);
    if (ret >= 0)
    {
        if (fmt_ctx->streams[stream_idx])
        {
            ctx->has_video_stream = 1;

            ctx->vdec_ctx = dec_ctx;
            ctx->video_stream_idx = stream_idx;
        }
        else
        {
            avcodec_free_context(&dec_ctx);
        }
    }

#ifdef SUPPORT_AUDIO
    ret = open_codec_ctx(&stream_idx, &dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO);
    if (ret >= 0)
    {
        if (fmt_ctx->streams[stream_idx])
        {
            ctx->has_audio_stream = 1;

            ctx->audio_stream_idx = stream_idx;
            ctx->adec_ctx = dec_ctx;
        }
        else
        {
            avcodec_free_context(&dec_ctx);
        }
    }
#endif

    if (!ctx->has_video_stream && !ctx->has_audio_stream)
    {
        fprintf(stderr, "No stream found\n");
        demux_close(ctx);
        return NULL;
    }

    /* allocate frame buffer */
    ctx->frame = av_frame_alloc();
    if (!ctx->frame)
    {
        fprintf(stderr, "Could not allocate frame\n");
        demux_close(ctx);
        return NULL;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&ctx->pkt);
    ctx->pkt.data = NULL;
    ctx->pkt.size = 0;

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, filename, 0);

    return ctx;
}

uint8_t* demux_get_frame(demux_ctx_t* ctx)
{
    AVFrame* frame = NULL;
    AVPacket* pkt = &ctx->pkt;

    int width;
    int height;

    int ret;

    /* read frames from the file */
    while (pkt->size > 0 || av_read_frame(ctx->fmt_ctx, pkt) >= 0)
    {
        AVPacket orig_pkt = *pkt;
        do
        {
            ret = decode_packet(ctx, &frame, pkt);
            if (ret < 0)
            {
                pkt->data = NULL;
                pkt->size = 0;
            }
            else
            {
                pkt->data += ret;
                pkt->size -= ret;
            }
        } while (pkt->size > 0);
        av_packet_unref(&orig_pkt);

        if (frame)
            break;
    }

    if (!frame)
    {
        /* flush cached frames */
        pkt->data = NULL;
        pkt->size = 0;

        decode_packet(ctx, &frame, pkt);
    }

    if (frame)
    {
        width  = ctx->vdec_ctx->width;
        height = ctx->vdec_ctx->height;

        // save pts time
        AVRational time_base = ctx->fmt_ctx->streams[ctx->video_stream_idx]->time_base;
        int frame_pts_in_ms = frame->pts * av_q2d(time_base) * 1000;
        printf("Frame in %d ms\n", frame_pts_in_ms);
        if(frame_pts_in_ms + 250 < ctx->cur_video_pts_in_ms) {
            printf("Skip old frames\n");
            av_frame_unref(frame);
            return NULL;
        }

        ctx->cur_video_pts_in_ms = frame_pts_in_ms;
        return yuv420_to_rgb24(frame, width, height);
    }
    else
        return NULL;

}

int demux_get_width(demux_ctx_t* ctx)
{
    return ctx->vdec_ctx->width;
}

int demux_get_height(demux_ctx_t* ctx)
{
    return ctx->vdec_ctx->height;
}

void demux_release_frame(demux_ctx_t* ctx, uint8_t* frame)
{
    free(frame);
}

void demux_close(demux_ctx_t* ctx)
{
    if (ctx->vdec_ctx != NULL)
        avcodec_free_context(&ctx->vdec_ctx);

    if (ctx->adec_ctx != NULL)
        avcodec_free_context(&ctx->adec_ctx);

    if (ctx->fmt_ctx != NULL)
        avformat_close_input(&ctx->fmt_ctx);

    if (ctx->frame != NULL)
        av_frame_free(&ctx->frame);

    free(ctx);
}

static int decode_video_packet(demux_ctx_t* ctx, AVFrame** frame, AVPacket* pkt)
{
    int got_frame;
    int ret;

    printf("Decode Pts: %lld\n", pkt->pts);

    /* decode video frame */
    ret = avcodec_decode_video2(ctx->vdec_ctx, ctx->frame, &got_frame, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error decoding video frame (%s)\n", av_err2str(ret));
        return ret;
    }

    if (got_frame)
        *frame = ctx->frame;
    else
        *frame = NULL;

    return pkt->size;
}

#ifdef SUPPORT_AUDIO
static int decode_audio_packet(demux_ctx_t* ctx, AVFrame **frame, AVPacket* pkt)
{
    int got_frame;
    int decoded;

    size_t size;

    /* decode audio frame */
    ret = avcodec_decode_audio4(ctx->adec_ctx, ctx->frame, &got_frame, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error decoding audio frame (%s)\n", av_err2str(ret));
        return ret;
    }

    /* Some audio decoders decode only part of the packet, and have to be
     * called again with the remainder of the packet data.
     * Sample: fate-suite/lossless-audio/luckynight-partial.shn
     * Also, some decoders might over-read the packet. */
    decoded = FFMIN(ret, pkt->size);

    // [FIXME] not updated yet (code in below is part of example demux code)
    if (got_frame)
    {
        *frame = ctx->frame;

        // [FIXME]
        size = frame->nb_samples * av_get_bytes_per_sample(frame->format);

        printf("audio_frame%s n:%d nb_samples:%d pts:%s\n",
               cached ? "(cached)" : "",
               audio_frame_count++, frame->nb_samples,
               av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));

        /* Write the raw audio data samples of the first plane. This works
         * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
         * most audio decoders output planar audio, which uses a separate
         * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
         * In other words, this code will write only the first audio channel
         * in these cases.
         * You should use libswresample or libavfilter to convert the frame
         * to packed data. */
        fwrite(frame->extended_data[0], 1, size, audio_dst_file);
    }

    return decoded;
}
#endif

static int decode_packet(demux_ctx_t* ctx, AVFrame **frame, AVPacket* pkt)
{
    *frame = NULL;

    if (ctx->has_video_stream && pkt->stream_index == ctx->video_stream_idx)
        return decode_video_packet(ctx, frame, pkt);
#ifdef SUPPORT_AUDIO
    if (ctx->has_audio_stream && pkt->stream_index == ctx->audio_stream_idx)
        return decode_audio_packet(ctx, frame, pkt);
#endif

    return pkt->size;
}

static int open_codec_ctx(int *stream_idx,
                          AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx,
                          enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream\n",
                av_get_media_type_string(type));
        return ret;
    }
    else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec)
        {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx)
        {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
        {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders, without reference counting */
        av_dict_set(&opts, "refcounted_frames", "0", 0);
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }
    return 0;
}

int demux_move(demux_ctx_t* ctx, int stream_type, int delta_ms) {
    int target_point_in_ms = ctx->cur_video_pts_in_ms + delta_ms;
    int flag = AVSEEK_FLAG_BACKWARD;
    if(delta_ms) {
        flag = AVSEEK_FLAG_ANY;
    }
    return demux_goto(ctx, stream_type, target_point_in_ms, flag);
}

int demux_goto(demux_ctx_t* ctx, int stream_type, int ms, int flag) {
    int stream_idx = -1;
    if(stream_type == 0 && ctx->has_video_stream) {
        stream_idx = ctx->video_stream_idx;
    } else if(stream_type == 1 && ctx->has_audio_stream) {
        stream_idx = ctx->audio_stream_idx;
    }

    if(stream_idx < 0) {
        printf("Stream Not Found\n");
        return -1;
    }

    if(!flag) {
      flag = AVSEEK_FLAG_BACKWARD;
    }

    AVRational time_base = ctx->fmt_ctx->streams[stream_idx]->time_base;
    int64_t seek_timestampe = ms * 0.001 / av_q2d(time_base);
    printf("Seek to %d ms, timestampe is %lld\n", ms, seek_timestampe);

    int ret = av_seek_frame(ctx->fmt_ctx, stream_idx, seek_timestampe, flag);

    if(ret >= 0) {
        ctx->cur_video_pts_in_ms = ms;
    }

    return ret;
}


#ifdef TEST
int main(int argc, char** argv)
{
    demux_ctx_t* ctx;
    uint8_t* frame;

    int i;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s video_file_path\n", argv[0]);
        return 1;
    }

    ctx = demux_open(argv[1]);

    for (i = 0 ; i < 100 ; i++)
    {
        frame = demux_get_frame(ctx);
        if (frame == NULL)
            break;
        else {
            fprintf(stderr, "%dth frame found\n", i);

            fprintf(stderr, "w: %d, h: %d, dst: %08x\n", ctx->vdec_ctx->width, ctx->vdec_ctx->height, frame);

            char buf[255];
            sprintf(buf, "a%d.rgb", "wb", i);
            FILE* fp = fopen(buf, "wb");
            printf("w:%d, h:%d\n", ctx->vdec_ctx->width, ctx->vdec_ctx->height);
            fwrite(frame, 1, ctx->vdec_ctx->width*ctx->vdec_ctx->height*3, fp);
            fclose(fp);
        }
    }
    demux_close(ctx);

    return 0;
}
#endif
