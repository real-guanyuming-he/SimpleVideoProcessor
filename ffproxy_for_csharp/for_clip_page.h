/*
* The clip page of the GUI talks with the functions here
*/

#pragma once

#include "../ffwrapper/public/media.h"
#include "../ffwrapper/public/muxer.h"
#include "../ffwrapper/public/demuxer.h"
// TODO: after adding direct encoder to the wrapper, enable per-frame clip
//#include "../ffwrapper/public/decoder.h"
//#include "../ffwrapper/public/encoder.h"

#include <memory> // For unique ptr
#include <vector>

extern bool clip_page_is_input_opened;
extern bool clip_page_is_output_opened;

// All variables are declared in pointers so that it's much eaiser to reset them (just delete them).
extern std::unique_ptr<ff::input_media> clip_page_input_video;
extern std::unique_ptr<ff::output_media> clip_page_output_video;
extern std::unique_ptr<ff::demuxer> clip_page_demuxer;
extern std::unique_ptr<ff::muxer> clip_page_muxer;

/*
* If it returns true, then a call to clip_page_open_video() has been successful,
* and the video can be processed by the other functions.
* 
* @returns true iff both the input and the output are opened.
*/
extern bool clip_page_is_video_ready();

/*
* Resets all variables to their initial states (i.e. nullptrs).
*/
extern void clip_page_reset();

/*
* Only resets the variables for input.
*/
extern void clip_page_reset_input_only();
/*
* Only resets the variables for output.
*/
extern void clip_page_reset_output_only();

/*
* Opens a video decided by file_path as the clipping input and initializes all corresponding variables on success.
* On failure, ALL variables (include those for output) are reset to ensure that they are in their initial states.
* 
* @param file_path: path to the file
* @returns true iff the video is opened successfully.
*/
extern bool clip_page_open_input_video(const char* file_path);

/*
* Requires that the input is opened.
* 
* Opens a video decided by file_path as the clipping input and initializes all corresponding variables on success.
* On failure, all output variables are reset to ensure they are in their initial states.
*
* @param file_path: path without the extension name to the file
* @returns true iff the output is opened successfully.
*/
extern bool clip_page_open_output_video(const char* filepath_without_extension);