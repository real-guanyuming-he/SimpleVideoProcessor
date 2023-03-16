extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h> // For AVFrame and AVCodec
#include <libavutil/imgutils.h>
}

#include "../private/ff_helpers.h"
#include "image_converter.h"
#include "codec.h"
#include "frame.h"
#include <stdexcept>

ff::image_converter::image_converter
(
	int src_w, int src_h, int src_pix_fmt,
	int dst_w, int dst_h, int dst_pix_fmt,
	int algorithm
) : sws_ctx(sws_getContext
(
	src_w, src_h, (AVPixelFormat)src_pix_fmt,
	dst_w, dst_h, (AVPixelFormat)dst_pix_fmt,
	algorithm,
	nullptr, nullptr, nullptr
))
{
	if (!sws_ctx)
	{
		ON_FF_ERROR("Could not get a sws context")
	}
}

ff::image_converter::image_converter
(
	const class codec_base& src_codec,
	const class codec_base& dst_codec,
	int algorithm
) :image_converter
(
	src_codec.get_codec_ctx()->width, src_codec.get_codec_ctx()->height, src_codec.get_codec_ctx()->pix_fmt,
	dst_codec.get_codec_ctx()->width, dst_codec.get_codec_ctx()->height, dst_codec.get_codec_ctx()->pix_fmt,
	algorithm
)
{}

ff::image_converter::~image_converter()
{
	ffhelpers::safely_free_sws_context(&sws_ctx);
}

void ff::image_converter::convert(frame & src, frame & dst)
{
	sws_scale(sws_ctx, src->data, src->linesize, 0, src->height, dst->data, dst->linesize);
}
