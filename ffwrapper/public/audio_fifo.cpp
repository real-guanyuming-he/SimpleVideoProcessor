extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
}

#include "../private/ff_helpers.h"
#include "audio_fifo.h"
#include "encoder.h"
#include "frame.h"

#include <stdexcept>

ff::audio_fifo::audio_fifo(int fmt, int num_channels, int num_start_samples) :
    fifo(av_audio_fifo_alloc((AVSampleFormat)fmt, num_channels, num_start_samples))
{
    if (!fifo)
    {
        ON_FF_ERROR("Could not allocate FIFO.")
    }
}

ff::audio_fifo::audio_fifo(const encoder& enc, int num_start_samples) :
    audio_fifo(enc.get_codec_ctx()->sample_fmt, enc.get_codec_ctx()->ch_layout.nb_channels, num_start_samples)
{

}

ff::audio_fifo::~audio_fifo()
{

}

int ff::audio_fifo::size() const
{
    return av_audio_fifo_size(fifo);
}

int ff::audio_fifo::write(void** data_planes, int num_samples_to_write)
{
    int ret = av_audio_fifo_write(fifo, data_planes, num_samples_to_write);
    
    if (ret < 0)
    {
        ON_FF_ERROR_WITH_CODE("Could not write data to the audio FIFO buffer.", ret)
    }

    return ret;
}

int ff::audio_fifo::write(const frame& f)
{
    return write((void**)f->data, f->nb_samples);
}

int ff::audio_fifo::read(void** data_planes, int num_samples_to_read)
{
    int ret = av_audio_fifo_read(fifo, data_planes, num_samples_to_read);

    if (ret < 0)
    {
        ON_FF_ERROR_WITH_CODE("Could not write data to the audio FIFO buffer.", ret)
    }

    return ret;
}

int ff::audio_fifo::read(const frame& f)
{
    return read((void**)f->data, f->nb_samples);
}

void ff::audio_fifo::clear(int num_samples_to_clear)
{
    if (num_samples_to_clear == -1)
    {
        av_audio_fifo_reset(fifo);
    }
    else
    {
        int ret;
        if ((ret = av_audio_fifo_drain(fifo, num_samples_to_clear)) < 0)
        {
            ON_FF_ERROR_WITH_CODE("Could not remove data from the fifo buffer.", ret)
        }
    }
}

