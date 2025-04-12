// Copyright (C) 2025 Marek Zalewski aka Drwalin
//
// This file is part of Concurrent project under MIT License
// You should have received a copy of the MIT License along with this program.

#ifndef CONCURRECT_THREAD_SAFE_VALUE_HPP
#define CONCURRECT_THREAD_SAFE_VALUE_HPP

#include <mutex>

namespace concurrent {
template <typename T> class thread_safe_value
{
public:
	thread_safe_value() = default;

	thread_safe_value(T &&v) : value(std::move(v)) {}
	thread_safe_value(T &v) : value(v) {}
	thread_safe_value(const T &v) : value(v) {}

	thread_safe_value(const thread_safe_value<T> &o) { value = o; }

	thread_safe_value &operator=(const thread_safe_value<T> &o)
	{
		T v = o;
		*this = (T)v;
		return *this;
	}

	thread_safe_value(thread_safe_value<T> &o) { value = (T)o; }

	thread_safe_value &operator=(thread_safe_value<T> &o)
	{
		T v = o;
		*this = (T)v;
		return *this;
	}

	thread_safe_value &operator=(T &v)
	{
		std::lock_guard lock(mutex);
		value = v;
		return *this;
	}
	thread_safe_value &operator=(const T &v)
	{
		std::lock_guard lock(mutex);
		value = v;
		return *this;
	}

	operator T()
	{
		T ret;
		{
			std::lock_guard lock(mutex);
			ret = value;
		}
		return ret;
	}

	operator T() const
	{
		T ret;
		{
			std::lock_guard lock(*(std::mutex *)&mutex);
			ret = value;
		}
		return ret;
	}

	template <typename T2> thread_safe_value(thread_safe_value<T2> &o)
	{
		value = (T)o;
	}
	template <typename T2> thread_safe_value &operator=(thread_safe_value<T2> &o)
	{
		T2 v = o;
		*this = (T)v;
		return *this;
	}

	template <typename T2> thread_safe_value &operator=(T2 &v)
	{
		std::lock_guard lock(mutex);
		value = v;
		return *this;
	}

	template <typename T2> operator T2()
	{
		T ret;
		{
			std::lock_guard lock(mutex);
			ret = value;
		}
		return ret;
	}

	template <typename T2> operator T2() const
	{
		T ret;
		{
			std::lock_guard lock(*(std::mutex *)&mutex);
			ret = value;
		}
		return ret;
	}

	void begin_access() { mutex.lock(); }
	bool try_begin_access() { return mutex.try_lock(); }
	void end_access() { mutex.unlock(); }
	T &access() { return value; }
	T *operator->() { return &value; }
	T &operator*() { return value; }

private:
	thread_safe_value(thread_safe_value &&) = delete;
	thread_safe_value &operator=(thread_safe_value &&) = delete;

private:
	std::mutex mutex;
	T value;
};
}

#endif
