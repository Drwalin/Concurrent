/*
 *  Concurrent primitive data structures.
 *  Copyright (C) 2022 Marek Zalewski aka Drwalin
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

#ifndef CONCURRENT_MUTEX_HPP
#define CONCURRENT_MUTEX_HPP

#include <cinttypes>
#include <thread>

namespace concurrent {
	namespace mpmc {
		
		class unaligned_mutex_spin_lock {
		public:
			
			unaligned_mutex_spin_lock() {
				flag = 0;
			}
			
			uint64_t flag;
			
			inline bool try_lock() {
				uint64_t prev = __atomic_fetch_add(&flag, 1, __ATOMIC_SEQ_CST);
				if(prev == 0)
					return true;
				__atomic_fetch_sub(&flag, 1, __ATOMIC_SEQ_CST);
				return false;
			}
			
			inline void lock() {
				while(true) {
BEG__GG:
					uint64_t prev = __atomic_fetch_add(&flag, 1, __ATOMIC_SEQ_CST);
					if(prev == 0)
						return;
					__atomic_fetch_sub(&flag, 1, __ATOMIC_SEQ_CST);
					int s = prev;
					for(int i=0; i<64; ++i) {
						for(int j=0; j<s && j<7; ++j, ++i) {
							s^=prev;
							s*=prev;
						}
// 						std::this_thread::yield();
// 						if(__atomic_load_n(&flag, __ATOMIC_SEQ_CST) == 0)
// 							goto BEG__GG;
					}
					while(true) {
						if(__atomic_load_n(&flag, __ATOMIC_SEQ_CST) == 0)
							goto BEG__GG;
						std::this_thread::yield();
					}
				}
			}
			
			inline void unlock() {
				__atomic_fetch_sub(&flag, 1, __ATOMIC_SEQ_CST);
			}
		};
		
		
		
		
		
		template<int alignement>
		class mutex {
		public:
			
			mutex() : m() {}
			~mutex() {}
			
			struct padding {
				uint8_t _padding[alignement];
			};
			
			union {
				unaligned_mutex_spin_lock m;
				padding p;
			};
			
			inline void lock() {m.lock();}
			inline void try_lock() {m.try_lock();}
			inline void unlock() {m.unlock();}
		};
		
		/*
		template<int alignement>
		class shared_mutex {
		public:
			
			shared_mutex() {};
			~shared_mutex() {};
			
			struct padding {
				uint8_t _padding[alignement];
			};
			
			union {
				std::shared_mutex m;
				padding p;
			};
			
			inline void lock() {m.lock();}
			inline void try_lock() {m.try_lock();}
			inline void unlock() {m.unlock();}
			
			inline void lock_shared() {m.lock_shared();}
			inline void try_lock_shared() {m.try_lock_shared();}
			inline void unlock_shared() {m.unlock_shared();}
			
			inline void lock_write() {m.lock();}
			inline void try_lock_write() {m.try_lock();}
			inline void unlock_write() {m.unlock();}
			
			inline void lock_read() {m.lock_shared();}
			inline void try_lock_read() {m.try_lock_shared();}
			inline void unlock_read() {m.unlock_shared();}
		};
		*/
	}
}

#endif

