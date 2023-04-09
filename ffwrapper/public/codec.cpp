extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "../private/ff_helpers.h"
#include "../private/utility/info.h"

#include "codec.h"

#include <stdexcept>

void ff::codec_base::destroy()
{
	ffhelpers::safely_free_codec_context(&codec_ctx);
}

void ff::codec_base::flush_codec()
{
	avcodec_flush_buffers(codec_ctx);
}

int ff::codec_base::get_current_pixel_format() const
{
	if (codec_ctx)
	{
		return codec_ctx->pix_fmt;
	}

	return 0;
}

void ff::codec_base::get_current_audio_channel(::AVChannelLayout* dst) const
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

void ff::codec_base::get_current_video_info(video_info& info) const
{
	info.height = codec_ctx->height;
	info.width = codec_ctx->width;
	info.pix_fmt = codec_ctx->pix_fmt;
}

void ff::codec_base::get_current_audio_info(audio_info& info) const
{
	info.sample_fmt = codec_ctx->sample_fmt;
	info.sample_rate = codec_ctx->sample_rate;

	int err = 0;
	if ((err = av_channel_layout_copy(&info.ch_layout, &codec_ctx->ch_layout)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not copy the channel layout.", err)
	}
}

void ff::codec_base::configure_multithreading(int num_threads)
{
	codec_ctx->thread_count = num_threads;
}
