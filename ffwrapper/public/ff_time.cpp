#include "../private/ff_math_helpers.h"

#include "ff_time.h"

ff::time::time(double secs) noexcept
{
	// as an integer. The numbers after the point will not be lost. They are added at the double seconds assignment.
	long all_seconds = (long)secs;

	long all_minutes = all_seconds / 60;
	long all_hours = all_minutes / 60;

	double seconds = static_cast<double>(all_seconds % 60) + secs - (double)all_seconds;
	int minutes = all_minutes % 60;

	hours = all_hours;
	mins = minutes;
	secs = seconds;
}

ff::time::time(const AVRational& t) noexcept : time(ffhelpers::av_rational_to_double(t)) {}