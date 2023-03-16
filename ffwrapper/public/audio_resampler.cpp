extern "C"
{
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

#include "audio_resampler.h"
#include "frame.h"
#include "decoder.h"
#include "encoder.h"
#include "../private/ff_helpers.h"

#include <stdexcept>

ff::audio_resampler::audio_resampler() :
	swr_ctx(swr_alloc()), src_rate(0), dst_rate(0)
{
	if (!swr_ctx)
	{
		ON_FF_ERROR("Could not alloc swr ctx.")
	}
}

ff::audio_resampler::audio_resampler
(
	int src_fmt, int src_sample_rate, const ::AVChannelLayout* src_ch_layout, 
	int dst_fmt, int dst_sample_rate, const ::AVChannelLayout* dst_ch_layout
) : src_rate(src_sample_rate), dst_rate(dst_sample_rate)
{
	int ret = swr_alloc_set_opts2
	(
		&swr_ctx,
		const_cast<::AVChannelLayout*>(dst_ch_layout),
		(AVSampleFormat)dst_fmt,
		dst_sample_rate,
		const_cast<::AVChannelLayout*>(src_ch_layout),
		(AVSampleFormat)src_fmt,
		src_sample_rate,
		0, nullptr
	);
	if (ret < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not alloc the resampler or set its options.", ret)
	}

	if ((ret = swr_init(swr_ctx)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not initialize swr ctx.", ret)
	}
}

ff::audio_resampler::audio_resampler(const decoder& dec, const encoder& enc):audio_resampler
(
	int(dec.get_codec_ctx()->sample_fmt), dec.get_codec_ctx()->sample_rate, &(dec.get_codec_ctx()->ch_layout),
	int(enc.get_codec_ctx()->sample_fmt), enc.get_codec_ctx()->sample_rate, &(enc.get_codec_ctx()->ch_layout)
)
{
}

ff::audio_resampler::~audio_resampler()
{
	ffhelpers::safely_free_swr_context(&swr_ctx);
}

int ff::audio_resampler::convert(const frame& src_frame, frame& dst_frame)
{
	int ret;

	if ((ret = swr_convert_frame(swr_ctx, dst_frame, src_frame)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not convert audio samples.", ret)
	}

	return ret;
}

int ff::audio_resampler::calculate_dst_num_samples(int src_num_samples) const
{
	// (swr_get_delay(swr_ctx, src_rate) + src_num_samples) * dst_rate/src_rate
	return av_rescale_rnd(swr_get_delay(swr_ctx, src_rate) + src_num_samples, dst_rate, src_rate, AV_ROUND_UP);
}

