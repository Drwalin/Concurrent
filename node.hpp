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

#ifndef CONCURRECT_NODE_HPP
#define CONCURRECT_NODE_HPP

#include <atomic>

namespace concurrent {
	template<typename T>
	class node {
	public:
		
		class iterator {
		public:
			iterator() : it(0) {}
			iterator(node<T>* it) : it(it) {}
			iterator(const iterator&) = default;
			iterator(iterator&) = default;
			iterator(iterator&&) = default;
			~iterator() = default;
			iterator& operator=(const iterator&) = default;
			iterator& operator=(iterator&) = default;
			iterator& operator=(iterator&&) = default;
			iterator& operator=(node<T>* it) { this->it = it; return *this; }
			
			inline T& operator++() {
				it = it?it->__m_next() : 0;
				return *this;
			}
			inline T operator++(int) {
				iterator next = it?it->__m_next() : 0;
				*this = next;
				return next;
			}
			
			inline T begin() { return *this; }
			inline T end() { return 0; }
			
			inline T& operator*() { return *this; }
			inline const T& operator*() const { return *this; }
			
			node<T>* it;
		};
		
		class const_iterator {
		public:
			const_iterator() : it(0) {}
			const_iterator(const node<T>* it) : it(it) {}
			const_iterator(node<T>*const it) : it(it) {}
			const_iterator(const const_iterator&) = default;
			const_iterator(const_iterator&) = default;
			const_iterator(const_iterator&&) = default;
			~const_iterator() = default;
			const_iterator& operator=(const const_iterator&) = default;
			const_iterator& operator=(const_iterator&) = default;
			const_iterator& operator=(const_iterator&&) = default;
			const_iterator& operator=(node<T>* it) {
				this->it = it;
				return *this;
			}
			
			inline T& operator++() {
				it = it?it->__m_next() : 0;
				return *this;
			}
			inline T operator++(int) {
				const_iterator next = it?it->__m_next() : 0;
				*this = next;
				return next;
			}
			
			inline T begin() { return *this; }
			inline T end() { return 0; }
			
			inline const T& operator*() const { return *this; }
			
			node<T>*const it;
		};
		
		node() = default;
		node(const node&) = default;
		node(node&) = default;
		node(node&&) = default;
		~node() = default;
		
		node &operator=(const node&) = default;
		node &operator=(node&) = default;
		node &operator=(node&&) = default;
		
		iterator __f_begin() { return iterator(this); }
		iterator __f_end() { return iterator(); }
		const_iterator __f_begin() const { return const_iterator(this); }
		const_iterator __f_end() const { return const_iterator(); }
		
		inline T*const __f_last() const {
			T*const it = (T*)this;
			for(; it->__m_next; it=it->__m_next) {
			}
			return it;
		}
		inline T* __f_last() {
			T* it = (T*)this;
			for(; it->__m_next; it=it->__m_next) {
			}
			return it;
		}
		
	public:
		
		std::atomic<T*> __m_next = 0;
	};
}

#endif

