extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "media.h"
#include "frame.h"
#include "encoder.h"
#include "decoder.h"
#include "../private/ff_helpers.h"
#include "../private/ff_math_helpers.h"

#include <stdexcept>

ff::encoder::encoder(const char* name)
{
	codec = avcodec_find_encoder_by_name(name);
}

ff::encoder::encoder(int ID)
{
	codec = avcodec_find_encoder((AVCodecID)ID);
}

int ff::encoder::feed_a_frame(frame& f)
{
    return avcodec_send_frame(codec_ctx, f);
}

bool ff::encoder::encode_next_packet(packet& pkt)
{
	// See https://ffmpeg.org/doxygen/5.1/group__lavc__encdec.html

	// try to get a frame from the decoder
	int ret = avcodec_receive_packet(codec_ctx, pkt);

	if (ret >= 0) // we got the frame we want
	{
		return true;
	}
	else if (ret == AVERROR_EOF)
	{
		if (!is_eof_reached) // the first time EOF is reached
		{
			is_eof_reached = true;

			return drain_encoder(pkt);
		}
		else
		{
			// No more frames is available.
			return false;
		}
	}
	else if (ret == AVERROR(EAGAIN)) // we need to feed more packets before we can get a frame.
	{
		return false;
	}

	return false;
}

int ff::encoder::get_desired_pixel_format() const
{
	return get_codec()->pix_fmts[0];
}

bool ff::encoder::drain_encoder(packet& pkt)
{
	// enter draining mode
	int ret = avcodec_send_frame(codec_ctx, nullptr);
	if (ret < 0)
	{
		ON_FF_ERROR("Could not enter the draining mode in decoding.")
	}
	// see if there are buffered packet remaining
	ret = avcodec_receive_packet(codec_ctx, pkt);
	if (ret >= 0) // we got the packet we want
	{
		return true;
	}
	if (ret == AVERROR_EOF) // no more packets
	{
		return false;
	}
	else
	{
		ON_FF_ERROR("Could not get a frame in decoding.")
	}
}

void ff::encoder::get_best_audio_channel(::AVChannelLayout* dst) const
{
	const AVChannelLayout* p, *best_ch_layout = nullptr;
	int best_nb_channels = 0;

	if (!codec->ch_layouts) // If the codec doesn't have any channel layout configuration
	{
		// gives a stereo channel configuration
		AVChannelLayout layout{};
		layout.order = AV_CHANNEL_ORDER_NATIVE;
		layout.nb_channels = 2;
		layout.u.mask = AV_CH_LAYOUT_STEREO;

		if (av_channel_layout_copy(dst, &layout) < 0)
		{
			ON_FF_ERROR("Could not copy audio channel layout.")
		}
	}
	else // If the codec has some
	{
		p = codec->ch_layouts;
		while (p->nb_channels)
		{
			int nb_channels = p->nb_channels;

			if (nb_channels > best_nb_channels) {
				best_ch_layout = p;
				best_nb_channels = nb_channels;
			}
			++p;
		}
		if (!best_ch_layout) // Could not select the best one
		{
			// Then use the first one
			best_ch_layout = codec->ch_layouts;
		}
		if (av_channel_layout_copy(dst, best_ch_layout) < 0)
		{
			ON_FF_ERROR("Could not copy audio channel layout.")
		}
	}

}

int ff::encoder::get_best_audio_sample_rate() const
{
	int best_samplerate = 0;

	if (!codec->supported_samplerates)
	{
		return 44100;
	}

	const int* p = codec->supported_samplerates;
	while (*p) 
	{
		if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
		{
			best_samplerate = *p;
		}
		++p;
	}
	return best_samplerate;
}

int ff::encoder::get_desired_audio_sample_format() const
{
	return codec->sample_fmts ? (int)codec->sample_fmts[0] : (int)AV_SAMPLE_FMT_S16;
}

int ff::encoder::get_required_number_of_samples_per_channel() const
{
	// see its comment
	return codec_ctx->frame_size;
}

ff::transcode_encoder::transcode_encoder(const decoder& d, const char* name) : encoder(name), dec(d)
{
}

ff::transcode_encoder::transcode_encoder(const decoder& d, int ID) : encoder(ID), dec(d)
{
}

void ff::transcode_encoder::create(const media& m)
{
	const output_media& opt_media = static_cast<const output_media&>(m);

	/* Allocate a codec context for the encoder */
	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx)
	{
		ON_FF_ERROR("Failed to allocate the encoder context.")
	}

	/* Copy codec parameters from the decoder and the codec to endoder codec context */
	{
		auto dec_ctx = dec.get_codec_ctx();

		// If the time base is set in the decoder, the we use it in the encoder.
		// If not, we set it according to the type.
		bool time_base_set = false;
		if (dec_ctx->time_base.num != 0 && dec_ctx->time_base.den != 0)
		{
			codec_ctx->time_base = dec_ctx->time_base;
			time_base_set = true;
		}

		//if (dec_ctx->bit_rate != 0)
		//{
		//	 codec_ctx->bit_rate = dec_ctx->bit_rate;
		//}

		switch (codec->type)
		{
		case AVMEDIA_TYPE_VIDEO:
			codec_ctx->pix_fmt = (AVPixelFormat)get_desired_pixel_format();
			codec_ctx->width = dec_ctx->width;
			codec_ctx->height = dec_ctx->height;

			if (!time_base_set) codec_ctx->time_base = ffhelpers::ff_common_video_time_base;
			break;

		case AVMEDIA_TYPE_AUDIO:
			codec_ctx->sample_fmt = (AVSampleFormat)get_desired_audio_sample_format();

			if (dec_ctx->sample_rate > 10000) // If the decoder's sample rate is good enough 
			{
				// then use its
				codec_ctx->sample_rate = dec_ctx->sample_rate;
			}
			else // If the decoder's sample rate isn't good enough
			{
				// Then use the best the codec supports
				codec_ctx->sample_rate = get_best_audio_sample_rate();
			}

			if (dec_ctx->ch_layout.nb_channels > 0)
			{
				av_channel_layout_copy(&codec_ctx->ch_layout, &dec_ctx->ch_layout);
			}
			else
			{
				get_best_audio_channel(&codec_ctx->ch_layout);
			}

			/* Some container formats (like MP4) require global headers to be present.
			* Mark the encoder so that it behaves accordingly. */
			if (opt_media.get_format_ctx()->oformat->flags & AVFMT_GLOBALHEADER)
				codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

			codec_ctx->time_base.num = 1;
			codec_ctx->time_base.den = codec_ctx->sample_rate;
			//if (!time_base_set) codec_ctx->time_base = ffhelpers::ff_common_audio_time_base;
			break;

		default:
			if (!time_base_set) codec_ctx->time_base = ffhelpers::ff_global_time_base;
		}

	}

	configure_multithreading(16);

	int ret = 0;
	/* Init the encoder */
	if ((ret = avcodec_open2(codec_ctx, codec, NULL)) < 0)
	{
		ON_FF_ERROR("Failed to open the encoder.")
	}

	//avcodec_parameters_free(&par);
}
