#pragma once

extern "C"
{
#include <libavutil/rational.h>
}

#include "../private/ff_math_helpers.h"

namespace ff
{
	// An encapsulation of ffmpeg's time type.
	// num: its numerator
	// den: its denominator
	// Equals to num/den seconds
	using time = ::AVRational;

	constexpr time zero_time{ 0,0 };

	// @returns the time in seconds represented by t
	inline constexpr double time_to_seconds(time t)
	{
		return ffhelpers::av_rational_to_double(t);
	}

	// @returns the time in seconds represented by t in time base b
	inline constexpr double time_in_base_to_seconds(int64_t t, AVRational b)
	{
		return ffhelpers::ff_time_in_base_to_seconds(t, b);
	}

	inline constexpr int64_t seconds_to_time_in_base(double s, AVRational b)
	{
		return ffhelpers::ff_seconds_to_time_in_base(s, b);
	}

	// time in hours mins and secs
	struct time_hms
	{
		time_hms() = default;
		constexpr time_hms(int h, int m, float s) : hours(h), mins(), secs(s) {}
		// from seconds
		time_hms(double secs) noexcept;
		time_hms(time t) noexcept;

		time_hms(const time_hms&) = default;
		time_hms& operator=(const time_hms&) = default;

		int hours = 0, mins = 0;
		float secs = 0.f;

	public:
		constexpr double to_seconds() const
		{
			return (double)hours * 3600.0 + (double)mins * 60.0 + secs;
		}
	private:

	};

	constexpr time_hms zero_time_hms{};


}