#include "packet_retimer.h"

#include "frame.h"

extern "C"
{
#include <libavformat/avformat.h>
}

void ff::packet_retimer::retime(ff::packet& pkt)
{
	av_packet_rescale_ts(pkt, current, target);

	// convert the start time to be in the target time base
	int64_t start_time_in_base = ff::seconds_to_time_in_base(start_time, target);

	pkt->dts -= start_time_in_base;
	pkt->pts -= start_time_in_base;
}
