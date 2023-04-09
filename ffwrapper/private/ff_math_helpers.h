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

	constexpr int common_audio_sample_rate = 44100;

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

	// @returns how many number of bases secs equals to
	inline constexpr int64_t ff_seconds_to_time_in_base(double secs, AVRational base)
	{
		return static_cast<int64_t>(secs * (double)base.den / (double)base.num);
	}

}

// @returns true if r1 == r2. Behaviour is undefined if r1 or r2 is invalid (i.e. den = 0)
constexpr bool operator==(AVRational r1, AVRational r2)
{
	// a/b = c/d -> ad = cb
	return r1.num * r2.den == r1.den * r2.num;
}

// @returns true if r1 != r2. Behaviour is undefined if r1 or r2 is invalid (i.e. den = 0)
constexpr bool operator!=(AVRational r1, AVRational r2)
{
	// Given b,d \ne 0, a/b = c/d <-> ad = cb
	return r1.num * r2.den != r1.den * r2.num;
}