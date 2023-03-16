#pragma once

struct SwrContext;
struct AVChannelLayout;

namespace ff
{
	/*
	* Resamples or converts the format of audio
	*/
	class audio_resampler
	{
	public:
		// Allocs the swr_ctx
		// @throws std::runtime error on failure
		audio_resampler();

		/*
		* Allocs the swr_ctx, and inits it with
		* @param src_fmt: src audio sample format
		* @param src_sample_rate: literally
		* @param: src_ch_layout: src channel layout
		* @param dst_fmt: dst audio sample format
		* @param dst_sample_rate: literally
		* @param: dst_ch_layout: dst channel layout
		*/
		audio_resampler
		(
			int src_fmt, int src_sample_rate, const ::AVChannelLayout* src_ch_layout,
			int dst_fmt, int dst_sample_rate, const ::AVChannelLayout* dst_ch_layout
		);

		/*
		* Allocs the swr_ctx, and inits it with the info from
		* @param dec: the decoder for the source audio
		* @param enc: the encoder for the destination audio
		*/
		audio_resampler(const class decoder& dec, const class encoder& enc);

		~audio_resampler();

	public:
		/*
		* Converts an audio frame in src audio format to an audio frame in dst audio format.
		* @param src_frame: the source frame, whose nb_samples must be set
		* @param dst_frame: the destination frame, 
		which must have an allocated buffer larger enough for converted data from calling create_audio_buffer.
		* @returns number of converted samples per channel. The number will also override dst_frame->nb_samples if it's strictly less than that.
		*/
		int convert(const struct frame& src_frame, struct frame& dst_frame);

	public:
		/*
		* If src_rate != dst_rate, then they will have different numbers of samples per channel.
		* @returns This function calculates the required number of samples per channel in the dst buffer to store the converted samples, 
		rounded up to the nearest integer.
		*/
		int calculate_dst_num_samples(int src_num_samples) const;

	private:
		::SwrContext* swr_ctx;

		// src sample rate and dst sample rate
		int src_rate, dst_rate;
	};


}