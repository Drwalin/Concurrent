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

#ifndef CONCURRENT_HASH_HPP
#define CONCURRENT_HASH_HPP

#include <cinttypes>
#include <string>

namespace concurrent {
	namespace default_hash {
		
		using Result = uint64_t;
		
		template<typename T>
		inline Result hash(T v);
		
		inline const uint64_t BASE = 14695981039346656037llu;
		inline const uint64_t PRIME = 1099511628211llu;
		
		template<>
		inline Result hash<int8_t>(int8_t v) {
			return (v^BASE)*PRIME;
		}
		template<>
		inline Result hash<int16_t>(int16_t v) {
			return (v^BASE)*PRIME;
		}
		template<>
		inline Result hash<int32_t>(int32_t v) {
			return (v^BASE)*PRIME;
		}
		template<>
		inline Result hash<int64_t>(int64_t v) {
			return (v^BASE)*PRIME;
		}
		template<>
		inline Result hash<uint8_t>(uint8_t v) {
			return (v^BASE)*PRIME;
		}
		template<>
		inline Result hash<uint16_t>(uint16_t v) {
			return (v^BASE)*PRIME;
		}
		template<>
		inline Result hash<uint32_t>(uint32_t v) {
			return (v^BASE)*PRIME;
		}
		template<>
		inline Result hash<uint64_t>(uint64_t v) {
// 			return (v^BASE)*PRIME;
			return v;
		}
		
		template<>
		inline Result hash<const char*>(const char* v) {
			uint64_t h = BASE;
			char const* ptr = v;
			for(;; ptr+=8) {
				struct b {
					char v[8];
				};
				union {
					uint64_t V;
					b bytes;
				};
				V = 0;
				
				for(int i=0; i<8 && ptr[i]; ++i) {
					bytes.v[i] = ptr[i];
				}
				
				h ^= V;
				h *= PRIME;
			}
			return h;
		}
		
		template<>
		inline Result hash<const std::string&>(const std::string& v) {
			return hash<const char*>(v.c_str());
		}
	}
}

#endif

