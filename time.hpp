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
	int64_t ns;
};

struct point {
	int64_t ns;
};

point now();
void sleep_for(diff dt);

inline diff nanoseconds(int t) { return {t}; }
inline diff microseconds(int t) { return {t * 1000ll}; }
inline diff milliseconds(int t) { return {t * 1000000ll}; }
inline diff seconds(int t) { return {t * 1000000000ll}; }

inline diff nanoseconds(int64_t t) { return {t}; }
inline diff microseconds(int64_t t) { return {t * 1000ll}; }
inline diff milliseconds(int64_t t) { return {t * 1000000ll}; }
inline diff seconds(int64_t t) { return {t * 1000000000ll}; }

inline diff nanoseconds(unsigned int t) { return {t}; }
inline diff microseconds(unsigned int t) { return {t * 1000ll}; }
inline diff milliseconds(unsigned int t) { return {t * 1000000ll}; }
inline diff seconds(unsigned int t) { return {t * 1000000000ll}; }

inline diff nanoseconds(uint64_t t) { return {(int64_t)t}; }
inline diff microseconds(uint64_t t) { return {int64_t(t * 1000ll)}; }
inline diff milliseconds(uint64_t t) { return {int64_t(t * 1000000ll)}; }
inline diff seconds(uint64_t t) { return {int64_t(t * 1000000000ll)}; }

inline diff microseconds(double t) { return {int64_t(t * 1000.0)}; }
inline diff milliseconds(double t) { return {int64_t(t * 1000000.0)}; }
inline diff seconds(double t) { return {int64_t(t * 1000000000.0)}; }

inline diff microseconds(float t) { return {int64_t(t * 1000.0f)}; }
inline diff milliseconds(float t) { return {int64_t(t * 1000000.0f)}; }
inline diff seconds(float t) { return {int64_t(t * 1000000000.0f)}; }

inline bool operator<(diff l, diff r) { return l.ns < r.ns; }
inline bool operator<=(diff l, diff r) { return l.ns <= r.ns; }
inline bool operator>(diff l, diff r) { return l.ns > r.ns; }
inline bool operator>=(diff l, diff r) { return l.ns >= r.ns; }
inline bool operator==(diff l, diff r) { return l.ns == r.ns; }
inline bool operator!=(diff l, diff r) { return l.ns != r.ns; }

inline bool operator<(point l, point r) { return l.ns < r.ns; }
inline bool operator<=(point l, point r) { return l.ns <= r.ns; }
inline bool operator>(point l, point r) { return l.ns > r.ns; }
inline bool operator>=(point l, point r) { return l.ns >= r.ns; }
inline bool operator==(point l, point r) { return l.ns == r.ns; }
inline bool operator!=(point l, point r) { return l.ns != r.ns; }

inline point operator+(point p, diff d) { return {p.ns + d.ns}; }
inline point operator-(point p, diff d) { return {p.ns - d.ns}; }

inline point &operator+=(point &p, diff d) { return (p = p + d, p); }
inline point &operator-=(point &p, diff d) { return (p = p + d, p); }

inline diff operator-(point a, point b) { return {a.ns - b.ns}; }
inline diff operator-(diff a, diff b) { return {a.ns - b.ns}; }
inline diff operator+(diff a, diff b) { return {a.ns + b.ns}; }

inline diff &operator-=(diff &a, diff b) { return (a = a - b, a); }
inline diff &operator+=(diff &a, diff b) { return (a = a + b, a); }

inline diff operator*(diff a, int v) { return {a.ns * v}; }
inline diff operator/(diff a, int v) { return {a.ns / v}; }
inline diff operator*(diff a, float v) { return {int64_t(a.ns * v)}; }
inline diff operator/(diff a, float v) { return {int64_t(a.ns / v)}; }
inline diff operator*(diff a, double v) { return {int64_t(a.ns * v)}; }
inline diff operator/(diff a, double v) { return {int64_t(a.ns / v)}; }

inline diff &operator*=(diff &a, int v) { return (a = a * v, a); }
inline diff &operator/=(diff &a, int v) { return (a = a / v, a); }
inline diff &operator*=(diff &a, float v) { return (a = a * v, a); }
inline diff &operator/=(diff &a, float v) { return (a = a / v, a); }
inline diff &operator*=(diff &a, double v) { return (a = a * v, a); }
inline diff &operator/=(diff &a, double v) { return (a = a / v, a); }

} // namespace time
} // namespace concurrent

#endif
