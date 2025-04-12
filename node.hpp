// Copyright (C) 2021-2025 Marek Zalewski aka Drwalin
//
// This file is part of Concurrent project under MIT License
// You should have received a copy of the MIT License along with this program.

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
			
			inline iterator& operator++() {
				it = it!=NULL?it->__m_next.load() : NULL;
				return *this;
			}
			inline iterator operator++(int) {
				iterator old = *this;
				it = it!=NULL?it->__m_next.load() : NULL;
				return old;
			}
			
			inline bool operator==(const iterator &other) const {
				return other.it == it;
			}
			inline bool operator!=(const iterator &other) const {
				return other.it != it;
			}
			
			inline T begin() { return *this; }
			inline T end() { return NULL; }
			
			inline T* operator*() { return (T*)it; }
			inline const T* operator*() const { return (const T*)it; }
			
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
			
			inline const_iterator& operator++() {
				it = it!=NULL?it->__m_next.load() : NULL;
				return *this;
			}
			inline const_iterator operator++(int) {
				const_iterator old = *this;
				it = it!=NULL?it->__m_next.load() : NULL;
				return old;
			}
			
			inline bool operator==(const const_iterator &other) const {
				return other.it == it;
			}
			inline bool operator!=(const const_iterator &other) const {
				return other.it != it;
			}
			
			inline T begin() { return *this; }
			inline T end() { return NULL; }
			
			inline const T* operator*() const { return (const T*)it; }
			
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

