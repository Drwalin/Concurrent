// Copyright (C) 2021-2025 Marek Zalewski aka Drwalin
//
// This file is part of Concurrent project under MIT License
// You should have received a copy of the MIT License along with this program.

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

