//extern "C"
//{
//#include <libavcodec/avcodec.h>
//}
//
//int main()
//{
//	auto codec = avcodec_find_encoder(AV_CODEC_ID_H264);
//	auto codec_ctx = avcodec_alloc_context3(codec);
//
//	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
//	codec_ctx->width = 2560;
//	codec_ctx->height = 1440;
//	codec_ctx->time_base.num = 1; codec_ctx->time_base.den = 172; // cannot be smaller than 1/172
//
//	avcodec_open2(codec_ctx, codec, NULL);
//
//	return 0;
//}