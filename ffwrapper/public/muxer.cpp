extern "C"
{
#include <libavformat/avformat.h>
}

#include "muxer.h"
#include "frame.h"
#include "media.h"
#include "../private/ff_helpers.h"

#include <stdexcept>


void ff::muxer::write_file_header()
{
	int ret;
	if ((ret = avformat_write_header(fmt_ctx, nullptr)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not write the file header.", ret)
	}
}

bool ff::muxer::try_feed(ff::packet& pkt)
{
	int ret = 0;
	if ((ret = av_interleaved_write_frame(fmt_ctx, pkt)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not feed a packet to the output file: ", ret)
	}

	return true;
}

void ff::muxer::finalize()
{
	int ret;
	if ((ret = av_write_trailer(fmt_ctx)) != 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not finalize the output file.", ret)
	}
}
