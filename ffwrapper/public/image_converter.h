/*
* image_converter.h:
* Uses libswscale to change the color format as well as the resolution of images.
*/

#pragma once

struct SwsContext;

namespace ff
{
	class image_converter
	{
	public:
		image_converter() : sws_ctx(nullptr) {};
		image_converter
		(
			int src_w, int src_h, int src_pix_fmt,
			int dst_w, int dst_h, int dst_pix_fmt,
			int algorithm
		);
		image_converter
		(
			const class codec_base& src_codec, 
			const class codec_base& dst_codec,
			int algorithm
		);

		~image_converter();

	public:
		// If the converter is ready to convert.
		bool is_ready() const { return sws_ctx != nullptr; }
		/*
		* Converts src to dst in the way instructed in the constructor.
		* @throws std::runtime_error is an unexpected error occurs.
		*/
		void convert(struct frame& src, struct frame& dst);

	private:
		::SwsContext* sws_ctx = nullptr;
	};
}