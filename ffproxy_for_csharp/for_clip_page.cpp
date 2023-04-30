#include "pch.h"
#include "for_clip_page.h"

#include <stdexcept>

namespace clip_page
{
	bool is_input_opened = false;
	bool is_output_opened = false;

	std::unique_ptr<ff::input_media> input_video;
	std::unique_ptr<ff::output_media> output_video;
	std::unique_ptr<ff::demuxer> demuxer;
	std::unique_ptr<ff::muxer> muxer;

	std::vector<std::pair<float, float>> tasks_to_be_scheduled;
	std::vector<std::vector<std::pair<float, float>>> tasks_scheduled;
}

bool clip_page_is_video_ready()
{
	return clip_page::is_input_opened && clip_page::is_output_opened;
}

void clip_page_reset()
{
	clip_page_reset_input_only();
	clip_page_reset_output_only();
}

void clip_page_reset_input_only()
{
	clip_page::input_video.reset();
	clip_page::demuxer.reset();

	clip_page::is_input_opened = false;
	
	// when the input changes, 
	// the tasks become meaningless because they may exceed the video duration.
	clip_page::tasks_to_be_scheduled.clear();
	clip_page::tasks_scheduled.clear();
}

void clip_page_reset_output_only()
{
	clip_page::output_video.reset();
	clip_page::muxer.reset();

	clip_page::is_output_opened = false;
}

bool clip_page_open_input_video(const char* file_path)
{
	try
	{
		clip_page::input_video.reset(new ff::input_media(file_path));
		clip_page::demuxer.reset(new ff::demuxer(*clip_page::input_video));
	}
	catch (const std::runtime_error& err) // failed
	{
		clip_page_reset();
		return false;
	}

	clip_page::is_input_opened = true;
	return true;
}

bool clip_page_open_output_video(const char* filepath_without_extension)
{
	if (!clip_page::is_input_opened)
	{
		clip_page_reset_output_only();
		return false;
	}

	try
	{
		clip_page::output_video.reset(new ff::output_media(std::string(filepath_without_extension) + clip_page::input_video->get_extension_name()));
		clip_page::muxer.reset(new ff::muxer(*clip_page::output_video));

		// For each input stream, create a corresponding output stream with exactly its information
		for (int i = 0; i != clip_page::input_video->num_streams(); ++i)
		{
			clip_page::output_video->add_stream(clip_page::input_video->get_stream(i));
		}
		// Don't do it here or the file won't be cleared if the opening fails.
		// muxer->write_file_header();
	}
	catch (const std::runtime_error& err) // failed
	{
		clip_page_reset_output_only();
		return false;
	}

	clip_page::is_output_opened = true;
	return true;
}

void clip_page_schedule_tasks()
{

}
