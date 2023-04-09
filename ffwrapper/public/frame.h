#pragma once

struct AVFrame;
struct AVPacket;
struct AVChannelLayout;

namespace ff
{
	// A buffer holding a decoded frame.
	// Has two levels of storage. The first level is the frame itself, which also has to be allocated (will be done by the constructors)
	// The second level is the data is holds, which is created by one of the create_xxx_buffer() methods.
	// On destruction, both levels of storage will be destroyed.
	struct frame
	{
	public:
		// Allocates a clean frame
		frame() : buffer(nullptr) { allocate(); }
		// Takes the ownership of f
		frame(struct ::AVFrame* f) : buffer(f) {}
		// throws std::runtime_error
		frame(const frame& other);
		frame(frame&& other) noexcept : buffer(other.buffer) { other.relinquish_ownership(); }

		inline frame& operator=(frame&& right) noexcept
		{
			buffer = right.buffer;
			right.relinquish_ownership();
			return *this;
		}

		~frame();

		operator AVFrame* () const { return buffer; }
		AVFrame* operator->() const { return buffer; }

	public:
		bool is_valid() const { return nullptr != buffer; }
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
		* Relinquishes the ownership to the frame.
		*/
		void relinquish_ownership() { buffer = nullptr; }

	public:
		// The same as av_frame_copy_props + av_frame_copy
		// @throws std::runtime_error on failure
		static void av_frame_copy_all(struct ::AVFrame* dst, struct ::AVFrame* src);
	};

	// An encoded packet.
	// Has two levels of storage. The first level is the frame itself, which also has to be allocated (will be done by the constructors)
	// The second level is the data is holds. The data is reference-counted. When it's destroyed, then the data will be unreferenced once.
	struct packet
	{
	public:
		// Allocates a clean frame
		packet() : buffer(nullptr) { allocate(); }
		// Takes the ownership of f
		packet(struct ::AVPacket* p) : buffer(p) {}
		packet(const packet& other);
		packet(packet&& other) noexcept : buffer(other.buffer) { other.relinquish_ownership(); }

		packet& operator=(const packet& right);
		inline packet& operator=(packet&& right) noexcept
		{
			buffer = right.buffer;
			right.relinquish_ownership();
			return *this;
		}

		~packet() { destroy(); }

		operator AVPacket* () const { return buffer; }
		AVPacket* operator->() const { return buffer; }

	public:
		bool is_valid() const { return nullptr != buffer; }

		// Allocates a clean AVPacket, but not any buffer.
		// @throws std::runtime_error on failure
		void allocate();

		// Unrefs its data it holds but does not destroy it.
		void unref();

		// Destroy the packet and unref its data.
		void destroy();

	public:
		struct ::AVPacket* buffer = nullptr;

		/*
		* Relinquishes the ownership to packet
		*/
		void relinquish_ownership() { buffer = nullptr; }

	public:
		/*
		* Assumes that its time fields like pts and dts are in the time base of s1,
		* then the method scales them to be in the time base of s2.
		*/
		void rescale_time(const struct stream& s1, const struct stream& s2);
	};
}