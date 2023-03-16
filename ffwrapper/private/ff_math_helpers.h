#pragma once

/*
* ff_math_helpers.h:
* Defines math helpers for ffmpeg programming
*/

extern "C"
{
// We don't really need AVFormat in math helpers, but we need at least one of these man lib headers to prevent a compile error:
// missing -D__STDC_CONSTANT_MACROS / #define __STDC_CONSTANT_MACROS
#include <libavformat/avformat.h>

#include <libavutil/avutil.h>
}

namespace ffhelpers
{
	// If a time base is not provided, one usually can use this instead.
	constexpr AVRational ff_global_time_base{ 1,AV_TIME_BASE };

	// Suitable for 24,25,30,60,120 and many other common fps values.
	constexpr AVRational ff_common_video_time_base{ 1, 600 };

	// Suitable for many common audio sample rates
	constexpr AVRational ff_common_audio_time_base{ 1, 90000 };

	// @returns the time in seconds represented by r
	inline constexpr double av_rational_to_double(AVRational r)
	{
		return (double)r.num / (double)r.den;
	}

	// @returns the time in seconds represented by t in time base b
	inline constexpr double ff_time_in_base_to_seconds(int64_t t, AVRational b)
	{
		return t * av_rational_to_double(b);
	}

	inline constexpr int64_t ff_seconds_to_time_in_base(double secs, AVRational base)
	{
		return static_cast<int64_t>(secs * base.den) / static_cast<int64_t>(base.num);
	}

}