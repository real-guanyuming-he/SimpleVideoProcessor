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

	std::vector<std::list<std::pair<float, float>>> tasks_scheduled;
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

void clip_page_schedule_tasks(const float* tasks, int number)
{
	// Invariant:
	// Tasks in every machine are also sorted in the ascending order of starting time

	// Algorithm used:
	// Schedule tasks of start and finishing time on a number of machines
	// such that the number of machines is minimized.
	// Greedy approach:
	// See if the remaining task of the smallest start time can be scheduled on existing machines.
	// If not, then start a new machine and schedule it there.

	// @param ts: start time of the target task
	// @param tf: finishing time of the target task
	// @returns true iff the machine can accept the task decided by ts and tf
	// also inserts the task into the machine and keeps the invariant if it can be accepted.
	auto can_that_machine_accept_this_task =
		[](float ts, float tf, decltype(clip_page::tasks_scheduled)::value_type& machine) -> bool
	{
		// Keep a record of the finishing time of the task examined in the last iteration.
		// At the beginning, the last task examined is none. Set it to the time boundary 0.f.
		float last_f = 0.f;
		// since tasks are sorted in ascending order of start time, 
		// just one iteration is needed
		for (auto iter = machine.begin(); iter != machine.end(); ++iter)
		{
			// start and finishing time of the task examined.
			float s = iter->first, f = iter->second;

			// find the first start time that's >= task_finishing.
			if (s < tf)
			{
				// Can use a continue here, but I need to set last_f, 
				// so let the flow naturally go down.
			}
			else // s is the first start time that's >= task_finishing.
			{
				// check if the finishing time of last task is <= task_start
				// if so the space in between is sufficient for the target
				if (last_f <= ts)
				{
					// the invariant will be maintained by the insertion.
					machine.insert
					(
						++iter /* need to insert after iter, but list::insert inserts before it */,
						std::make_pair(ts, tf)
					);
					return true;
				}
				else
				{
					// the target can only be inserted in the space before the first task that's behind the target
					// if there IS such a space
					return false;
				}
			}

			last_f = f;
		}

		// if the iteration does not find a task that's behind target, 
		// that is, if all existing tasks on the machine are executed before target,
		// then there is still a possibility that the space after all present tasks can fit the target.
		if (last_f <= ts)
		{
			// no need to check finishing time as it's one of the assumptions that f <= duration.
			machine.insert(machine.cend(), std::make_pair(ts, tf));
			return true;
		}

		return false;
	};

	// i: index to indicate which tasks (that are before it) are already scheduled
	for (int i = 0; i != number; ++i)
	{
		const float* p_task = tasks + 2 * i;
		float task_start = *(p_task), task_finishing = *(p_task + 1);

		bool is_task_accepted_by_existing_machines = false;
		for (auto& machine : clip_page::tasks_scheduled)
		{
			if // if the target can be accepted by one of the existing machines, then schedule it there
			(
				is_task_accepted_by_existing_machines =
				can_that_machine_accept_this_task(task_start, task_finishing, machine)
			)
			{
				break;
			}
		}

		if (!is_task_accepted_by_existing_machines)
		{
			// if the target cannot be accepted by one of the existing machines, 
			// then add a new machine and schedule it there
			clip_page::tasks_scheduled.push_back({ std::make_pair(task_start, task_finishing) });
		}
	}
}
