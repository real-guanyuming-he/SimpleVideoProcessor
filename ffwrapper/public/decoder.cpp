extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "../private/ff_helpers.h"
#include "../private/ff_math_helpers.h"

#include "decoder.h"
#include "demuxer.h"
#include "frame.h"

#include <stdexcept>

ff::decoder::decoder(int ID)
{
    codec = avcodec_find_decoder((AVCodecID)ID);
    if (!codec)
    {
        ON_FF_ERROR("Failed to find the decoder.")
    }

    /* Allocate a codec context for the decoder */
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        ON_FF_ERROR("Failed to allocate the decoder context.")
    }
}

ff::decoder::decoder(const char* name)
{
    codec = avcodec_find_decoder_by_name(name);
    if (!codec)
    {
        ON_FF_ERROR("Failed to find the decoder.")
    }

    /* Allocate a codec context for the decoder */
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        ON_FF_ERROR("Failed to allocate the decoder context.")
    }
}

bool ff::decoder::try_feed(ff::packet& pkt)
{
    int ret = avcodec_send_packet(codec_ctx, pkt);

    if (ret < 0)
    {
        switch (ret)
        {
        case AVERROR_EOF: 
            // Intentionally no break.
        case AVERROR(EAGAIN):
            return false;
        default:
            ON_FF_ERROR_WITH_CODE("Could not feed a packet to the decoder.", ret);
        }
    }

    return true;
}

ff::frame ff::decoder::try_get_one()
{
    ff::frame f;
    int ret = avcodec_receive_frame(codec_ctx, f);

    if (ret >= 0) // we got the frame we want
    {
        return f;
    }
    else
    {
        switch (ret)
        {
        case AVERROR_EOF:
            eof_reached = true;
            // Intentionally no break.
        case AVERROR(EAGAIN):
            return ff::frame(nullptr);
        default:
            ON_FF_ERROR_WITH_CODE("Could not decode a frame.", ret);
        }       
    }
}

void ff::decoder::flush_codec()
{
    super::flush_codec();
    // reset eof state as the decoder is flushed.
    eof_reached = false;
}

void ff::decoder::start_draining()
{
    int ret = avcodec_send_packet(codec_ctx, nullptr);
    if (ret < 0)
    {
        ON_FF_ERROR_WITH_CODE("Could not enter the draining mode during decoding.", ret)
    }
}

ff::input_decoder::input_decoder(const demuxer_port& port) : input_decoder(port.stream.p_stream) {}

ff::input_decoder::input_decoder(const::AVStream* st) : decoder(st->codecpar->codec_id)
{
    /* Copy codec parameters from input stream to output codec context */
    if (avcodec_parameters_to_context(codec_ctx, st->codecpar) < 0)
    {
        ON_FF_ERROR("Failed to copy decoder parameters from the stream to decoder context.")
    }

    // time_base is deprecated for decoding.
    // But we still set it for reference
    codec_ctx->time_base = st->time_base;
    // Instead, set the frame rate
    codec_ctx->framerate = st->r_frame_rate;

    // for debug purposes only
    //configure_multithreading(16);

    create();
}
