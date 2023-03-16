#pragma once

struct AVFrame;
struct AVPacket;
struct AVChannelLayout;

namespace ff
{
	// A buffer holding a decoded frame.
	struct frame
	{
	public:
		// Allocates a clean frame
		frame() : buffer(nullptr) { allocate(); }
		// Takes the ownership of f
		frame(struct ::AVFrame* f) : buffer(f) {}
		// throws std::runtime_error
		frame(const frame& other);
		frame(frame&& other) noexcept : buffer(other.buffer) { other.buffer = nullptr; }

		~frame();

		operator AVFrame* () const { return buffer; }
		AVFrame* operator->() const { return buffer; }

	public:
		bool is_allocated() const { return nullptr != buffer; }
		// @returns if it has a buffer so that it can contain a frame
		bool has_a_buffer() const;

		// Allocates a clean AVFrame, but not any buffer.
		// @throws std::runtime_error on failure
		void allocate();
		// Destroy the frame and delete everything it stores
		void destroy();

		/*
		* Creates a buffer large enough for containing a video frame.
		* 
		* @param width: width of the frame
		* @param height: height of the frame
		* @param pix_fmt: pixel format of the frame
		* @param alignment: the alignment for the buffer. For speed set it to 32 or 64 so that faster methods like SIMD can be applied.
		* By default, 0 is used, which means it will be automatically choosen for the current cpu.
		* 
		* @throws std::runtime_error on failure
		*/
		void create_video_buffer(int width, int height, int pix_fmt, int alignment = 0);

		void create_audio_buffer(int nb_samples, int sample_format, const ::AVChannelLayout* ch_layout, int alignment = 0);

	public:
		struct ::AVFrame* buffer = nullptr;

		/*
		* Relinquishes the ownership to the buffer.
		*/
		void relinquish_ownership() { buffer = nullptr; }

	private:
		// The same as av_frame_copy_props + av_frame_copy
		// @throws std::runtime_error on failure
		void av_frame_copy_all(struct ::AVFrame* dst, struct ::AVFrame* src);
	};

	// An encoded packet.
	struct packet
	{
	public:
		// Allocates a clean frame
		packet() : pkt(nullptr) { allocate(); }
		// Takes the ownership of f
		packet(struct ::AVPacket* p) : pkt(p) {}
		// I can't find a ffmpeg function to copy the data of a AVPacket.
		packet(const packet& other) = delete;
		packet(packet&& other) noexcept : pkt(other.pkt) { other.pkt = nullptr; }

		~packet() { destroy(); }

		operator AVPacket* () const { return pkt; }
		AVPacket* operator->() const { return pkt; }

	public:
		bool is_allocated() const { return nullptr != pkt; }

		// Allocates a clean AVPacket, but not any buffer.
		// @throws std::runtime_error on failure
		void allocate();

		// Clears the data it holds but does not destroy it.
		void unref();

		// Destroy the packet and delete everything it stores
		void destroy();

	public:
		struct ::AVPacket* pkt = nullptr;

		/*
		* Relinquishes the ownership to packet
		*/
		void relinquish_ownership() { pkt = nullptr; }

	private:
	};
}