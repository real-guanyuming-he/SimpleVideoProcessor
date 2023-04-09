/*
* src_sink.h
* Defines interface classes that are sources or sinks of packets/frames.
* 
* A source of packets/frames is where they come from;
* a sink of packets/frames is where they go in the end (i.e. where their duties end).
* 
* ** Today's task: finish the interfaces and rewrite the codecs and demu/muxers
* 
*/

#pragma once

#include "../frame.h"

#include <type_traits>

namespace ff
{
	/*
	* Where elements come from.
	* For example, a demuxer is a packet source; an encoder is a packet source (also a frame sink)
	* Holds the ownerships to its elements until they are extracted or discarded.
	*/
	template 
	<
		typename T, 
		typename E = std::enable_if_t // enabled iff T is frame or packet
		<
			std::is_same_v<T, ff::frame> ||
			std::is_same_v<T, ff::packet>
		>
	>
	class source
	{
	public:
		using element_t = T;

	public:
		source() = default;
		virtual ~source() = default;

	public:
		/*
		* Tries to get an element available from the source. The order in which elements are given depends solely on the source.
		* @returns the element if the try succeeds, an invalid element of its type otherwise.
		*/
		virtual T try_get_one() = 0;

	};

	using packet_source = source<ff::packet>;
	using frame_source = source<ff::frame>;

	/*
	* Where elements go in the end. After an element goes into a sink, the user does not care about it anymore.
	* For example, a muxer is a packet sink; a decoder is a packet sink (also a frame source).
	*/
	template
	<
		typename T,
		typename E = std::enable_if_t // enabled iff T is frame or packet
		<
			std::is_same_v<T, ff::frame> ||
			std::is_same_v<T, ff::packet>
		>
	>
	class sink
	{
	public:
		using element_t = T;

	public:
		sink() = default;
		virtual ~sink() = default;

	public:
		/*
		* Tries to feed an element to the sink. If the attempt succeeds, then the ownership of the element may be taken. 
		It depends on the actual implementation.
		* @param ele: the element to feed. Declared to be a reference so that the implementation can actually take its ownership.
		* @returns true iff the attempt succeeds.
		*/
		virtual bool try_feed(T& ele) = 0;

	};

	using packet_sink = sink<ff::packet>;
	using frame_sink = sink<ff::frame>;
}