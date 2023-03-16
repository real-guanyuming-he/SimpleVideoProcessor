/*
* ff_helpers.h:
* Defines PRIVATE helpers for ffmpeg C API programming.
* 
* Note: Math helpers are defined in another header because constexpr and inline are used heavily there
*/

#pragma once

struct AVFormatContext;
struct AVIOContext;
struct AVFrame;
struct AVPacket;
struct AVCodecContext;
struct SwsContext;
struct SwrContext;
struct AVAudioFifo;

#include <string>

namespace ffhelpers
{
	// Any of these functions frees the ptr if it's not nullptr and then sets it to nullptr
#pragma region safely_free

	void safely_close_input_format_context(::AVFormatContext** ppfc);

	// Unlike safely_close_input_format_context, it calls avformat_free_context()
	void safely_free_format_context(::AVFormatContext** ppfc);

	void safely_free_avio_context(::AVIOContext** ppioct);

	void safely_free_frame(::AVFrame** ppf);

	void safely_free_packet(::AVPacket** pppkt);

	void safely_free_codec_context(::AVCodecContext** ppcodctx);

	void safely_free_sws_context(::SwsContext** sws_ctx);

	void safely_free_swr_context(::SwrContext** swr_ctx);

	void safely_free_audio_fifo(::AVAudioFifo** fifo);

	// Translates the ffmpeg c api error code into string.
	std::string ff_translate_error_code(int err_code);

#define ON_FF_ERROR(msg) throw std::runtime_error(msg);
#define ON_FF_ERROR_WITH_CODE(msg, code) ON_FF_ERROR(std::string(msg) + " " + ffhelpers::ff_translate_error_code(code))

#pragma endregion
}

