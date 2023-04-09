/*
* remux.cpp: Defines remux()
*/

#include <inttypes.h>
#include <stdint.h>

#include <iostream>
#include "../ffwrapper/public/media.h"
#include "../ffwrapper/public/frame.h"
#include "../ffwrapper/public/demuxer.h"
#include "../ffwrapper/public/muxer.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

constexpr auto input_file_name = "D:\\GameRec\\Doom Eternal\\lv1.mp4";
constexpr auto output_file_name = "remux_output.mp4";

/*
* Remuxes in_file into out_file since start_time.
* Assuems that in_file and out_file have the same extension.
*/
void remux(const char* in_file, const char* out_file, double start_time = 0.0)
{
	try
	{
		ff::input_media input(in_file);
		if (input.num_streams() == 0)
		{
			throw std::runtime_error("Input file does not contain any media streams.");
		}
		ff::demuxer dem(input);

		ff::output_media output(out_file);
		ff::muxer mux(output);

		// select the key stream in this way:
		// if the media has video streams, then it's the key video stream.
		// otherwise, if the media has audio streams, then it's the key audio stream.
		// otherwise, it's the first stream.
		ff::input_stream key_input_stream;
		int key_input_stream_ind = -1;
		if (input.has_videos())
		{
			key_input_stream_ind = input.get_video_i(0);

		}
		else if (input.has_audios())
		{
			key_input_stream_ind = input.get_audio_i(0);
		}
		else
		{
			key_input_stream_ind = 0;
		}
		key_input_stream = input.get_stream(key_input_stream_ind);

		int port_num = -1;
		
		// If the user wants to seek, then seek.
		if (start_time != 0.0)
		{
			port_num = dem.seek(ff::seconds_to_time_in_base(start_time, key_input_stream.get_time_base()), key_input_stream_ind);
		}
		std::vector<int64_t> start_pts;

		// For each input stream, use its info to create an output stream
		for (int i = 0; i != input.num_streams(); ++i)
		{
			ff::output_stream ostream = output.add_stream(input.get_stream(i));
			start_pts.push_back(ff::seconds_to_time_in_base(start_time, ostream->time_base));
		}
		mux.write_file_header();

		// Feed packets to muxer
		// Use do-while because first the packet from seek should be fed.
		do
		{
			if (port_num != -1)
			{
				ff::packet pkt(dem.get_port(port_num).try_get_one());

				_ASSERT(pkt.is_valid());
				ff::output_stream ostream = output.get_stream(port_num);

				pkt->stream_index = ostream->index;
				pkt->time_base = ostream->time_base;

				pkt.rescale_time(input.get_stream(port_num), output.get_stream(port_num));
				pkt->dts -= start_pts[port_num];
				pkt->pts -= start_pts[port_num];
				pkt->pos = -1;

				mux.try_feed(pkt);
			}
		} while ((port_num = dem.demux_next_packet()) != -1);

		mux.finalize();
	}
	catch (const std::runtime_error& e)
	{
		std::cout << std::string("ERROR: ") + e.what() << std::endl;
	}
}