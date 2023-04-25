#include "pch.h"
#include "for_clip_page.h"

#include <stdexcept>

bool clip_page_is_input_opened = false;
bool clip_page_is_output_opened = false;

std::unique_ptr<ff::input_media> clip_page_input_video;
std::unique_ptr<ff::output_media> clip_page_output_video;
std::unique_ptr<ff::demuxer> clip_page_demuxer;
std::unique_ptr<ff::muxer> clip_page_muxer;

bool clip_page_is_video_ready()
{
	return clip_page_is_input_opened && clip_page_is_output_opened;
}

void clip_page_reset()
{
	clip_page_reset_input_only();
	clip_page_reset_output_only();
}

void clip_page_reset_input_only()
{
	clip_page_input_video.reset();
	clip_page_demuxer.reset();

	clip_page_is_input_opened = false;
}

void clip_page_reset_output_only()
{
	clip_page_output_video.reset();
	clip_page_muxer.reset();

	clip_page_is_output_opened = false;
}

bool clip_page_open_input_video(const char* file_path)
{
	try
	{
		clip_page_input_video.reset(new ff::input_media(file_path));
		clip_page_demuxer.reset(new ff::demuxer(*clip_page_input_video));
	}
	catch (const std::runtime_error& err) // failed
	{
		clip_page_reset();
		return false;
	}

	clip_page_is_input_opened = true;
	return true;
}

bool clip_page_open_output_video(const char* filepath_without_extension)
{
	if (!clip_page_is_input_opened)
	{
		clip_page_reset_output_only();
		return false;
	}

	try
	{
		clip_page_output_video.reset(new ff::output_media(std::string(filepath_without_extension) + clip_page_input_video->get_extension_name()));
		clip_page_muxer.reset(new ff::muxer(*clip_page_output_video));

		// For each input stream, create a corresponding output stream with exactly its information
		for (int i = 0; i != clip_page_input_video->num_streams(); ++i)
		{
			clip_page_output_video->add_stream(clip_page_input_video->get_stream(i));
		}
		// Don't do it here or the file won't be cleared if the opening fails.
		// clip_page_muxer->write_file_header();
	}
	catch (const std::runtime_error& err) // failed
	{
		clip_page_reset_output_only();
		return false;
	}

	clip_page_is_output_opened = true;
	return true;
}

