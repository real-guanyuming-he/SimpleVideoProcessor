#pragma once

// An encapsulation of adjusting the time fields of a packet on 
// presence of differing time bases between the input and the output

#include "ff_time.h"

namespace ff
{
	struct packet;

	class packet_retimer
	{
	public:
		packet_retimer() = delete;
		/*
		* Sets up a retimer to retime packets of fields
		* in current time base to fields in target time base
		* 
		* @param current the current time base
		* @param target the target time base
		*/
		packet_retimer(time cur, time tar, double st = 0.0) : 
			current(cur), target(tar), start_time(st) {}
		~packet_retimer() = default;

	public:
		/*
		* Retimes a packet.
		* 
		* @param pkt the packet to be retimed.
		*/
		void retime(ff::packet& pkt);

	private:
		time current;
		time target;
		// Used for clipping. 
		// Its value will be converted to the target time base and will be substracted from the time fields of the packet.
		double start_time;
	};
}