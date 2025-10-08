// Copyright (C) 2025 Marek Zalewski aka Drwalin
//
// This file is part of Concurrent project under MIT License
// You should have received a copy of the MIT License along with this program.

#ifndef CONCURRECT_TIME_HPP
#define CONCURRECT_TIME_HPP

#include <cstdint>

namespace concurrent
{
namespace time
{
struct diff {
	int64_t ns = 0;
	inline constexpr diff operator-() const { return {-ns}; }
};

struct point {
	int64_t ns = 0;
};

point now();
void sleep_for(diff dt);

inline constexpr diff abs(diff t) { return t.ns < 0 ? diff{-t.ns} : t; }
inline constexpr diff clamp(diff t, diff min, diff max) { return t.ns < min.ns ? min : t.ns > max.ns ? max : t; }
inline constexpr diff min(diff a, diff b) { return a.ns < b.ns ? a : b; }
inline constexpr diff max(diff a, diff b) { return a.ns > b.ns ? a : b; }

inline constexpr diff nanoseconds(int t)  { return {t}; }
inline constexpr diff microseconds(int t) { return {t * 1000ll}; }
inline constexpr diff milliseconds(int t) { return {t * 1000000ll}; }
inline constexpr diff seconds(int t)      { return {t * 1000000000ll}; }

inline constexpr diff nanoseconds(int64_t t)  { return {t}; }
inline constexpr diff microseconds(int64_t t) { return {t * 1000ll}; }
inline constexpr diff milliseconds(int64_t t) { return {t * 1000000ll}; }
inline constexpr diff seconds(int64_t t)      { return {t * 1000000000ll}; }

inline constexpr diff nanoseconds(unsigned int t)  { return {t}; }
inline constexpr diff microseconds(unsigned int t) { return {t * 1000ll}; }
inline constexpr diff milliseconds(unsigned int t) { return {t * 1000000ll}; }
inline constexpr diff seconds(unsigned int t)      { return {t * 1000000000ll}; }

inline constexpr diff nanoseconds(uint64_t t)  { return {(int64_t)t}; }
inline constexpr diff microseconds(uint64_t t) { return {int64_t(t * 1000ll)}; }
inline constexpr diff milliseconds(uint64_t t) { return {int64_t(t * 1000000ll)}; }
inline constexpr diff seconds(uint64_t t)      { return {int64_t(t * 1000000000ll)}; }

inline constexpr diff microseconds(double t) { return {int64_t(t * 1000.0)}; }
inline constexpr diff milliseconds(double t) { return {int64_t(t * 1000000.0)}; }
inline constexpr diff seconds(double t)      { return {int64_t(t * 1000000000.0)}; }

inline constexpr diff microseconds(float t) { return {int64_t(t * 1000.0f)}; }
inline constexpr diff milliseconds(float t) { return {int64_t(t * 1000000.0f)}; }
inline constexpr diff seconds(float t)      { return {int64_t(t * 1000000000.0f)}; }

} // namespace time
} // namespace concurrent

inline constexpr bool operator< (concurrent::time::diff l, concurrent::time::diff r) { return l.ns < r.ns; }
inline constexpr bool operator<=(concurrent::time::diff l, concurrent::time::diff r) { return l.ns <= r.ns; }
inline constexpr bool operator> (concurrent::time::diff l, concurrent::time::diff r) { return l.ns > r.ns; }
inline constexpr bool operator>=(concurrent::time::diff l, concurrent::time::diff r) { return l.ns >= r.ns; }
inline constexpr bool operator==(concurrent::time::diff l, concurrent::time::diff r) { return l.ns == r.ns; }
inline constexpr bool operator!=(concurrent::time::diff l, concurrent::time::diff r) { return l.ns != r.ns; }

