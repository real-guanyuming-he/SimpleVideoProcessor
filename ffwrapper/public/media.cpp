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

ff::input_stream::input_stream(::AVStream* ps) : stream(ps)
{
	calculate_stream_duration();
}

ff::input_stream& ff::input_stream::operator=(const input_stream& right)
{
	stream::operator=(right);

	duration = right.duration;

	// TODO: insert return statement here
	return *this;
}

void ff::input_stream::calculate_stream_duration()
{
	duration = ffhelpers::ff_time_in_base_to_seconds(p_stream->duration, p_stream->time_base);
}

ff::stream& ff::stream::operator=(const stream& right)
{
	p_stream = right.p_stream;
	// TODO: insert return statement here
	return *this;
}

void ff::stream::set_format(int fmt)
{
	p_stream->codecpar->format = fmt;
}

void ff::stream::set_time_base(int numerator, int denominator)
{
	p_stream->time_base = AVRational{ numerator,denominator };
}

bool ff::stream::is_video() const
{
	return p_stream ? p_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO : false;
}

bool ff::stream::is_audio() const
{
	return p_stream ? p_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO : false;
}

bool ff::stream::is_subtitle() const
{
	return p_stream ? p_stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE : false;
}

ff::output_media::output_media(const std::string& fp)
{
	// See https://ffmpeg.org/doxygen/5.1/group__lavf__encoding.html#details

	// get the extension name by finding the last occurence of '.'
	size_t dot_pos = fp.find_last_of('.');
	std::string extension_name;
	if (dot_pos == std::string::npos)
	{
		ON_FF_ERROR("The filename does not have an extension")
	}
	extension_name = fp.substr(dot_pos + 1);

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

	int ind = streams.size() - 1;
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

void ff::output_media::write_file_header()
{
	if (avformat_write_header(p_format_ctx, nullptr) < 0)
	{
		ON_FF_ERROR("Could not write the file header.")
	}
}

void ff::output_media::feed_packet(const packet& pkt)
{
	int ret = 0;
	if ((ret = av_interleaved_write_frame(p_format_ctx, pkt)) < 0)
	{
		ON_FF_ERROR_WITH_CODE("Could not feed a packet to the output file: ", ret)
	}
}

void ff::output_media::finalize()
{
	if (av_write_trailer(p_format_ctx) != 0)
	{
		ON_FF_ERROR("Could not finalize the output file.")
	}
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

	/*p_stream->time_base = enc_ctx->time_base;
	switch (p_stream->codecpar->codec_type)
	{
	case AVMEDIA_TYPE_VIDEO:
		p_stream->codecpar->width = enc_ctx->width;
		p_stream->codecpar->height = enc_ctx->height;
		p_stream->codecpar->format = enc_ctx->pix_fmt;
		break;
	case AVMEDIA_TYPE_AUDIO:
		av_channel_layout_copy(&p_stream->codecpar->ch_layout, &enc_ctx->ch_layout);
		p_stream->codecpar->sample_rate = enc_ctx->sample_rate;
		p_stream->codecpar->format = enc_ctx->sample_fmt;
	}*/
}
