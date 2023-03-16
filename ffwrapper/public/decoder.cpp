extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "../private/ff_helpers.h"
#include "../private/ff_math_helpers.h"

#include "decoder.h"
#include "frame.h"

#include <stdexcept>

ff::input_decoder::input_decoder(const input_stream& st, ::AVFormatContext* fmt) :
    decoder(), s(st), fmt_ctx(fmt) 
{
    AVStream* pst = st.p_stream;

    /* find decoder for the stream */
    codec = avcodec_find_decoder(pst->codecpar->codec_id);
    if (!codec)
    {
        ON_FF_ERROR("Failed to find the decoder.")
    }
}

void ff::input_decoder::create(const class media& m)
{
    if (is_bound() && !is_ready())
    {
        AVStream* st = s.p_stream;

        /* Allocate a codec context for the decoder */
        codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx) 
        {
            ON_FF_ERROR("Failed to allocate the decoder context.")
        }

        /* Copy codec parameters from input stream to output codec context */
        if (avcodec_parameters_to_context(codec_ctx, st->codecpar) < 0) 
        {
            ON_FF_ERROR("Failed to copy decoder parameters from the stream to decoder context.")
        }

        configure_multithreading(16);

        /* Init the decoders */
        if (avcodec_open2(codec_ctx, codec, NULL) < 0) 
        {
            ON_FF_ERROR("Failed to open the decoder.")
        }
    }

}

ff::time ff::input_decoder::get_duration() const
{
    if (is_bound())
    {
        return time(s.duration);
    }

    return ff::zero_time;
}

bool ff::input_decoder::seek(double t, frame& f)
{
    if (!is_ready() || t < 0. || t > s.duration)
    {
        return false;
    }

    // Seeks to the first keyframe before t.
    // 
    // A keyframe is a frame that is independent in decoding to its previous frames.
    // That is, the decoder does not need information (packets) from previous frames to decode it.
    // From the first keyframe before t, we decode one by one and check the time until it's within t.
    //
    // It's also why seek() outputs the frame within t. 
    // Generally speaking, there's no way to just stop before the frame within t.

    if (av_seek_frame
    (
        fmt_ctx,
        s.p_stream->index,
        ffhelpers::ff_seconds_to_time_in_base(t, s.p_stream->time_base),
        AVSEEK_FLAG_BACKWARD
    ) < 0)
    {
        ON_FF_ERROR("Could not seek to the location specified.")
    }

    // Cleans the decoder's buffers (where there may be info left before seeking) so it can start decoding the keyframe.
    avcodec_flush_buffers(codec_ctx);

    bool ret = false;
    while (ret = decode_next_frame(f))
    {
        double frame_time = ffhelpers::ff_time_in_base_to_seconds(f.buffer->pts, s.p_stream->time_base);
        
        if (frame_time >= t) // the first frame whose time >= t
        {
            return true;
        }
    }

    if (!ret)
    {
        ON_FF_ERROR("Unexpected error: No frame is availble in seeking.")
    }

    return false;
}

bool ff::input_decoder::decode_next_frame(frame& f)
{
    if (!is_ready())
    {
        return false;
    }

    // We could first check if the decoder already has some frames left to be received
    // before submitting packets.
    // However, we choose to submit a packet every time without checking it.
    // This is for the following reasons:
    // 1. Always submitting packets makes the time cost more stable
    // 2. It doesn't matter if there are no more packets, as on that occasion the packet we get will be empty
    // 3. It make the logic and the code simpler

    // See https://ffmpeg.org/doxygen/5.1/group__lavc__encdec.html

    ff::packet pkt;
    pkt.allocate();

    int ret = 0;

    // feed packets to the decoder until we have a complete frame.
    while ((ret = av_read_frame(fmt_ctx, pkt)) >= 0)
    {
        if (pkt->stream_index == s.p_stream->index) // it's packet of the stream
        {
            // feed the packet to the decoder
            ret = avcodec_send_packet(codec_ctx, pkt);

            if (ret == AVERROR(ENOMEM))
            {
                ON_FF_ERROR("Could not sumbit a packet to the decoder.")
            }
            else
            {
                // try to get a frame from the decoder
                ret = avcodec_receive_frame(codec_ctx, f);

                if (ret >= 0) // we got the frame we want
                {
                    return true;
                }
                else if (ret == AVERROR_EOF)
                {
                    if (!is_eof_reached) // the first time EOF is reached
                    {
                        is_eof_reached = true;

                        return drain_decoder(f);
                    }
                    else
                    {
                        // No more frames is available.
                        return false;
                    }
                }
                else if (ret == AVERROR(EAGAIN)) // we need to feed more packets before we can get a frame.
                {
                    // continue feeding
                    continue;
                }
            }
        }

        // Don't forget to unref it after using
        pkt.unref();
    }

    if (ret == AVERROR_EOF)
    {
        return false;
    }
    else
    {
        ON_FF_ERROR_WITH_CODE("Could not read a packet from the file.", ret)
    }
    
}

bool ff::input_decoder::drain_decoder(frame& f)
{
    // enter draining mode
    int ret = avcodec_send_packet(codec_ctx, nullptr);
    if (ret < 0)
    {
        ON_FF_ERROR("Could not enter the draining mode in decoding.")
    }
    // see if there are buffered frames remaining
    ret = avcodec_receive_frame(codec_ctx, f);
    if (ret >= 0) // we got the frame we want
    {
        return true;
    }
    if (ret == AVERROR_EOF) // no more frames
    {
        return false;
    }
    else
    {
        ON_FF_ERROR("Could not get a frame in decoding.")
    }
}

int ff::decoder::feed_a_packet(packet& pkt)
{
    return avcodec_send_packet(codec_ctx, pkt);
}
