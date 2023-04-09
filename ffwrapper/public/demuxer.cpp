#include "demuxer.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "../private/ff_helpers.h"
#include <stdexcept>

ff::demuxer_port::demuxer_port(demuxer& dem, int index) :
    stream(dem.format_ctx->streams[index]), in_use(true)
{
}       

int ff::demuxer_port::get_required_decoder_id() const
{
    return stream->codecpar->codec_id;
}

void ff::demuxer_port::add_packet(ff::packet&& pkt)
{
    queue.emplace(std::move(pkt));
}

ff::demuxer::demuxer(const input_media& m, const std::vector<int> unused_ports):
    file(m), format_ctx(m.get_format_ctx())
{
    ports.resize(m.num_streams());

    for (int i = 0, j = 0; i != m.num_streams(); ++i)
    {
        // j is the index of the first unused port that is not checked.
        // Now check if i == unused_ports[j]
        if (j < unused_ports.size())
        {
            if (i == unused_ports[j]) // skip this port
            {
                ++j; // go to the next unused port.
                continue; 
            }
        }
        else // don't skip this port
        {
            ports[i].stream = m.get_stream(i);
            ports[i].in_use = true;
        }
    }
}

int ff::demuxer::seek(int64_t time, int ref_port)
{
    if (ref_port < 0 || ref_port >= ports.size())
    {
        return -1;
    }
    else if (!ports[ref_port].is_used())
    {
        return -1;
    }
    if (time <= 0)
    {
        return -1;
    }

    int err = 0;
    if ((err = av_seek_frame
    (
        format_ctx,
        ref_port,
        time,
        AVSEEK_FLAG_BACKWARD
    )) < 0)
    {
        ON_FF_ERROR_WITH_CODE("Could not seek to the location specified.", err)
    }

    int ind = -1;
    ff::packet pkt;

    if ((err = av_read_frame(format_ctx, pkt)) >= 0)
    {
        ind = pkt->stream_index;

		// found the packet. put it in the port and return.
		ports[ind].add_packet(std::move(pkt));
		return ind;
    }
    else if(err != AVERROR_EOF)
    {
        ON_FF_ERROR_WITH_CODE("Unexpected error during seeking.", err);
    }

    return -1;
}

int ff::demuxer::demux_next_packet()
{
    int ret = -1;
    int err = 0;
    ff::packet pkt;

    if ((err = av_read_frame(format_ctx, pkt)) < 0)
    {
        if (err == AVERROR_EOF)
        {
            return -1;
        }
        else
        {
            ON_FF_ERROR_WITH_CODE("Could not demux the next packet.", err);
        }
    }

    ret = pkt->stream_index;
    ports[ret].add_packet(std::move(pkt));

    return ret;
}
