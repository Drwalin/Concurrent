/*
 *  Concurrent primitive data structures.
 *  Copyright (C) 2021 Marek Zalewski aka Drwalin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CONCURRENT_MPSC_QUEUE_HPP
#define CONCURRENT_MPSC_QUEUE_HPP

#include "mpsc_stack.hpp"
#include "node_stack.hpp"

namespace concurrent {
	namespace mpsc {
		template<typename T>
		class queue {
		public:
			
			queue(const queue&) = delete;
			queue(queue&&o) = default;
			queue() = default;
			~queue() = default;
			
			inline queue& operator=(queue&&) = default;
			inline queue& operator=(const queue&) = delete;
			
			inline T* pop() {
				if(output_stack.empty()) {
					T* all = input_stack.pop_all();
					if(all)
						output_stack.push_all_revert(all);
				}
				if(!output_stack.empty()) {
					return output_stack.pop();
				}
				return NULL;
			}
			
			inline void push(T* new_elem) {
				input_stack.push(new_elem);
			}
			
			inline bool empty() const {
				return output_stack.empty() && input_stack.empty();
			}
			
			inline stack<T> &get_input_stack() {
				return input_stack;
			}
			
			inline nonconcurrent::node_stack<T> &get_output_stack() {
				return output_stack;
			}
			
		private:
			stack<T> input_stack;
			nonconcurrent::node_stack<T> output_stack;
		};
	}
}

#endif

