extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "../private/ff_helpers.h"

#include "codec.h"

#include <stdexcept>

void ff::codec_base::destroy()
{
	ffhelpers::safely_free_codec_context(&codec_ctx);
}

int ff::codec_base::get_current_pixel_format() const
{
	if (codec_ctx)
	{
		return codec_ctx->pix_fmt;
	}

	return 0;
}

void ff::codec_base::copy_current_audio_channel(::AVChannelLayout* dst) const
{
	int ret = 0;
	if (codec_ctx)
	{
		if ((ret = av_channel_layout_copy(dst, &codec_ctx->ch_layout)) < 0)
		{
			ON_FF_ERROR_WITH_CODE("Could not copy channel layout.", ret)
		}
	}
}

const::AVChannelLayout* ff::codec_base::get_current_audio_channel() const
{
	if (codec_ctx)
	{
		return &codec_ctx->ch_layout;
	}

	return nullptr;
}

int ff::codec_base::get_current_sample_rate() const
{
	if (codec_ctx)
	{
		return codec_ctx->sample_rate;
	}

	return 0;
}

int ff::codec_base::get_current_audio_sample_format() const
{
	if (codec_ctx)
	{
		return codec_ctx->sample_fmt;
	}

	return 0;
}

const AVRational* ff::codec_base::get_current_time_base() const
{
	if (codec_ctx)
	{
		return &codec_ctx->time_base;
	}

	return nullptr;
}

void ff::codec_base::configure_multithreading(int num_threads)
{
	codec_ctx->thread_count = num_threads;
}
