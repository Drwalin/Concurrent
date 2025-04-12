// Copyright (C) 2021-2025 Marek Zalewski aka Drwalin
//
// This file is part of Concurrent project under MIT License
// You should have received a copy of the MIT License along with this program.

#ifndef CONCURRECT_FUTURE_HPP
#define CONCURRECT_FUTURE_HPP

#include <memory>
#include <atomic>

#include "time.hpp"

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
		
		void wait_for(time::diff wait_time) {
			wait_for(wait_time, {1000});
		}
		
		void wait_for(time::diff wait_time, time::diff atom_sleep) {
			if (state.get() != NULL) {
				auto end = time::now() + wait_time;
				while(state->finished.test() == false && end > time::now()) {
					time::sleep_for(atom_sleep);
				}
			}
		}
		
		void wait_for_nanoseconds(int64_t wait_time) {
			wait_for(time::nanoseconds(wait_time), time::microseconds(1));
		}
		
		void wait_for_microseconds(int64_t wait_time) {
			wait_for(time::microseconds(wait_time), time::microseconds(1));
		}
		
		void wait_for_milliseconds(int64_t wait_time) {
			wait_for(time::milliseconds(wait_time), time::microseconds(1));
		}
		
		void wait_for_seconds(double wait_time) {
			wait_for(time::seconds(wait_time), time::microseconds(1));
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
