/*
 * Function        : Composer driver
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef ANDROID_ATRACE_H
#define ANDROID_ATRACE_H

#include <linux/kernel.h>
#include <linux/sched.h>

/*! \brief generate trace events
 *  \param[in] begin  trace type
 *  \param[in] pid    process id
 *  \param[in] name   name of trace
 *  \param[in] val    value of trace
 *  \return none
 *  \details
 *  Trace log is generated using trace_printk.
 *  The syntax of log is the same as ATRACE. defined for android.
 */
extern void tracing_mark_write(int begin, int pid, const char *name, int val);

/*! \brief Trace the begin of function.
 *  \param[in] P  process ID.
 *  \param[in] X  name of function.
 *  \msc
 *    function;
 *    |||;
 *    function box function
 *        [label="ATRACE_PID_BEGIN", URL="\ref ::ATRACE_PID_BEGIN"];
 *    |||;
 *    function box function
 *        [label="ATRACE_PID_END",   URL="\ref ::ATRACE_PID_END"];
 *  \endmsc
 */
#define ATRACE_PID_BEGIN(P, X)          tracing_mark_write(1, P, X, 0)

/*! \brief Trace the end of function.
 *  \param[in] P  process ID.
 *  \param[in] X  name of function.
 *  \attention
 *  This macro must use after correct ATRACE_PID_BEGIN.
 */
#define ATRACE_PID_END(P, X)            tracing_mark_write(0, P, X, 0)

/*! \brief Trace the integer.
 *  \param[in] P  process ID.
 *  \param[in] X  name of integer.
 *  \param[in] Y  value of integer.
 *  \msc
 *    function;
 *    |||;
 *    function box function
 *        [label="ATRACE_PID_INT", URL="\ref ::ATRACE_PID_INT"];
 *  \endmsc
 */
#define ATRACE_PID_INT(P, X, Y)         tracing_mark_write(2, P, X, Y)

/*! \brief Trace the begin of asynchronous event.
 *  \param[in] P  process ID.
 *  \param[in] X  name of asynchronous event.
 *  \param[in] Y  identify of events.
 *  \msc
 *    event;
 *    |||;
 *    event box event
 *        [label="ATRACE_PID_ASYNC_BEGIN", URL="\ref ::ATRACE_PID_ASYNC_BEGIN"];
 *    |||;
 *    event box event
 *        [label="ATRACE_PID_ASYNC_END",   URL="\ref ::ATRACE_PID_ASYNC_END"];
 *  \endmsc
 */
#define ATRACE_PID_ASYNC_BEGIN(P, X, Y) tracing_mark_write(3, P, X, (int)Y)

/*! \brief Trace the end of asynchronous event.
 *  \param[in] P  process ID.
 *  \param[in] X  name of asynchronous event.
 *  \param[in] Y  identify of events.
 *  \attention
 *  This macro must use after correct ATRACE_PID_ASYNC_BEGIN.
 */
#define ATRACE_PID_ASYNC_END(P, X, Y)   tracing_mark_write(4, P, X, (int)Y)

/*! \brief Trace the begin of function.
 *  \param[in] X  name of function.
 *  \sa ATRACE_PID_BEGIN
 */
#define ATRACE_BEGIN(X)          ATRACE_PID_BEGIN(current->pid, X)

/*! \brief Trace the end of function.
 *  \param[in] X  name of function.
 *  \sa ATRACE_PID_END
 */
#define ATRACE_END(X)            ATRACE_PID_END(current->pid, X)

/*! \brief Trace the integer.
 *  \param[in] X  name of integer.
 *  \param[in] Y  value of integer.
 *  \sa ATRACE_PID_INT
 */
#define ATRACE_INT(X, Y)         ATRACE_PID_INT(current->pid, X, Y)

/*! \brief Trace the begin of asynchronous event.
 *  \param[in] X  name of asynchronous event.
 *  \param[in] Y  identify of events.
 *  \sa ATRACE_PID_ASYNC_BEGIN
 */
#define ATRACE_ASYNC_BEGIN(X, Y) ATRACE_PID_ASYNC_BEGIN(current->pid, X, Y)

/*! \brief Trace the end of asynchronous event.
 *  \param[in] X  name of asynchronous event.
 *  \param[in] Y  identify of events.
 *  \sa ATRACE_PID_ASYNC_END
 */
#define ATRACE_ASYNC_END(X, Y)   ATRACE_PID_ASYNC_END(current->pid, X, Y)


#endif
