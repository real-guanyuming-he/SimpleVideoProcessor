#pragma once

#include "interfaces/src_sink.h"
#include "media.h"

struct AVFormatContext;

namespace ff
{
	/*
	* Muxing work flow:
	* 
	* 1. Create a output_media with its file path. The format will be deducted from its extension name.
	The output_media will give a list of encoder IDs, and each ID is for encoding a particular type (e.g. video,audio) of frames for that media. 

	* 2. Create encoders for all type of frames used with the IDs returned in 1. Then, create streams, and provide each of them the encoder
	matching its type.

	* 3. After streams are created, we have all the info we need for the file's header. Now call write_file_header() to write it.
	* 
	* 4. Repeatedly feed encoded packets from the encoders by calling try_feed(). The order in which the packets are fed is unimportant.
	* 
	* 5. After all packets are fed, call finalize().
	*/
	class muxer : public packet_sink
	{
	public:
		explicit muxer(::AVFormatContext* fmt) : fmt_ctx(fmt) {}
		muxer(const output_media& m) : muxer(m.get_format_ctx()) {}
		virtual ~muxer() = default;

	public:
		/*
		* Called to initialize the output media file.
		 Packets cannot be fed to the file until this is called.

		 Note: After doing this, the media format cannot be modified.
		Any modification like adding streams should be done before calling this.

		* @throws std::runtime_error on failure.
		*/
		void write_file_header();

		/*
		* Feeds a packet to the output file.
		* 
		* @returns always true.
		* @throws std::runtime_error on failure.
		*/
		bool try_feed(ff::packet& pkt) override;

		/*
		* Finalizes the output media. After calling this, the output file will be ready and
		* the user should not do anything to it except reading the exisiting info.
		*/
		void finalize();

	protected:
		// Does not own this. Just for referencing.
		::AVFormatContext* fmt_ctx;
	};
}