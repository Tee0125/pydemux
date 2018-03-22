
#ifndef __YUV2RGB_H__
#define __YUV2RGB_H__

#include <stdint.h>
#include <libavutil/frame.h>

uint8_t* yuv420_to_rgb24(AVFrame* frame, int w, int h);

#endif
