#include "info.h"

#include "../ff_helpers.h"
#include <stdexcept>

ff::audio_info::audio_info(int fmt, const AVChannelLayout& layout, int rate) : sample_fmt(fmt), sample_rate(rate)
{
	int ret;
	if ((ret = av_channel_layout_copy(&ch_layout, &layout)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not copy channel layout.", ret);
	}
}

ff::audio_info::audio_info(const audio_info& other) : sample_fmt(other.sample_fmt), sample_rate(other.sample_rate)
{
	int ret;
	if ((ret = av_channel_layout_copy(&ch_layout, &other.ch_layout)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not copy channel layout.", ret);
	}
}

ff::audio_info& ff::audio_info::operator=(const audio_info& right)
{
	sample_fmt = right.sample_fmt;
	sample_rate = right.sample_rate;

	int ret;
	if ((ret = av_channel_layout_copy(&ch_layout, &right.ch_layout)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not copy channel layout.", ret);
	}

	return *this;
}

bool ff::audio_info::operator==(const audio_info& right) const
{
	return 
		!av_channel_layout_compare(&ch_layout, &right.ch_layout) &&
		sample_fmt == right.sample_fmt &&
		sample_rate == right.sample_rate;
}

bool ff::video_info::operator==(const video_info& right) const
{
	return
		pix_fmt == right.pix_fmt &&
		width == right.width &&
		height == right.height;
}
