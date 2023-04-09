/*
* info.h 
* defines structs for basic video/audio information
*/

#pragma once

extern "C"
{
#include <libavformat/avformat.h>
}

namespace ff
{
	struct video_info
	{
		constexpr video_info(): 
			pix_fmt(AV_PIX_FMT_NONE), width(-1), height(-1) {}
		constexpr video_info(int fmt, int w, int h) :
			pix_fmt(fmt), width(w), height(h) {}

		video_info(const video_info&) = default;
		video_info& operator=(const video_info&) = default;

		~video_info() = default;

		// If two different video formats are the same
		bool operator==(const video_info&) const;

		constexpr bool valid() const { return AV_PIX_FMT_NONE != pix_fmt; }

		int pix_fmt;
		int width, height;
	};

	struct audio_info
	{
		constexpr audio_info() :
			sample_fmt(AV_SAMPLE_FMT_NONE), ch_layout(), sample_rate(0) {}
		audio_info(int fmt, const AVChannelLayout& layout, int rate);

		audio_info(const audio_info& other);
		audio_info& operator=(const audio_info&);

		~audio_info() = default;
		
		// If two different audio formats are the same
		bool operator==(const audio_info&) const;

		constexpr bool valid() const { return AV_SAMPLE_FMT_NONE != sample_fmt; }

		AVChannelLayout ch_layout;
		int sample_fmt;
		int sample_rate;
	};
}
