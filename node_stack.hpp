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

#ifndef CONCURRENT_NODE_STACK_HPP
#define CONCURRENT_NODE_STACK_HPP

#include <atomic>
#include <cstdlib>
#include <bit>
#include <cinttypes>

#include "node.hpp"

namespace nonconcurrent {
	template<typename T>
	class node_stack {
	public:
		
		using iterator = typename concurrent::node<T>::iterator;
		using const_iterator = typename concurrent::node<T>::const_iterator;

		node_stack() : head(NULL) {}
		node_stack(node_stack&&o) : head(o.head) { o.head = NULL; }
		node_stack(const node_stack&) = delete;
		~node_stack() {
			while(!empty()) {
				delete pop();
			}
		}
		
		inline node_stack& operator=(node_stack&&) { head=o.head; o.head=NULL; }
		inline node_stack& operator=(const node_stack&) = delete;
		
		inline bool empty() const { return !head; }
		
		inline T* pop() {
			T* first = head;
			if(first == NULL)
				return NULL;
			head = first->__m_next;
			first->__m_next = NULL;
			return first;
		}
		
		inline void push(T* new_elem) {
			new_elem->__m_next = NULL;
			push_all(new_elem);
		}
		
		inline T* pop_all() {
			T* first = head;
			head = NULL;
			return first;
		}
		
		inline void push_all(T* first) {
			T* last = first->__f_last();
			last->__m_next = head;
			head = first;
		}
		
		inline void push_all(T* first, T* last) {
			last->__m_next = head;
			head = first;
		}
		
		inline static T* revert(T* first) {
			node_stack<T> all_straight, all_revert;
			all_straight.push_all(first);
			for(; !all_straight.empty();)
				all_revert.push(all_straight.pop());
			return all_revert.pop_all();
		}
		
		inline void push_all_revert(T* first) {
			T* last = first;
			first = node_stack<T>::revert(first);
			push_all(first, last);
		}
		
		iterator begin() { return head->__f_begin(); }
		iterator end() { return head->__f_end(); }
		const_iterator begin() const { return head->__f_begin(); }
		const_iterator end() const { return head->__f_end(); }
		
	private:
		
		T* head;
	};
}

#endif

