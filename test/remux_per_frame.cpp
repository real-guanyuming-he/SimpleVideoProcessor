/*
* remux_per_frame.cpp: Defines remux_per_frame()
*/

#include <inttypes.h>
#include <stdint.h>

#include <iostream>
#include "../ffwrapper/public/media.h"
#include "../ffwrapper/public/frame.h"
#include "../ffwrapper/public/demuxer.h"
#include "../ffwrapper/public/muxer.h"
#include "../ffwrapper/public/encoder.h"
#include "../ffwrapper/public/decoder.h"
#include "../ffwrapper/public/image_converter.h"
#include "../ffwrapper/public/audio_resampler.h"
#include "../ffwrapper/public/packet_retimer.h"
#include "../ffwrapper/private/utility/info.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

void remux_per_frame(const char* in_file, const char* out_file, double start_time = 0.0)
{
	try
	{
		ff::input_media input(in_file);
		if (input.num_streams() == 0)
		{
			throw std::runtime_error("Input file does not contain any media streams.");
		}
		ff::demuxer dem(input);

		std::vector<ff::encoder*> encoders;
		std::vector<ff::decoder*> decoders;

		ff::output_media output(out_file);
		ff::muxer mux(output);

		for (int i = 0; i != input.num_streams(); ++i)
		{
			decoders.push_back(new ff::input_decoder(dem.get_port(i)));
			encoders.push_back(new ff::direct_encoder(*decoders[i], output, input.get_stream(i)));
		}

		// It is possible that the formats and the time bases between input and output
		// are actually different even if they are of the same codec ID.
		// Therefore, these converters have to be prepared for any stream mapping needed.
		// After initialization, at each slot, the pointer will be
		// nullptr if the converter is not needed, or the converter initialized.
		std::vector<void*> converters;
		// same for packet retimers
		std::vector<ff::packet_retimer*> pkt_retimers;

		// Do not prepare retimers now because the time base of out streams may be changed
		// during writing the file header.

		// prepare converters
		for (int i = 0; i != input.num_streams(); ++i)
		{
			// image converters
			if (input.get_stream(i).is_video())
			{
				ff::video_info src_info, dst_info;
				decoders[i]->get_current_video_info(src_info); encoders[i]->get_current_video_info(dst_info);
				if (src_info == dst_info)
				{
					converters.push_back(nullptr);
				}
				else
				{
					converters.push_back((void*)new ff::image_converter(*decoders[i], *encoders[i], SWS_BILINEAR));
				}
			}
			// audio resamplers
			else if (input.get_stream(i).is_audio())
			{
				ff::audio_info src_info, dst_info;
				decoders[i]->get_current_audio_info(src_info); encoders[i]->get_current_audio_info(dst_info);
				if (src_info == dst_info)
				{
					converters.push_back(nullptr);
				}
				else
				{
					converters.push_back((void*)new ff::audio_resampler(*decoders[i], *encoders[i]));
				}
			}

		}

		// Declare the demuxer port number earlier because it'll be used in seeking to indicate
		// the port of the packet that's put in the queue.
		int port_num = -1;

		// seeking
		{
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

			// If the user wants to seek, then seek.
			if (start_time != 0.0)
			{
				port_num = dem.seek(ff::seconds_to_time_in_base(start_time, key_input_stream.get_time_base()), key_input_stream_ind);
			}
		}

		// For each input stream, use its info to create an output stream
		for (int i = 0; i != input.num_streams(); ++i)
		{
			ff::output_stream ostream = output.add_stream(*encoders[i]);
		}
		mux.write_file_header();

		// prepare packet retimers
		// Now the time bases can never change
		for (int i = 0; i != input.num_streams(); ++i)
		{
			// A packet's time fields are based on the stream's time base,
			// not that of itself (and the field can be 0).

			ff::time current = input.get_stream(i).get_time_base();
			// the output stream's info is obtained from the encoder.
			// since the stream is not created, just use the encoder's
			ff::time target = output.get_stream(i).get_time_base();

			pkt_retimers.push_back(new ff::packet_retimer
			(
				current, target, start_time
			));
		}

		// Feed packets to muxer
		// Use do-while because first the packet from seek should be fed.
		do
		{
			if (port_num != -1)
			{
				ff::packet pkt(dem.get_port(port_num).try_get_one());

				_ASSERT(pkt.is_valid());
				ff::output_stream ostream = output.get_stream(port_num);

				auto* dec = decoders[port_num]; 
				auto* enc = encoders[port_num];

				dec->try_feed(pkt);
				pkt.unref();

				ff::frame frame;
				while ((frame = dec->try_get_one()).is_valid())
				{
					// seeking will give us frames of time before the time we want. Just discard them.
					double st = ff::time_in_base_to_seconds(frame->pts, input.get_stream(port_num).get_time_base());
					if (st < start_time)
					{
						// Don't forget this
						frame.unref();
						continue;
					}

					if (converters[port_num] != nullptr)
					{
						ff::frame dst_frame;

						if (ostream.is_video())
						{
							auto* img_converter = (ff::image_converter*)converters[port_num];
							img_converter->convert(frame, dst_frame);
						}
						else if (ostream.is_audio())
						{
							auto* aud_resampler = (ff::audio_resampler*)converters[port_num];
							aud_resampler->convert(frame, dst_frame);
						}
						enc->try_feed(dst_frame);
					}
					else
					{
						enc->try_feed(frame);
					}
					// Don't forget this
					frame.unref();

					ff::packet out_pkt;
					while ((out_pkt = enc->try_get_one()).is_valid())
					{
						out_pkt->stream_index = ostream->index;
						out_pkt->pos = -1;

						// If the packet needs to be retimed, then retime it.
						if (pkt_retimers[port_num] != nullptr)
						{
							pkt_retimers[port_num]->retime(out_pkt);
							out_pkt->time_base = output.get_stream(port_num).get_time_base();
						}
						mux.try_feed(out_pkt);
					}
				}
			}
		} while ((port_num = dem.demux_next_packet()) != -1);

		mux.finalize();
	}
	catch (const std::runtime_error& e)
	{
		std::cout << std::string("ERROR: ") + e.what() << std::endl;
	}
}