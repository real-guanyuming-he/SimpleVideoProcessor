#pragma once

struct AVRational;

namespace ff
{
	// time in hours mins and secs
	struct time
	{
		time() = default;
		constexpr time(int h, int m, float s) : hours(h), mins(), secs(s) {}
		// from seconds
		time(double secs) noexcept;
		// The user should not care about this ctor.
		// Builds the object from an AVRational
		time(const struct ::AVRational& t) noexcept;

		time(const time&) = default;
		time& operator=(const time&) = default;

		int hours = 0, mins = 0;
		float secs = 0.f;

	public:
		constexpr double to_seconds() const
		{
			return (double)hours * 3600.0 + (double)mins * 60.0 + secs;
		}
	private:

	};

	constexpr time zero_time{};
}