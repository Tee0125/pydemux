
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

    int cur_video_pts_in_ms;

} demux_ctx_t;

demux_ctx_t* demux_open(char* filename);
void demux_release_frame(demux_ctx_t* ctx, uint8_t* frame);
void demux_close(demux_ctx_t* ctx);

uint8_t* demux_get_frame(demux_ctx_t* ctx);

int demux_get_width(demux_ctx_t* ctx);
int demux_get_height(demux_ctx_t* ctx);

// forward/backward by delta_ms
// stream_type: 0 for video 1 for audio
int demux_move(demux_ctx_t* ctx, int stream_type, int delta_ms);
int demux_goto(demux_ctx_t* ctx, int stream_type, int ms, int flag);


#endif
