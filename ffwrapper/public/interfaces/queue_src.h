/*
* queue_src.h
* 
* Defines queue sources that use queues to buffer elements so that the user know more about if elements are available and how many.
*/

#pragma once

#include "src_sink.h"

#include <queue>

namespace ff
{
	/*
	* Uses a queue to buffer elements so that the user know more about if elements are available and how many.
	*/
	template <typename T>
	class queue_source : public source<T>
	{
	public:
		using super = source<T>;
		using element_t = typename super::element_t;

	public:
		queue_source() = default;
		virtual ~queue_source() = default;

		/*
		* Tries to extract the first available element.
		* 
		* If !empty(), then this method always succeeds.
		*/
		T try_get_one() override
		{
			if (!empty())
			{
				T ret{ queue.front().buffer };
				queue.front().relinquish_ownership();

				queue.pop();
				return ret;
			}

			return T();
		}

		/*
		* Note: even if the size is 0, it doesn't mean there will not be elements available here in the future.
		* 
		* @returns the number of elements current available in the source.
		*/
		auto size() const { return queue.size(); }
		// @returns true iff size() == 0.
		bool empty() const { return size() == 0; }

		/*
		* Peeks the first element available. If empty(), then its behaviour is undefined.
		* @returns the element peeked.
		*/
		virtual const T& peek_first() const
		{
			return queue.front();
		}

		/*
		* Discards all currently available elements.
		*/
		virtual void clear()
		{
			// this will call the destructor of queue and destroy it.
			std::queue<T>().swap(queue);
		}

	protected:
		std::queue<T> queue;
	};

	using queue_packet_source = queue_source<ff::packet>;
	using queue_frame_source = queue_source<ff::frame>;
}