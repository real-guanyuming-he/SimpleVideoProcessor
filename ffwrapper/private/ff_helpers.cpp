extern "C"
{
#include <libavutil/audio_fifo.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include "ff_helpers.h"

namespace ffhelpers
{
	void safely_close_input_format_context(AVFormatContext** ppfc)
	{
		if (*ppfc)
		{
			avformat_close_input(ppfc);
		}
	}

	void safely_free_format_context(::AVFormatContext** ppfc)
	{
		if (*ppfc)
		{
			avformat_free_context(*ppfc);
			*ppfc = nullptr;
		}
	}

	void safely_free_avio_context(AVIOContext** ppioct)
	{
		if (*ppioct)
		{
			avio_closep(ppioct);
		}
	}

	void safely_free_frame(AVFrame** ppf)
	{
		if (*ppf)
		{
			// the function already does the setting nullptr for us
			av_frame_free(ppf);
		}
	}

	void safely_free_packet(AVPacket** pppkt)
	{
		if (*pppkt)
		{
			// the functions already does the setting nullptr for us
			av_packet_free(pppkt);
		}
	}

	void safely_free_codec_context(AVCodecContext** ppcodctx)
	{
		if (*ppcodctx)
		{
			// the functions already does the setting nullptr for us
			avcodec_free_context(ppcodctx);
		}
	}
	void safely_free_sws_context(::SwsContext** sws_ctx)
	{
		// Don't need to check for nullptr as the func does so
		sws_freeContext(*sws_ctx);
		sws_ctx = nullptr;
	}
	void safely_free_swr_context(::SwrContext** swr_ctx)
	{
		if (*swr_ctx)
		{
			// Don't need to set it to nullptr as this call does this for us.
			swr_free(swr_ctx);
		}
	}
	void safely_free_audio_fifo(::AVAudioFifo** fifo)
	{
		if (*fifo)
		{
			av_audio_fifo_free(*fifo);
			*fifo = nullptr;
		}
	}
	std::string ff_translate_error_code(int err_code)
	{
		std::string ret;
		ret.resize(AV_ERROR_MAX_STRING_SIZE);
		
		av_make_error_string(&ret[0], AV_ERROR_MAX_STRING_SIZE, err_code);

		return ret;
	}

	void codec_parameters_reset(::AVCodecParameters* par)
	{
		av_freep(&par->extradata);
		av_channel_layout_uninit(&par->ch_layout);

		memset(par, 0, sizeof(*par));

		par->codec_type = AVMEDIA_TYPE_UNKNOWN;
		par->codec_id = AV_CODEC_ID_NONE;
		par->format = -1;
		par->ch_layout.order = AV_CHANNEL_ORDER_UNSPEC;
		par->field_order = AV_FIELD_UNKNOWN;
		par->color_range = AVCOL_RANGE_UNSPECIFIED;
		par->color_primaries = AVCOL_PRI_UNSPECIFIED;
		par->color_trc = AVCOL_TRC_UNSPECIFIED;
		par->color_space = AVCOL_SPC_UNSPECIFIED;
		par->chroma_location = AVCHROMA_LOC_UNSPECIFIED;
		par->sample_aspect_ratio = AVRational{ 0, 1 };
		par->profile = FF_PROFILE_UNKNOWN;
		par->level = FF_LEVEL_UNKNOWN;
	}
}