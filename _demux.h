
#ifndef __DEMUX_H__
#define __DEMUX_H__

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

typedef struct
{
    AVFormatContext* fmt_ctx;

    AVCodecContext* vdec_ctx;
    AVCodecContext* adec_ctx;

    AVFrame* frame;
    AVPacket pkt;

    int video_stream_idx;
    int audio_stream_idx;

    int has_video_stream;
    int has_audio_stream;

} demux_ctx_t;

demux_ctx_t* demux_open(char* filename);
void demux_relase_frame(demux_ctx_t* ctx, uint8_t* frame);
void demux_close(demux_ctx_t* ctx);

uint8_t* demux_get_frame(demux_ctx_t* ctx);

int demux_get_width(demux_ctx_t* ctx);
int demux_get_height(demux_ctx_t* ctx);

#endif
