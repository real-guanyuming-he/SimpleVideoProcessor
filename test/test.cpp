// test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS

#include <inttypes.h>
#include <stdint.h>

#include <iostream>
#include "../ffwrapper/public/media.h"
#include "../ffwrapper/public/frame.h"
#include "../ffwrapper/public/decoder.h"
#include "../ffwrapper/public/encoder.h"
#include "../ffwrapper/public/audio_fifo.h"
#include "../ffwrapper/public/audio_resampler.h"
#include "../ffwrapper/public/image_converter.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

constexpr auto input_file_name = "D:\\GameRec\\Doom Eternal\\lv1.mp4";
constexpr auto remux_output_file_name = "remux_output.mp4";

void remux(const char* in_file, const char* out_file, double start_time);

int main()
{
    /*try
    {
        ff::input_media input(input_file_name);
        ff::input_stream ivs = input.get_stream(input.get_video_i(0));
		ff::input_stream ias = input.get_stream(input.get_audio_i(0));

		ff::output_media output(std::string("output") + ".avi");

        ff::input_decoder video_decoder(input, input.get_video_i(0));

		ff::input_decoder audio_decoder(input, input.get_audio_i(0));

		ff::transcode_encoder video_encoder(video_decoder, output.get_deduced_codec_id(AVMEDIA_TYPE_VIDEO));
		auto pix_fmt = video_encoder.get_current_pixel_format();

		ff::transcode_encoder audio_encoder(audio_decoder, output.get_deduced_codec_id(AVMEDIA_TYPE_AUDIO));

		ff::frame vid_frame;
		ff::frame aud_frame;

		int64_t vid_initial_pts = 0;
		int64_t aud_pts = 0;

		ff::image_converter converter(video_decoder, video_encoder, SWS_BILINEAR);

		ff::audio_resampler resampler(audio_decoder, audio_encoder);
		ff::audio_fifo fifo{ audio_encoder };

		auto o_vid_stream = output.add_stream(video_encoder);
		auto o_aud_stream = output.add_stream(audio_encoder);

		auto feed_vid_frames_to_encoder = [&]() -> void
		{
			do
			{
				ff::frame converted_vid_frame;
				converted_vid_frame.create_video_buffer(vid_frame->width, vid_frame->height, pix_fmt, 1);

				converter.convert(vid_frame, converted_vid_frame);

				// We can do this since we are not going to change the fps and speed.
				converted_vid_frame->pts = vid_frame->pts - vid_initial_pts;

				video_encoder.feed_a_frame(converted_vid_frame);

				ff::packet pkt;
				while (video_encoder.encode_next_packet(pkt))
				{
					pkt->stream_index = o_vid_stream->index;

					av_packet_rescale_ts(pkt, ivs->time_base, o_vid_stream->time_base);

					output.feed_packet(pkt);
				}
			} while (video_decoder.decode_next_frame(vid_frame));
		};

		auto target_audio_format = audio_encoder.get_current_audio_sample_format();
		auto target_audio_channel = audio_encoder.get_current_audio_channel();
		auto target_audio_sample_rate = audio_encoder.get_current_sample_rate();
		auto target_num_samples = audio_encoder.get_required_number_of_samples_per_channel();
		auto target_time_base = audio_encoder.get_current_time_base();

		auto feed_aud_frames_to_encoder = [&]() -> void
		{
			do
			{
				ff::frame converted_frame;
				converted_frame.create_audio_buffer
				(
					resampler.calculate_dst_num_samples(aud_frame->nb_samples),
					target_audio_format,
					target_audio_channel
				);
				converted_frame->sample_rate = target_audio_sample_rate;
				resampler.convert(aud_frame, converted_frame);

				fifo.write(converted_frame);

				while (fifo.size() >= target_num_samples)
				{
					ff::frame frame_to_be_encoded;
					frame_to_be_encoded.create_audio_buffer
					(
						target_num_samples,
						target_audio_format,
						target_audio_channel
					);
					fifo.read(frame_to_be_encoded);

					// Unlike how we set vid_frame->pts
					frame_to_be_encoded->pts = aud_pts;
					aud_pts += // num samples / sample rate * (1/time base)
						frame_to_be_encoded->nb_samples / target_audio_sample_rate 
						* target_time_base->den / target_time_base->num;
					audio_encoder.feed_a_frame(frame_to_be_encoded);

					ff::packet pkt;
					while (audio_encoder.encode_next_packet(pkt))
					{
						pkt->stream_index = o_aud_stream->index;

						av_packet_rescale_ts(pkt, ias->time_base, o_aud_stream->time_base);

						output.feed_packet(pkt);
					}
				}

			} while ((audio_decoder.decode_next_frame(aud_frame)));

			// there may still be data in the fifo. Retrieve it out.
			if (fifo.size() > 0)
			{
				ff::frame last_aud_frame;
				last_aud_frame.create_audio_buffer
				(
					fifo.size(),
					target_audio_format,
					target_audio_channel
				);

				fifo.read(last_aud_frame);

				last_aud_frame->pts = aud_pts;
				aud_pts += // num samples / sample rate * (1/time base)
					last_aud_frame->nb_samples / target_audio_sample_rate
					* target_time_base->den / target_time_base->num;
				audio_encoder.feed_a_frame(last_aud_frame);

				ff::packet pkt;
				while (audio_encoder.encode_next_packet(pkt))
				{
					pkt->stream_index = o_aud_stream->index;

					av_packet_rescale_ts(pkt, ias->time_base, o_aud_stream->time_base);

					output.feed_packet(pkt);
				}
			}
		};

		output.write_file_header();

		video_decoder.seek(ivs.duration - 30., vid_frame);
		vid_initial_pts = vid_frame->pts;

		feed_vid_frames_to_encoder();

		audio_decoder.seek(ivs.duration - 30., aud_frame);
		aud_pts = 0;

		feed_aud_frames_to_encoder();

		output.finalize();
        
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }*/

	remux(input_file_name, remux_output_file_name, 1200.0);

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
