/*
* media.h:
* Defines media container and stream classes.
*/

#pragma once

#include "ff_time.h"

#include <string>
#include <vector>

struct AVFormatContext;
struct AVStream;
struct AVRational;

namespace ff
{
	// Represents a stream
	// Although all stream structures in the program have the same field: p_stream, they have different methods.
	// For example, we genearlly get something from an input stream and set the fields of an output stream.
	// Therefore, there are multiple structures inherited from this struct.
	struct stream
	{
	public:
		stream(struct ::AVStream* ps = nullptr) : p_stream(ps) {}
		stream(const stream& other) : p_stream(other.p_stream) {}
		virtual ~stream() = default;

		stream& operator=(const stream& right);

		struct ::AVStream* operator->() const { return p_stream; }

		// It's just a reference to it. The class does not own the stream in any way.
		struct ::AVStream* p_stream = nullptr;
	};

	struct input_stream : public stream
	{
	public:
		input_stream() = default;
		input_stream(::AVStream* ps) : stream(ps) {}
		input_stream(const input_stream& other) : stream(other) {}

		input_stream& operator=(const input_stream& right);

		ff::time get_time_base() const;

		bool is_video() const;
		bool is_audio() const;
		bool is_subtitle() const;

		// @returns its duration in its time base units
		int64_t get_duration() const;
		// @returns its duration in seconds
		double calculate_duration_in_sec() const;
		
	};

	struct output_stream : public stream
	{
	public:
		output_stream() = default;
		/*
		* Creates a new output stream for the output media f, 
		and copies the parameters from the input_stream to it.
		*/
		output_stream(input_stream i, const class output_media& f);
		/*
		* Creates a new output stream for the output media f, 
		* and copies the parameters from the encoder that will be encoding packets for it to it.
		* If the user wants to use some different settings for the stream, feel free to modify the fields afterwards.
		*/
		output_stream(const class encoder& enc, const class output_media& f);

		// If it's a video stream, then the pixel format is set to fmt.
		// If it's a audio stream, then the format is set to fmt.
		void set_format(int fmt);

		// Sets the time base
		void set_time_base(int numerator, int denominator);
	};

	// A container of multimedia streams
	class media
	{
	public:
		media() = default;
		virtual ~media() = default;

		virtual bool has_videos() const = 0;
		virtual bool has_audios() const = 0;
		virtual bool has_subtitles() const = 0;

		virtual int num_videos() const = 0;
		virtual int num_audios() const = 0;
		virtual int num_subtitles() const = 0;

	public:
		struct ::AVFormatContext* get_format_ctx() const { return p_format_ctx; }

	protected:
		struct ::AVFormatContext* p_format_ctx = nullptr;
	};

	// A container of multimedia streams for demuxing and decoding
	class input_media : public media
	{
	public:
#pragma region ctors and dtors
		input_media() = default;

		/*
		* Loads the media from a file.
		* @param fp: filepath
		* 
		* @throws std::runtime_error if the media could be opened from that file
		*/
		input_media(const std::string& fp): filepath(fp) { load(fp); }

		~input_media() { unload(); }
#pragma endregion

		/*
		* If it's not loaded, then loads it from a file.
		* @param fp: filepath
		* 
		* @throws std::runtime_error if the media could be opened from that file
		*/
		void load(const std::string& fp);

		// closes the media container and clears everything.
		void unload();

		// clears all information about the streams.
		void clear_streams();

#pragma region streams
		bool loaded() const { return nullptr != p_format_ctx; }

		bool has_videos() const override { return !vinds.empty(); }
		bool has_audios() const override { return !ainds.empty(); }
		bool has_subtitles() const override { return !sinds.empty(); }
		int num_videos() const override { return (int)vinds.size(); }
		int num_audios() const override { return (int)ainds.size(); }
		int num_subtitles() const override { return (int)sinds.size(); }

		int get_video_i(int i) const { return vinds[i]; }
		int get_audio_i(int i) const { return ainds[i]; }
		int get_subtitle_i(int i) const { return sinds[i]; }

		int num_streams() const { return (int)streams.size(); }
		input_stream get_stream(int i) const { return streams[i]; }
		const auto& get_streams() const { return streams; }
#pragma endregion

		/*
		* @returns the extension name of the file, with the . prefixed.
		* An empty string if the input is not linked to any file.
		*/
		std::string get_extension_name() const;

	private:
		std::string filepath;

#pragma region streams
		// each index in the vector is the index of the stream in the container.
		std::vector<input_stream> streams;

		// for categorization, store the indices here separately for v/a/s
		// The first index in each vector is the "best" stream among those streams.
		std::vector<int> vinds, ainds, sinds;
#pragma endregion

		// duration of the 
		double duration;
	};

	class output_media : public media
	{
	public:
#pragma region ctors and dtors
		output_media() = default;

		/*
		* Links the output media with the file, and chooses a muxer based on the filename.
		* In addition, it guesses the codec IDs.
		* It does not create any streams or do any further work.
		* 
		* @param fp: filepath
		*
		* @throws std::runtime_error on failure
		*/
		output_media(const std::string& fp);

		~output_media() { unload(); }
#pragma endregion

		// closes the media container and destroys all streams.
		void unload();

	public:
		// @returns the codec id corresponding to the media type, which can be used to initialize an encoder.
		int get_codec_id(int i) const { return codec_ids[i]; }

		/*
		* Adds an output stream to the output_media, whose information is obtained from an encoder.
		* 
		* The class gauranteens that the streams are stored in the order they are added.
		 That is, calling get_stream with i will get the ith added stream.
		 The user should maintain a stream map himself with this functionality.
		*
		* @returns the stream added.
		*/
		output_stream add_stream(const class encoder& enc);
		/*
		* Adds an output stream to the output_media, whose information is obtained from an input stream.
		*
		* The class gauranteens that the streams are stored in the order they are added.
		 That is, calling get_stream with i will get the ith added stream.
		 This is actually a stream map. The user may also choose to implement his own.
		*
		* @returns the stream added.
		*/
		output_stream add_stream(const struct input_stream& is);

		// @returns the ith stream added.
		output_stream get_stream(int i) const { return streams[i]; }

		bool has_videos() const override { return !vinds.empty(); }
		bool has_audios() const override { return !ainds.empty(); }
		bool has_subtitles() const override { return !sinds.empty(); }
		int num_videos() const override { return (int)vinds.size(); }
		int num_audios() const override { return (int)ainds.size(); }
		int num_subtitles() const override { return (int)sinds.size(); }

	private:
		// v,a,data,s,attachment,nb
		int codec_ids[6] = { -1,-1,-1,-1,-1,-1 };

		std::vector<output_stream> streams;

		// for categorization, store the indices here separately for v/a/s
		std::vector<int> vinds, ainds, sinds;

	private:
		// Compares two extension names case-insensitively.
		// @returns true iff they equal
		static bool extension_compare(const std::string& ext1, const std::string& ext2);
	};
}