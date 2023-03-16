/*
* Decoder.h:
* Defines decoder classes for decoding multimedia streams.
*/

#pragma once

#include "codec.h"
#include "media.h"
#include "ff_time.h"

struct AVCodec;
struct AVCodecContext;

namespace ff
{
	/*
	* Base class for all decoders
	*/
	class decoder : public codec_base
	{
	public:
		decoder() = default;
		~decoder() = default;

	public:
		/*
		* Feeds a packet to the decoder for decoding.
		* 
		* @returns 0 on success. Otherwise:
		AVERROR(EAGAIN): input is not accepted in the current state - user must read output with avcodec_receive_frame() (once all output is read, the packet should be resent, and the call will not fail with EAGAIN). 
		AVERROR_EOF: the decoder has been flushed, and no new packets can be sent to it (also returned if more than 1 flush packet is sent) 
		AVERROR(EINVAL): codec not opened, it is an encoder, or requires flush 
		AVERROR(ENOMEM): failed to add packet to internal queue, or similar 
		Other errors: legitimate decoding errors
		*/
		int feed_a_packet(struct packet& pkt);

		/*
		* Decodes the next frame and put it in f.
		*
		* @returns true iff there is a next frame; false if there are no more or the decoder is not ready.
		*/
		virtual bool decode_next_frame(struct frame& f) = 0;
	};

	/*
	* decoder for input media streams
	*/
	class input_decoder : public decoder
	{
	public:
		input_decoder() = default;
		// Creates the decoder directly from st and fmt.
		input_decoder(const input_stream& st, struct ::AVFormatContext* fmt);
		// Binds it to m's stream si.
		input_decoder(const input_media& m, int si) : input_decoder(m.get_stream(si), m.get_format_ctx()) {}

		~input_decoder() { destroy(); }

	public:
		// If it's bound and not already created, then the method creates the decoder for the stream bound;
		void create(const class media& m) override;
		/*
		* Destroys the decoder and unbinds it from any binding.
		*/
		inline void destroy() noexcept { decoder::destroy(); clear_binding(); }

		// Is the decoder bound to some stream?
		bool is_bound() const { return nullptr != fmt_ctx; }
		// Is the decoder ready for decoding?
		bool is_ready() const { return is_bound() && codec_ctx != nullptr; }

	public:
		// Gets the duration of the stream (from s). If not bound, returns a zero time.
		time get_duration() const;
		/*
		*  If the decoder is ready and t is within the duration, then seeks to t for the bound stream,
		* and in addition decodes the frame within the time sought to.
		* 
		* Note: although it seeks for the stream bound, the whole file is probably sought to that place.
		* @param t the time to seek to
		* @param f in which the decoded frame is put
		* @returns true iff the the condition for seeking is satisfied.
		* @throws std::runtime_error on failure
		*/
		bool seek(double t, struct frame& f);
		/*
		* Decodes the next frame and put it in f.
		* 
		* @returns true iff there is a next frame; false if there are no more or the decoder is not ready.
		* @throws std::runtime_error if the decoding failed.
		*/
		bool decode_next_frame(struct frame& f);

	private:
		// If EOF is reached for the first time, then the method is called to enter draining mode 
		// and retriving the first frame after entering the mode (if there are any).
		// @returns true iff there are some frames buffered to drain.
		bool drain_decoder(struct frame& f);

	private:
		// Clears the binding but does not do anything to the decoder.
		inline void clear_binding() noexcept
		{
			fmt_ctx = nullptr;
		}

	private:
		// The input stream.
		input_stream s;
		// the format context of the media file.
		struct ::AVFormatContext* fmt_ctx = nullptr;

		// Generally EOF can only be reached once per stream, which then further requires draining the buffered packets once.
		// As the method only extracts one frame per call while multiple can be buffered there, it's necessary to have this to indicate
		// if it's the first time EOF is reached.
		// See https://ffmpeg.org/doxygen/5.1/group__lavc__encdec.html
		bool is_eof_reached = false;
	};
}