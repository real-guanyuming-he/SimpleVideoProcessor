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
		codec_base(::AVCodec* c, ::AVCodecContext* ctx) : codec(c), codec_ctx(ctx) {}
		virtual ~codec_base() = default;

	public:
		/*
		* Requires that codec and codec_ctx are initialized.
		* Requires additionally that codec_ctx has been filled with necessary information.
		* 
		* Creates the codec by calling avcodec_open2.
		* Throws std::runtime_error on error.
		*/
		virtual void create();

		// Destroys the codec ctx, but not the codec.
		virtual void destroy();

		/*
		* Flushes the codec.
		* Can be used when, for example, draining is needed, or when a seeking is done.
		*/
		virtual void flush_codec();

		/*
		* Starts draining to ouput all of the buffered data.
		*/
		virtual void start_draining() = 0;

	public:
		// @returns the pixel format the codec uses currently, or undefined if the codec is not created yet.
		int get_current_pixel_format() const;

		// @param dst: the channel layout the codec uses currently will be copied there, or undefined if the codec is not created yet.
		void get_current_audio_channel(::AVChannelLayout* dst) const;

		// @returns a pointer to the channel layout the codec uses currently. nullptr if the codec is not created.
		const ::AVChannelLayout* get_current_audio_channel() const;

		// @returns the audio sample rate the codec uses currently, or undefined if the codec is not created yet.
		int get_current_sample_rate() const;

		// @returns the audio sample format the codec uses currently, or undefined if the codec is not created yet.
		int get_current_audio_sample_format() const;

		// @returns a pointer to the time base the codec uses currently. nullptr if the codec is not created.
		const struct AVRational* get_current_time_base() const;

		// Fills info with the codec's video information. Behaviour is undefined if it's not ready or if it's not for video.
		void get_current_video_info(struct video_info& info) const;
		// Fills info with the codec's audio information. Behaviour is undefined if it's not ready or if it's not for audio.
		void get_current_audio_info(struct audio_info& info) const;

	public:
		const ::AVCodec* get_codec() const { return codec; }
		::AVCodecContext* get_codec_ctx() const { return codec_ctx; }

		// @returns true iff the codec is ready for encoding/decoding
		virtual bool ready() const { return codec_ctx != nullptr; }

	protected:
		/*
		* Configures multithreading settings for the codec.
		* @param num_threads: the number of threads to use. If it's 0, then the number will be determined by the CPU's core number
		* If it's 1, then multithreading is not used.
		*/
		void configure_multithreading(int num_threads = 0);

	protected:
		const ::AVCodec* codec = nullptr;
		::AVCodecContext* codec_ctx = nullptr;
	};
}
