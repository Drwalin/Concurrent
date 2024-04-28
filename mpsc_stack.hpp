/*
 *  Concurrent primitive data structures.
 *  Copyright (C) 2021-2024 Marek Zalewski aka Drwalin
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

#ifndef CONCURRENT_MPSC_STACK_HPP
#define CONCURRENT_MPSC_STACK_HPP

#include <atomic>
#include <cstdlib>

#include "node_stack.hpp"

namespace concurrent {
	namespace mpsc {
		template<typename T>
		class stack {
		public:
			
			stack() : head(NULL) {}
			stack(stack&&o) : head(o.pop_all()) {}
			stack(const stack&) = delete;
			~stack() {
				nonconcurrent::node_stack<T> tmp;
				auto* elems = pop_all();
				if(elems)
					tmp.push_all(elems);
			}
			
			// call to operator= is not thread safe at all
			inline stack& operator=(stack&& other)
			{
				nonconcurrent::node_stack<T> tmp;
				auto* elems = pop_all();
				if(elems)
					tmp.push_all(elems);
				head = other.pop_all();
			}
			
			inline stack& operator=(const stack&) = delete;
			
			// safe concurrently only without any other pop() not pop_all()
			inline T* pop() {
				for(;;) {
					T* first = head;
					if(first == NULL)
						return NULL;
					T* next = first->__m_next.load();
					if(head.compare_exchange_strong(first, next)) {
						first->__m_next.store(NULL);
						return first;
					}
				}
				return NULL;
			}
			
			inline void push(T* new_elem) {
				new_elem->__m_next.store(NULL);
				push_all(new_elem, new_elem);
			}
			
			// safe with other pop_all() but not with pop()
			inline T* pop_all() {
				for(;;) {
					T* first = head;
					if(head.compare_exchange_weak(first, NULL))
						return first;
				}
				return NULL;
			}
			
			inline void push_all(T* first) {
				T* last = first->__f_last();
				push_all(first, last);
			}
			
			inline void push_all(T* first, T* last) {
				for(;;) {
					T *old_head = head.load();
					last->__m_next.store(old_head);
					if(head.compare_exchange_weak(old_head, first))
						return;
				}
			}
			
			inline void push_all_revert(T* first) {
				T* last = first;
				first = nonconcurrent::node_stack<T>::revert(first);
				push_all(first, last);
			}
			
			inline bool empty() const {
				return !head;
			}
			
		private:
			
			std::atomic<T*> head;
		};
		
	}
}

#endif

