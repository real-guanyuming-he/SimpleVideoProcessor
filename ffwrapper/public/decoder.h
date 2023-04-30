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
struct AVStream;

#include "interfaces/src_sink.h"

namespace ff
{
	/*
	* Base class for all decoders
	*/
	class decoder : public codec_base, public packet_sink, public frame_source
	{
	private:
		using super = codec_base;

	public:
		decoder() = default;
		// Finds the decoder by ID and initializes the codec context
		decoder(int ID);
		// Finds the decoder by name and initializes the codec context
		decoder(const char* name);

		~decoder() = default;

	public:
		/*
		* Feeds a packet to the decoder for decoding.
		* 
		* @param pkt: the packet to feed. Its ownership will not be taken and its content will be copied.
		* Therefore it's necessary for the caller to destroy it after calling the method.
		* 
		* @returns true if the packet is successfully fed;
		* false if no more packets can be fed until some decoded frames are retrieved, 
		or if the decoder is currently in draining mode.
		* 
		* @throws std::runtime_error on failure.
		*/
		bool try_feed(ff::packet& pkt) override;

		/*
		* Tries to decode the next frame.
		*
		* @returns a valid frame if the decoding succeeds;
		* an invalide one if more packets are needed, or if EOF is reached.
		* The caller should check eof() to ascertain if EOF is reached.
		*/
		ff::frame try_get_one() override;

		/*
		* Flushes the decoder and resets its eof state.
		* Can be called after a draining is complete so that the decoder can be reused, or after a seeking is done.
		* Must be called after a seeking is done.
		*/
		void flush_codec() override;

		/*
		* After the packet source for the decoder has no more packets and all available frames until then are extracted,
		* the caller should start draining the decoder to push any buffered data out so that the result is complete.
		* After draining starts, call try_get_one() repeatedly until eof() is true.
		*/
		void start_draining() override;

	public:
		// is eof reached in draining.
		bool eof() const { return eof_reached; }

	protected:
		bool eof_reached = false;
	};

	/*
	* decoder for packets from a demuxer
	*/
	class input_decoder : public decoder
	{
	public:
		input_decoder() = default;
		// Creates the decoder for packets from a demuxer port
		explicit input_decoder(const struct demuxer_port& port);
		explicit input_decoder(const ::AVStream* stream);

		~input_decoder() { destroy(); }
	};
}