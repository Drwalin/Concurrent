// Copyright (C) 2021-2025 Marek Zalewski aka Drwalin
//
// This file is part of Concurrent project under MIT License
// You should have received a copy of the MIT License along with this program.

#ifndef CONCURRENT_NODE_STACK_HPP
#define CONCURRENT_NODE_STACK_HPP

#include <cstdlib>

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
		
		inline node_stack& operator=(node_stack&&o) { head=o.head; o.head=NULL; return *this; }
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
			if (head == nullptr) {
				head = first;
			} else {
				T* last = first->__f_last();
				last->__m_next = head;
				head = first;
			}
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

