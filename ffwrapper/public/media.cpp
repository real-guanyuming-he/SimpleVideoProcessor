extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "../private/ff_helpers.h"
#include "../private/ff_math_helpers.h"

#include "media.h"
#include "frame.h"
#include "encoder.h"
#include "decoder.h"

#include <filesystem>

void ff::input_media::load(const std::string& fp)
{
	if (loaded())
	{
		return;
	}
	else
	{
		clear_streams();
	}

	int err_code = 0;

	// get context format from the file
	if ((err_code = avformat_open_input(&p_format_ctx, 
		std::filesystem::path(fp).string().c_str(), nullptr, nullptr) != 0))
	{
		ON_FF_ERROR("The format context could not be obtained from the file.")
	}
	
	// read stream information
	if (avformat_find_stream_info(p_format_ctx, nullptr) < 0)
	{
		ON_FF_ERROR("Could not find stream information.")
	}

	// Find stream indices and categorize them
	// First, find best streams and put them first for each vector.
	int bvi = -1, bai = -1, bsi = -1;
	{
		int stream_ind = -1;
		// find best video stream
		if ((stream_ind = av_find_best_stream(p_format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0)) < 0)
		{
			// It doesn't matter if the container doesn't have a specific kind of stream. So we do nothing if we could not find a best stream.
		}
		else
		{
			bvi = stream_ind;
			vinds.push_back(stream_ind);
		}
		// find best audio stream
		if ((stream_ind = av_find_best_stream(p_format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0)) < 0)
		{
			// It doesn't matter if the container doesn't have a specific kind of stream. So we do nothing if we could not find a best stream.
		}
		else
		{
			bai = stream_ind;
			ainds.push_back(stream_ind);
		}

		// find best audio stream
		if ((stream_ind = av_find_best_stream(p_format_ctx, AVMEDIA_TYPE_SUBTITLE, -1, -1, nullptr, 0)) < 0)
		{
			// It doesn't matter if the container doesn't have a specific kind of stream. So we do nothing if we could not find a best stream.
		}
		else
		{
			bsi = stream_ind;
			sinds.push_back(stream_ind);
		}
	}

	// then fill in other indices
	for (unsigned int i = 0; i < p_format_ctx->nb_streams; ++i)
	{
		streams.push_back(p_format_ctx->streams[i]);

		if (p_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			// add the index if it's not the best one
			if(i != bvi) vinds.push_back(i);
		}
		else if (p_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			// add the index if it's not the best one
			if (i != bai) ainds.push_back(i);
		}
		else if (p_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
		{
			// add the index if it's not the best one
			if (i != bsi) sinds.push_back(i);
		}
	}

	duration = ffhelpers::ff_time_in_base_to_seconds(p_format_ctx->duration, ffhelpers::ff_global_time_base);
}

void ff::input_media::unload()
{
	clear_streams();

	ffhelpers::safely_close_input_format_context(&p_format_ctx);
}

void ff::input_media::clear_streams()
{
	streams.clear();
	vinds.clear(); ainds.clear(); sinds.clear();
}

ff::input_stream& ff::input_stream::operator=(const input_stream& right)
{
	stream::operator=(right);

	// TODO: insert return statement here
	return *this;
}

int64_t ff::input_stream::get_duration() const
{
	return p_stream->duration;
}

double ff::input_stream::calculate_duration_in_sec() const
{
	return ffhelpers::ff_time_in_base_to_seconds(p_stream->duration, p_stream->time_base);
}

ff::stream& ff::stream::operator=(const stream& right)
{
	p_stream = right.p_stream;
	// TODO: insert return statement here
	return *this;
}

void ff::output_stream::set_format(int fmt)
{
	p_stream->codecpar->format = fmt;
}

ff::time ff::input_stream::get_time_base() const
{
	return p_stream->time_base;
}

void ff::output_stream::set_time_base(int numerator, int denominator)
{
	p_stream->time_base = AVRational{ numerator,denominator };
}

bool ff::input_stream::is_video() const
{
	return p_stream ? p_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO : false;
}

bool ff::input_stream::is_audio() const
{
	return p_stream ? p_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO : false;
}

bool ff::input_stream::is_subtitle() const
{
	return p_stream ? p_stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE : false;
}

ff::output_media::output_media(const std::string& fp)
{
	// See https://ffmpeg.org/doxygen/5.1/group__lavf__encoding.html#details

	p_format_ctx = avformat_alloc_context();
	if (!p_format_ctx)
	{
		ON_FF_ERROR("Could not allocate format context.")
	}

	p_format_ctx->oformat = av_guess_format(nullptr, fp.c_str(), nullptr);
	if (!(p_format_ctx->oformat))
	{
		ON_FF_ERROR("The output extension is not supported.")
	}

	// Don't need to check failures as not all formats have all these
	for (int i = 0; i < 6; ++i)
	{
		codec_ids[i] = av_guess_codec(p_format_ctx->oformat, nullptr, fp.c_str(), nullptr, AVMediaType(i));
	}

	int ret = avio_open(&p_format_ctx->pb, fp.c_str(), AVIO_FLAG_READ_WRITE);
	if (ret < 0)
	{
		ON_FF_ERROR("Could not open or create the output file.")
	}

	auto size = (fp.size() + 1) * sizeof(char);
	p_format_ctx->url = (char*)av_malloc(size);
	strcpy_s(p_format_ctx->url, size, &fp[0]);
}

void ff::output_media::unload()
{
	// This already frees all the streams
	ffhelpers::safely_free_format_context(&p_format_ctx);
}

ff::output_stream ff::output_media::add_stream(const encoder& enc)
{
	streams.emplace_back(enc, *this);

	int ind = (int)streams.size() - 1;
	switch (enc.get_codec()->type)
	{
	case AVMEDIA_TYPE_VIDEO:
		vinds.push_back(ind);
		break;
	case AVMEDIA_TYPE_AUDIO:
		ainds.push_back(ind);
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		sinds.push_back(ind);
		break;
	}

	return streams[ind];
}

ff::output_stream ff::output_media::add_stream(const input_stream& is)
{
	streams.emplace_back(is, *this);

	int ind = (int)streams.size() - 1;
	switch (is->codecpar->codec_type)
	{
	case AVMEDIA_TYPE_VIDEO:
		vinds.push_back(ind);
		break;
	case AVMEDIA_TYPE_AUDIO:
		ainds.push_back(ind);
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		sinds.push_back(ind);
		break;
	}

	return streams[ind];
}

bool ff::output_media::extension_compare(const std::string& ext1, const std::string& ext2)
{
	if (ext1.size() != ext2.size())
	{
		return false;
	}

	for (size_t i = 0; i != ext1.size(); ++i)
	{
		if (ext1[i] != ext2[i])
		{
			return false;
		}
	}

	return true;
}

ff::output_stream::output_stream(input_stream i, const output_media& f):
	stream(avformat_new_stream(f.get_format_ctx(), nullptr))
{
	if (!p_stream)
	{
		ON_FF_ERROR("Could not create an output stream.")
	}

	/*
	* It is advised to manually initialize only the relevant fields in AVCodecParameters, 
	* rather than using avcodec_parameters_copy() during remuxing: 
	* there is no guarantee that the codec context values remain valid for both input and output format contexts.
	*/

	p_stream->time_base = i->time_base;

	int ret = 0;
	if ((ret = avcodec_parameters_copy(p_stream->codecpar, i->codecpar)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not copy codec parameters", ret);
	}
	// Don't know why but set this anyway.
	p_stream->codecpar->codec_tag = 0;	

	//auto par = p_stream->codecpar;
	//auto ipar = i->codecpar;

	//ffhelpers::codec_parameters_reset(par);

	//par->codec_id = ipar->codec_id;
	//par->codec_type = ipar->codec_type;
	//par->bit_rate = ipar->bit_rate;
	//par->bits_per_coded_sample = ipar->bits_per_coded_sample;
	//par->bits_per_raw_sample = ipar->bits_per_raw_sample;
	//par->profile = ipar->profile;
	//par->level = ipar->level;



	//int err = 0;
	//switch (par->codec_type)
	//{
	//case AVMEDIA_TYPE_VIDEO:
	//	par->format = ipar->format;
	//	par->width = ipar->width;
	//	par->height = ipar->height;

	//	par->sample_aspect_ratio = ipar->sample_aspect_ratio;
	//	par->field_order = ipar->field_order;
	//	par->color_range = ipar->color_range;
	//	par->color_primaries = ipar->color_primaries;
	//	par->color_trc = ipar->color_trc;
	//	par->color_space = ipar->color_space;																																																																															
	//	par->chroma_location = ipar->chroma_location;
	//	par->video_delay = ipar->video_delay;
	//	break;

	//case AVMEDIA_TYPE_AUDIO:
	//	par->format = ipar->format;
	//	if ((err = av_channel_layout_copy(&par->ch_layout, &ipar->ch_layout)) < 0)
	//	{
	//		ON_FF_ERROR_WITH_CODE("Could not copy channel layout", err);
	//	}
	//	par->sample_rate = ipar->sample_rate;
	//	par->block_align = ipar->block_align;
	//	par->frame_size = ipar->frame_size;
	//	par->initial_padding = ipar->initial_padding;
	//	par->trailing_padding = ipar->trailing_padding;
	//	par->seek_preroll = ipar->seek_preroll;
	//	break;

	//case AVMEDIA_TYPE_SUBTITLE:
	//	par->width = ipar->width;
	//	par->height = ipar->height;
	//	break;

	//default:
	//	par->codec_id = ipar->codec_id;
	//}
}

ff::output_stream::output_stream(const class encoder& enc, const class output_media& f) :
	stream(avformat_new_stream(f.get_format_ctx(), nullptr))
{
	if (!p_stream)
	{
		ON_FF_ERROR("Could not create an output stream.")
	}

	p_stream->codecpar->codec_type = enc.get_codec()->type;
	p_stream->codecpar->codec_id = enc.get_codec()->id;

	auto enc_ctx = enc.get_codec_ctx();

	int ret = 0;
	if ((ret = avcodec_parameters_from_context(p_stream->codecpar, enc_ctx) < 0))
	{
		ON_FF_ERROR_WITH_CODE("Could not copy info from the encoder to the stream.", ret);
	}
}
