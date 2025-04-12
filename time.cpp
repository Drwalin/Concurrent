// Copyright (C) 2025 Marek Zalewski aka Drwalin
//
// This file is part of Concurrent project under MIT License
// You should have received a copy of the MIT License along with this program.

#ifndef CONCURRECT_TIME_CPP
#define CONCURRECT_TIME_CPP

#include <chrono>
#include <thread>

#include "time.hpp"

namespace concurrent
{
namespace time
{
point now()
{
	return {std::chrono::steady_clock::now().time_since_epoch().count()};
}

void sleep_for(diff dt)
{
	std::this_thread::sleep_for(std::chrono::nanoseconds(dt.ns));
}
} // namespace time
} // namespace concurrent

#endif
