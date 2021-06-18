/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: task.h,v 1.4 2013/10/22 14:54:14 ando Exp $
 *
 * Copyright: (C) 2001,2002 Erwin Rol <erwin@muffin.org>
 *
 * Licence:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef __TASK_H__
#define __TASK_H__

#include "rtai_config.h"
#include "rtai.h"
#include "rtai_sched.h"
#include "time.h"
#include "count.h"


extern "C" void entry_point(int this_pointer);

extern "C" void signal_handler(void);

namespace RTAI {

extern void get_global_lock(void);
extern void release_global_lock(void);
extern int hard_cpu_id(void);
extern int assign_irq_to_cpu(int irq, unsigned long cpus_mask);
extern int reset_irq_to_sym_mode(int irq);

extern void set_oneshot_mode(void);
extern void set_periodic_mode(void);
extern Count start_timer(void);
extern Count start_timer(const Count& period);
extern void stop_timer(void);


extern void linux_use_fpu(bool use_fpu_flag);
extern void preempt_always(bool yes_no);
extern void preempt_always_cpuid(bool yes_no, unsigned int cpu_id);


/**
 * Task wrapper class
 */
class Task {
	friend void entry_point(int this_pointer);
public:
	Task();
	Task(int stack_size, 
	     int priority, 
	     bool uses_fpu,
	     bool use_signal,
	     unsigned int cpuid);

	Task(const char* name,
	     int stack_size, 
	     int priority, 
	     bool uses_fpu,
	     bool use_signal,
	     unsigned int cpuid);

	Task(const char* name );

	virtual ~Task();

	virtual bool init(int stack_size, 
	                  int priority, 
	                  bool uses_fpu, 
	                  bool use_signal, 
	                  unsigned int cpuid);

	virtual bool init(const char* name,
			  int stack_size, 
	                  int priority, 
	                  bool uses_fpu, 
	                  bool use_signal, 
	                  unsigned int cpuid);


	virtual bool init(const char* name);
#if 0
	static Task* self();
#endif
	static void yield();
	void suspend();
	void resume();

	virtual int make_periodic(const Count& start_time, const Count& period);
	virtual int make_periodic_relative(const Time& start_delay, const Time& period);
	static void wait_period();
	static void busy_sleep(const Time& time);
	static void sleep(const Count& delay);
	static void sleep_until(const Count& count);
	int use_fpu(bool use_fpu_flag);

	void set_runnable_on_cpus(unsigned int cpu_mask);
	void set_runnable_on_cpuid(unsigned int cpuid);

	virtual void signal_handler(){}
	virtual int run() = 0;

	bool is_valid();
	
	void inc_cpu_use();
	void dump_cpu_use();	
protected:
	int m_CpuUse[ CONFIG_RTAI_CPUS ];

	bool m_Named;
	// true if this task object is the owner of the m_Task RT_TASK*
	bool m_TaskOwner;
	RT_TASK* m_Task;
}; // class Task

// global functions inside RTAI:: namespace


}; // namespace RTAI

#endif
