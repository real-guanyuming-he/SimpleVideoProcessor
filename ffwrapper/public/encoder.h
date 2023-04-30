/*
* Encoder.h:
* Defines encoders
*/

#pragma once

#include "codec.h"
#include "interfaces/src_sink.h"

struct AVChannelLayout;

namespace ff
{
	struct input_stream;

	/*
	* Base class for all encoders
	*/
	class encoder : public codec_base, public frame_sink, public packet_source
	{
	private:
		using super = codec_base;

	public:
		encoder() = default;
		// Finds the codec by name
		encoder(const char* name);
		// Finds the codec by ID (can be returned by a muxer)
		encoder(int ID);
		~encoder() = default;

	public:
		/*
		* Feeds a frame to the decoder for encoding.
		*
		* @param frame: the frame to feed. Its ownership will not be taken and its content will be copied.
		* Therefore it's necessary for the caller to destroy it after calling the method.
		* 
		* @returns true if the frame is successfully fed;
		* false if no more frames can be fed until some encoded packets are retrieved,
		or if the encoder is currently in draining mode.
		*
		* @throws std::runtime_error on failure.
		*/
		bool try_feed(ff::frame & frame) override;

		/*
		* Tries to encode the next packet
		*
		* @returns a valid packet if the encoding succeeds;
		* an invalide one if more frames are needed, or if EOF is reached.
		* The caller should check eof() to ascertain if EOF is reached.
		*/
		ff::packet try_get_one() override;

		/*
		* Flushes the encoder and resets its eof state.
		* Can be called after a draining is complete so that the encoder can be reused.
		*/
		void flush_codec() override;

		/*
		* After the frame source for the encoder has no more to feed the encoder and all available packets until then are extracted,
		* the caller should start draining the encoder to push all of the buffered data out so that the result is complete.
		* After draining starts, call try_get_one() repeatedly until eof() is true.
		*/
		void start_draining() override;
	
		// is eof reached in draining.
		bool eof() const { return eof_reached; }

	public:

		// These methods are virtual whenever the format is not strictly required so the implementations may decide to use different ones. 
#pragma region desired settings
		// @returns the "best" pixel format supported by the encoder.
		virtual int get_desired_pixel_format() const;

		// Selects the audio layout of the highest channel count supported by the codec,
		// and copies the layout configuration to dst
		virtual void get_best_audio_channel(::AVChannelLayout* dst) const;

		// @returns the best (highest) audio sample rate the codec supports; 44100 if none; 0 if unknown
		virtual int get_best_audio_sample_rate() const;

		// @returns the "best" sample format supported by the encoder.
		virtual int get_desired_audio_sample_format() const;
#pragma endregion

		// These methods return false if something is unsupported or if supported formats are unknown.
#pragma region is something supported?
		bool is_pixel_format_supported(int fmt) const;
		bool is_audio_sample_format_supported(int fmt) const;
		bool is_audio_sample_rate_supported(int rate) const;
		bool is_audio_channel_layout_supported(const ::AVChannelLayout* layout) const;

		bool is_video_setting_supported(const struct video_info&) const;
		bool is_audio_setting_supported(const struct audio_info&) const;
#pragma endregion

#pragma region required settings
		/*
		* An audio encoder will demand the number of audio samples per channel in one frame.
		* All submitted frames except the last one should contain EXACTLY the number of samples per channel.
		*/
		int get_required_number_of_samples_per_channel() const;
#pragma endregion

	protected:
		/*
		* Requires: the encoder has already been constrcuted by one of the base constructors.
		 Supposed to be called during derived class constructors to fill info into the codec ctx.

		* Fills the information of the encoder with the info from the decoder and the output media's format.
		 For each format specified in the decoder, the method checks if that's supported by the encoder.

		* Note: even if the decoder and the encoder use the same codec name or ID, 
		it may still be the case that some of the formats used by the decoder are not supported by the encoder.
		*/
		virtual void fill_encoder_info(const class decoder& dec, const class output_media& m);
			
	protected:
		bool eof_reached = false;

	public:


	};

	/*
	* Transcode means to change the format of something.
	* Therefore, the user must provide a decoder (for knowing the original format)
	to construct such an encoder.

	* Transcoding workflow:
	* 1. For each setting used by the decoder (e.g. pixel format, audio sample format, sample rate), 
	the class's constructor checks if it's supported by the desired encoder. If so, that setting is copied to the encoder.
	Otherwise, the best supported setting of the encoder is used.

	* 2. After 1, that is, after the construction of the transcode encoder, the user should check the current settings
	* (by calling get_current_...()) to see which ones are different from the ones of the decoder. 
	* Conversions may be necessary. For example, different pixel formats or audio sample formats require a conversion.
	* 
	* 3. After 2, the encoder may have some other required settings for frames to be encoded.
	The user should check them all (by calling get_required_...()) and ensure all frames submitted are in these settings.
	* 
	* Note that, if the sample rate (for audio encoding) or the frame rate (for video) changes, 
	then the time base will be set to a new one according to the new sample rate.
	* If the user decides to use the new time base in muxing, then time synchronization will be required, 
	that is, adjust the output packet's pts according to the input's time base and the output's.
	* 
	*/
	class transcode_encoder : public encoder
	{
	public:
		transcode_encoder() = delete;
		// A encoder of name that transcodes frames decoded by dec.
		transcode_encoder(const class decoder& dec, const class output_media& m, const char* name);
		// A encoder of ID that transcodes frames decoded by dec.
		transcode_encoder(const class decoder& dec, const class output_media& m, int ID);

	private:


	};

	/*
	* Used only for encoding that uses exactly the same codec as the input video's
	*/
	class direct_encoder : public encoder
	{
	public:
		direct_encoder() = default;
		// Creates the encoder that uses the same setting as the decoder and the input stream.
		explicit direct_encoder(const decoder& d, const output_media& m, const ff::input_stream& ins);

		~direct_encoder() { destroy(); }
	};

	/*
	* Unlike a transcode encoder that (tries to) uses the settings (like width/height) of a decoder,
	* a general encoder allows the user to specify all the encoding settings.
	* 
	* It is thus the user's responsibility to ensure that the frames he submitted are using the settings.
	*/
	class general_encoder : public encoder
	{

	};
}