inline constexpr bool operator< (concurrent::time::point l, concurrent::time::point r) { return l.ns < r.ns; }
inline constexpr bool operator<=(concurrent::time::point l, concurrent::time::point r) { return l.ns <= r.ns; }
inline constexpr bool operator> (concurrent::time::point l, concurrent::time::point r) { return l.ns > r.ns; }
inline constexpr bool operator>=(concurrent::time::point l, concurrent::time::point r) { return l.ns >= r.ns; }
inline constexpr bool operator==(concurrent::time::point l, concurrent::time::point r) { return l.ns == r.ns; }
inline constexpr bool operator!=(concurrent::time::point l, concurrent::time::point r) { return l.ns != r.ns; }

inline constexpr concurrent::time::point operator+(concurrent::time::point p, concurrent::time::diff d) { return {p.ns + d.ns}; }
inline constexpr concurrent::time::point operator-(concurrent::time::point p, concurrent::time::diff d) { return {p.ns - d.ns}; }

inline constexpr concurrent::time::point &operator+=(concurrent::time::point &p, concurrent::time::diff d) { p.ns += d.ns; return p; }
inline constexpr concurrent::time::point &operator-=(concurrent::time::point &p, concurrent::time::diff d) { p.ns -= d.ns; return p; }

inline constexpr concurrent::time::diff operator-(concurrent::time::point a, concurrent::time::point b) { return {a.ns - b.ns}; }
inline constexpr concurrent::time::diff operator-(concurrent::time::diff a, concurrent::time::diff b)   { return {a.ns - b.ns}; }
inline constexpr concurrent::time::diff operator+(concurrent::time::diff a, concurrent::time::diff b)   { return {a.ns + b.ns}; }

inline constexpr concurrent::time::diff &operator+=(concurrent::time::diff &a, concurrent::time::diff b) { a.ns += b.ns; return a; }
inline constexpr concurrent::time::diff &operator-=(concurrent::time::diff &a, concurrent::time::diff b) { a.ns -= b.ns; return a; }

inline constexpr concurrent::time::diff operator*(concurrent::time::diff a, int v)     { return {a.ns * v}; }
inline constexpr concurrent::time::diff operator/(concurrent::time::diff a, int v)     { return {a.ns / v}; }
inline constexpr concurrent::time::diff operator*(concurrent::time::diff a, int64_t v) { return {a.ns * v}; }
inline constexpr concurrent::time::diff operator/(concurrent::time::diff a, int64_t v) { return {a.ns / v}; }
inline constexpr concurrent::time::diff operator*(concurrent::time::diff a, float v)   { return {int64_t(a.ns * v)}; }
inline constexpr concurrent::time::diff operator/(concurrent::time::diff a, float v)   { return {int64_t(a.ns / v)}; }
inline constexpr concurrent::time::diff operator*(concurrent::time::diff a, double v)  { return {int64_t(a.ns * v)}; }
inline constexpr concurrent::time::diff operator/(concurrent::time::diff a, double v)  { return {int64_t(a.ns / v)}; }

inline constexpr concurrent::time::diff &operator*=(concurrent::time::diff &a, int v)     { a.ns *= v; return a; }
inline constexpr concurrent::time::diff &operator/=(concurrent::time::diff &a, int v)     { a.ns /= v; return a; }
inline constexpr concurrent::time::diff &operator*=(concurrent::time::diff &a, int64_t v) { a.ns *= v; return a; }
inline constexpr concurrent::time::diff &operator/=(concurrent::time::diff &a, int64_t v) { a.ns /= v; return a; }
inline constexpr concurrent::time::diff &operator*=(concurrent::time::diff &a, float v)   { a.ns *= v; return a; }
inline constexpr concurrent::time::diff &operator/=(concurrent::time::diff &a, float v)   { a.ns /= v; return a; }
inline constexpr concurrent::time::diff &operator*=(concurrent::time::diff &a, double v)  { a.ns *= v; return a; }
inline constexpr concurrent::time::diff &operator/=(concurrent::time::diff &a, double v)  { a.ns /= v; return a; }

#endif
