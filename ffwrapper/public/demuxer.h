/*
* demuxer.h:
* Defines the demuxer class and the demuxer_port class.
*/

#pragma once

#include "interfaces/queue_src.h"
#include "media.h"
#include "ff_time.h"

#include <vector>

struct AVFormatContext;

namespace ff
{
	/*
	* A demuxer port outputs the packets from a specific stream of the file.
	*/
	struct demuxer_port : public queue_packet_source
	{
	public:
		// A demuxer port must be associated with a demuxer.
		// but it can be unused.
		demuxer_port() : stream(), in_use(false) {}
		/*
		* Creates a port associated with the demuxer at index.
		*/
		demuxer_port(class demuxer& dem, int index);

	public:
		bool is_used() const { return in_use; }

		// @returns the id of the decoder that is the only option for decoding the packets from here.
		int get_required_decoder_id() const;

		// Adds a packet using the move constructor.
		// Should only be called by its demuxer.
		void add_packet(ff::packet&& pkt);

	public:
		input_stream stream;

		// The user may choose to ignore a particular stream. On this case, the field is false.		
		bool in_use;
	};

	class demuxer
	{
		friend struct demuxer_port;
	public:
		// A demuxer must be associated with a multimedia file.
		demuxer() = delete;
		/*
		* Creates a demuxer associated with media m.
		* @param m: the input media
		* @param unused_ports: the indices of unused ports, in ascending order.
		* Empty by default.
		*/
		demuxer(const input_media& m, const std::vector<int> unused_ports = {});

		// I do not see a reason to enable this now.
		demuxer(const demuxer&) = delete;

	public:
		/*
		* Seeks to time in the time base of the stream of ref_port.
		* 
		* Due to the division of keyframes and other frames, the seeking will stop at the place of the first keyframe before param time.
		* 
		* A keyframe is a frame that is independent in decoding to its previous frames.
		 That is, the decoder does not need information (packets) from previous frames to decode it.
		 From the first keyframe before t, we check one by one and compare its time with the parameter until we find the one we want.
		 Generally speaking, there's no way to just stop before the frame within t.
		*
		* If it succeeds, then the keyframe will be put in its corresponding port.
		* 
		* After a successful seeking, any decoder that was previously decoding packets from any port of the demuxer must be flushed
		* by calling its flush() method.
		* 
		* @returns -1 iff the seeking cannot be done. This can be as a consequence of, for example, the port at ref_port being unused. 
		Otherwise, the index of the port that the packet is put.
		* @throws std::runtime_error on unexpected failure
		*/
		int seek(int64_t time, int ref_port);

		/*
		* Demuxes the next packet and puts it into the corresponding demuxer port.
		* 
		* @returns the port number of the port where the next packet is put, or -1 if there are no more packets.
		*/
		int demux_next_packet();

		const demuxer_port& get_port(int i) const { return ports[i]; }
		demuxer_port& get_port(int i) { return ports[i]; }

	private:

		// For faster access, demuxer ports are stored in an array so that the time to access the port of stream i is O(0).
		std::vector<demuxer_port> ports;

		const input_media& file;
		::AVFormatContext* format_ctx;
	};
}