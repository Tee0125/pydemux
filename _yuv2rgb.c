
#include "_yuv2rgb.h"

#define clip255(a) ((a>255)?255:((a<0)?0:a))

uint8_t* yuv420_to_rgb24(AVFrame* frame, int w, int h)
{
    uint8_t* y = frame->data[0];
    uint8_t* u = frame->data[1];
    uint8_t* v = frame->data[2];

    uint8_t* rgb = malloc(sizeof(uint8_t)*w*h*3);
    uint8_t* dst = rgb;

    int c, d, e;
    int i, j;
    int offset;

    int stride = w*3;

    if (dst == NULL)
        return NULL;

    for (j = 0 ; j < h ; j++)
    {
        offset = 0;

        for (i = 0 ; i < w ; i++)
        {
            c = y[(i   )] -  16;
            d = u[(i>>1)] - 128;
            e = v[(i>>1)] - 128;

            dst[offset+0] = clip255((298 * c           + 409 * e + 128) >> 8);
            dst[offset+1] = clip255((298 * c - 100 * d - 208 * e + 128) >> 8);
            dst[offset+2] = clip255((298 * c + 516 * d           + 128) >> 8);

            offset += 3;
        }

        dst += stride;

        y += frame->linesize[0];
        if (j&1)
        {
            u += frame->linesize[1];
            v += frame->linesize[2];
        }
    }

    return rgb;
}
