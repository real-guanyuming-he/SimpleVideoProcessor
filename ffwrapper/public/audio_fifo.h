#pragma once

struct AVAudioFifo;

namespace ff
{	
	/*
	* An audio fifo buffer.
	* Since the decoder's and the encoder's frame size may differ, we
	* need a FIFO buffer to store as many frames worth of input samples
	* that they make up at least one frame worth of output samples.
	*/
	class audio_fifo
	{
	public:
		audio_fifo() = delete;
		audio_fifo(int fmt, int num_channels, int num_start_samples = 1);
		explicit audio_fifo(const class encoder& enc, int num_start_samples = 1);
		~audio_fifo();

	public:
		int size() const;

		/*
		* Writes num_samples_to_write samples from the data_planes to the fifo buffer.
		* @returns the number of samples actually written
		*/
		int write(void** data_planes, int num_samples_to_write);

		/*
		* Writes the audio data stored in frame f into the fifo buffer.
		* @returns number of samples actually written. Should be compared with f->nb_samples to see if all of the data is successfully written.
		*/
		int write(const struct frame& f);

		/*
		* Reads num_samples_to_write samples from the fifo buffer to the data_planes.
		* Cannot read more samples than its size()
		* @returns the number of samples actually read
		*/
		int read(void** data_planes, int num_samples_to_read);

		/*
		* Reads the audio data stored in the fifo buffer into frame f.
		* @returns number of samples actually read. Should be compared with f->nb_samples to see if the frame is completely filled.
		*/
		int read(const struct frame& f);

		/*
		* Removes data from the fifo buffer.
		* @param num_samples_to_clear: number of samples to remove. If it's -1, then all samples will be removed.
		*/
		void clear(int num_samples_to_clear = -1);

	private:
		::AVAudioFifo* fifo;
	};
}