/*
 *  Concurrent primitive data structures.
 *  Copyright (C) 2021-2025 Marek Zalewski aka Drwalin
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

#ifndef CONCURRECT_FUTURE_HPP
#define CONCURRECT_FUTURE_HPP

#include <memory>
#include <chrono>
#include <atomic>
#include <thread>

namespace concurrent {
	template<typename T>
	class future;
	template<typename T>
	class promise;
	
	template<typename T>
	class promise_state final {
	public:
		promise_state() {
			error.clear();
			finished.clear();
		}
		std::atomic_flag finished;
		std::atomic_flag error;
		T value;
	};
	
	template<typename T>
	class promise final {
	public:
		promise() {
		}
		~promise() {
			if (state.get() != NULL && finished == false) {
				state->error.test_and_set();
				state->finished.test_and_set();
				state->finished.notify_all();
			}
		}
		promise(promise &&) = default;
		promise(promise &) = delete;
		promise &operator=(promise &&) = default;
		promise &operator=(promise &) = delete;
		
		void set_value(T &&v) {
			finished = true;
			state->value = std::move(v);
			state->finished.test_and_set();
			state->finished.notify_all();
			state = nullptr;
		}
		
		void set_value(T &v) {
			finished = true;
			state->value = v;
			state->finished.test_and_set();
			state->finished.notify_all();
		}
		
		void set_value(const T &v) {
			finished = true;
			state->value = v;
			state->finished.test_and_set();
			state->finished.notify_all();
		}
		
		void set_error() {
			finished = true;
			state->error.test_and_set();
			state->finished.test_and_set();
			state->finished.notify_all();
		}
		
		future<T> get_future() {
			init();
			return future<T>(state);
		}
		
	private:
		void init() {
			if (state.get() == NULL) {
				state = std::make_shared<promise_state<T>>();
			}
		}
		std::shared_ptr<promise_state<T>> state;
		bool finished = false;
	};
	
	template<typename T>
	class future final {
	public:
		future() = default;
		~future() = default;
		future(future &&) = default;
		future(future &) = default;
		future &operator=(future &&) = default;
		future &operator=(future &) = default;
		
		void wait() {
			if (state.get() != NULL) {
				state->finished.wait(false);
			}
			return;
		}
		
		template<typename TT>
		void wait_for(TT wait_time) {
			wait_for(wait_time, std::chrono::microseconds(1));
		}
		
		template<typename TT, typename TT2>
		void wait_for(TT wait_time, TT2 atom_sleep) {
			if (state.get() != NULL) {
				auto end = std::chrono::steady_clock::now() + wait_time;
				while(state->finished.test() == false && end > std::chrono::steady_clock::now()) {
					std::this_thread::sleep_for(atom_sleep);
				}
			}
		}
		
		void wait_for_nanoseconds(int64_t wait_time) {
			wait_for(std::chrono::nanoseconds(wait_time), std::chrono::microseconds(1));
		}
		
		void wait_for_microseconds(int64_t wait_time) {
			wait_for(std::chrono::microseconds(wait_time), std::chrono::microseconds(1));
		}
		
		void wait_for_milliseconds(int64_t wait_time) {
			wait_for(std::chrono::milliseconds(wait_time), std::chrono::microseconds(1));
		}
		
		void wait_for_seconds(double wait_time) {
			wait_for_microseconds(wait_time*1000.0*1000.0);
		}
		
		bool is_valid() const {
			if (state.get() != NULL) {
				return !state->error.test();
			}
			return false;
		}
		
		bool has_value() const {
			if (state.get() != NULL) {
				return state->finishied.test() && !state->error.test();
			}
			return false;
		}
		
		T &get() {
			if (state.get() != NULL) {
				if (state->finished.test() == false) {
					wait();
				}
				return state->value;
			}
			// TODO: maybe throw error instead?
			return state->value;
		}
		
		bool has_any_state() const {
			return state.get() != NULL;
		}
		
		bool finished() const {
			if (state.get() != NULL) {
				return state->finished.test();
			}
			return false;
		}
		
		friend class promise<T>;
		
	private:
		future(std::shared_ptr<promise_state<T>> state) : state(state) {}
		
		std::shared_ptr<promise_state<T>> state;
	};
}

#endif
