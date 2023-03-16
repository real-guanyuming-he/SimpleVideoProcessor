/*
* Encoder.h:
* Defines encoders
*/

#pragma once

#include "codec.h"

struct AVChannelLayout;

namespace ff
{
	/*
	* Base class for all encoders
	*/
	class encoder : public codec_base
	{
	public:
		encoder() = default;
		// Finds the codec by name
		encoder(const char* name);
		// Finds the codec by ID
		// (The user doesn't have to care about this as it uses fmpeg's ID)
		encoder(int ID);
		~encoder() = default;

	public:
		/*
		* Feeds a frame to the encoder for decoding
		* 
		* @returns 0 on success, otherwise negative error code: 
		AVERROR(EAGAIN): input is not accepted in the current state - user must read output with avcodec_receive_packet() (once all output is read, the packet should be resent, and the call will not fail with EAGAIN). 
		AVERROR_EOF: the encoder has been flushed, and no new frames can be sent to it 
		AVERROR(EINVAL): codec not opened, it is a decoder, or requires flush 
		AVERROR(ENOMEM): failed to add packet to internal queue, or similar 
		Other errors: legitimate encoding errors
		*/
		int feed_a_frame(struct frame& f);
		/*
		* Decodes the next frame and put it in f.
		*
		* @returns true iff there is a next frame; false otherwise
		(if there are no more or the encoder is not ready or the user needs to feed more)
		*/
		virtual bool encode_next_packet(struct packet& pkt);

		// The frame's pixel format should agree with the encoders.
		// However, in many cases, one knows the pix fmts supported by the encoder first.
		// Therefore, here's a way to get the first of them.
		virtual int get_desired_pixel_format() const;

		// Selects the audio layout of the highest channel count supported by the codec,
		// and copies the layout configuration to dst
		virtual void get_best_audio_channel(::AVChannelLayout* dst) const;

		// @returns the best audio sample rate the codec supports, or 44100 if none.
		virtual int get_best_audio_sample_rate() const;

		// c.f. get_desired_pixel_format
		virtual int get_desired_audio_sample_format() const;

		// @returns the number of samples per channel in one frame to be encoded, required by the encoder.
		int get_required_number_of_samples_per_channel() const;

	private:
		// If EOF is reached for the first time, then the method is called to enter draining mode 
		// and retriving the first packet after entering the mode (if there are any).
		// @returns true iff there are some packet buffered to drain.
		bool drain_encoder(struct packet& pkt);

		// Generally EOF can only be reached once, which then further requires draining the buffered packets once.
		// As the method only extracts one frame per call while multiple can be buffered there, it's necessary to have this to indicate
		// if it's the first time EOF is reached.
		// See https://ffmpeg.org/doxygen/5.1/group__lavc__encdec.html
		bool is_eof_reached = false;
	};

	/*
	* Transcode means to change the format of something.
	* Therefore, the user must provide a decoder 
	(so that the encoder knows the info like height/width that are not changed during transcoding)
	to construct such an encoder.
	*/
	class transcode_encoder : public encoder
	{
	public:
		transcode_encoder() = delete;
		// A encoder of name that transcodes frames decoded by dec.
		transcode_encoder(const class decoder& dec, const char* name);
		// A encoder of ID that transcodes frames decoded by dec.
		transcode_encoder(const class decoder& dec, int ID);

	public:
		void create(const class media& m) override;

	private:
		const class decoder& dec;
	};

	/*
	* Unlike a transcode encoder that uses the settings (like width/height) of a decoder,
	* a general encoder has the uttermost flexibility and thus the user has to specify everything in its constructors.
	*/
	class general_encoder : public encoder
	{

	};
}