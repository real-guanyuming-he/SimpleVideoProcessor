extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "frame.h"

#include "../private/ff_helpers.h"

#include <stdexcept>

ff::frame::frame(const frame& other)
{
	allocate();

	av_frame_copy_all(buffer, other.buffer);
}

ff::frame::~frame()
{
	destroy();
}

bool ff::frame::has_a_buffer() const 
{ 
	return buffer->buf[0] != nullptr; 
}

void ff::frame::allocate()
{
	if (buffer == nullptr)
	{
		buffer = av_frame_alloc();
		if (!buffer)
		{
			ON_FF_ERROR("Could not allocate AVFrame.")
		}
	}
}

void ff::frame::destroy()
{
	ffhelpers::safely_free_frame(&buffer);
}

void ff::frame::create_video_buffer(int width, int height, int pix_fmt, int alignment)
{
	buffer->width = width;
	buffer->height = height;
	buffer->format = pix_fmt;

	int ret;
	if ((ret = av_frame_get_buffer(buffer, alignment)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not allocate the buffer of a frame", ret)
	}
}

void ff::frame::create_audio_buffer(int nb_samples, int sample_format, const AVChannelLayout* ch_layout, int alignment)
{
	buffer->nb_samples = nb_samples;
	buffer->format = sample_format;
	int ret;
	if ((ret = av_channel_layout_copy(&buffer->ch_layout, ch_layout)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not copy channel layout", ret)
	}

	if ((ret = av_frame_get_buffer(buffer, alignment)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not allocate the buffer of a frame", ret)
	}
}

void ff::frame::av_frame_copy_all(::AVFrame* dst, ::AVFrame* src)
{
	if (av_frame_copy_props(dst, src) < 0)
	{
		ON_FF_ERROR("Could not copy AVFrame prop.")
	}
	if (av_frame_copy(dst, src) < 0)
	{
		ON_FF_ERROR("Could not copy AVFrame.")
	}
}

void ff::packet::allocate()
{
	pkt = av_packet_alloc();
	if (!pkt)
	{
		ON_FF_ERROR("Could not alloc packet.")
	}
}

void ff::packet::unref()
{
	av_packet_unref(pkt);
}

void ff::packet::destroy()
{
	ffhelpers::safely_free_packet(&pkt);
}