/*
* Codec.h:
* Defines the common ground for decoder/encoders.
*/

#pragma once

struct AVRational;
struct AVCodec;
struct AVCodecContext;
struct AVChannelLayout;

namespace ff
{
	class codec_base
	{
	public:
		codec_base() = default;
		codec_base(struct ::AVCodec* c, struct ::AVCodecContext* ctx) : codec(c), codec_ctx(ctx) {}
		virtual ~codec_base() = default;

	public:
		// The method combines the info from the constructors
		// and from the corresponding media passed in as a parameter
		// Then, it creates the codec and its context
		virtual void create(const class media& m) = 0;
		// Destroys the codec ctx (the codec doesn't need to be destroyed)
		virtual void destroy();

	public:
		// @returns the pixel format the codec uses currently, or undefined if the codec is not created yet.
		int get_current_pixel_format() const;

		// @param dst: the channel layout the codec uses currently will be copied there, or undefined if the codec is not created yet.
		void copy_current_audio_channel(::AVChannelLayout* dst) const;

		// @returns a pointer to the channel layout the codec uses currently. nullptr if the codec is not created.
		const ::AVChannelLayout* get_current_audio_channel() const;

		// @returns the audio sample rate the codec uses currently, or undefined if the codec is not created yet.
		int get_current_sample_rate() const;

		// @returns the audio sample format the codec uses currently, or undefined if the codec is not created yet.
		int get_current_audio_sample_format() const;

		// @returns a pointer to the time base the codec uses currently. nullptr if the codec is not created.
		const struct AVRational* get_current_time_base() const;

	public:
		const struct ::AVCodec* get_codec() const { return codec; }
		struct ::AVCodecContext* get_codec_ctx() const { return codec_ctx; }

	protected:
		/*
		* Configures multithreading settings for the codec.
		* @param num_threads: the number of threads to use. If it's 0, the the number will be determined by the CPU's core number
		* If it's 1, then multithreading is not used.
		*/
		void configure_multithreading(int num_threads = 0);

	protected:
		const struct ::AVCodec* codec = nullptr;
		struct ::AVCodecContext* codec_ctx = nullptr;
	};
}
