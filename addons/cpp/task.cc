/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: task.cc,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#include "task.h"
#include "iostream.h"
#include "tld_key.h"
#include <asm/rtai.h>
#include <rtai_registry.h>
#include <rtai_wrapper.h>

extern "C" {

// OK, this is _ugly_, int casting to this* is even
// worse than casting a void* to a this* (like with pthreads)
void entry_point(int this_pointer){
	RTAI::Task* This = (RTAI::Task*)this_pointer;
	This->run();
}

}

void signal_handler(){
#if 0
	RTAI::Task* This = RTAI::Task::self();
	This->signal_handler();
#endif
}

namespace RTAI {

void get_global_lock(void){
	__rt_get_global_lock();
}

void release_global_lock(void){
	__rt_release_global_lock();
}

int hard_cpu_id(void){
	return hard_cpu_id();
}

int assign_irq_to_cpu(int irq, unsigned long cpus_mask){
	return rt_assign_irq_to_cpu(irq,cpus_mask);
}

int reset_irq_to_sym_mode(int irq){
	return rt_reset_irq_to_sym_mode(irq);
}

void set_oneshot_mode(void){
	rt_set_oneshot_mode();
}

void set_periodic_mode(void){
	rt_set_periodic_mode();
}

Count start_timer(void){
	return Count( start_rt_timer(0) );
}

Count start_timer(const Count& period){
	return Count( start_rt_timer( period ) );
}

void stop_timer(void){
	stop_rt_timer();
}

void linux_use_fpu(bool use_fpu_flag){
	rt_linux_use_fpu(use_fpu_flag);
}

void preempt_always(bool yes_no){
	rt_preempt_always(yes_no);
}

void preempt_always_cpuid(bool yes_no, unsigned int cpu_id){
	rt_preempt_always_cpuid(yes_no,cpu_id);
}

/////////////////////////////////////////////
// class RTAI::Task
//

Task::Task(){
	std::cerr << "Task::Task() " << (void*)this << std::endl;
	m_TaskOwner = false;
	m_Named = false;
	m_Task = 0;
	for(int n=0; n < CONFIG_RTAI_CPUS;n++)
		m_CpuUse[n] = 0; 
}

Task::~Task(){
	std::cerr << "Task::~Task() " << (void*)this << std::endl;	

	if(m_Task != 0) 
	{
		if( m_TaskOwner ){
			if( m_Named){ 
				rt_named_task_delete(m_Task);
			} else {
				rt_task_delete(m_Task);
			}
		}
	}
}

Task::Task(int stack_size,
           int priority,
           bool uses_fpu,
           bool use_signal,
           unsigned int cpuid)
{
	std::cerr << "Task::Task(...) " << (void*)this << std::endl;

	m_TaskOwner = true;
	m_Named = false;	
	m_Task = 0;
	for(int n=0; n < CONFIG_RTAI_CPUS;n++)
		m_CpuUse[n] = 0; 

	init(stack_size,priority,uses_fpu,use_signal,cpuid);
}

Task::Task(const char* name,
	   int stack_size,
           int priority,
           bool uses_fpu,
           bool use_signal,
           unsigned int cpuid)
{
	std::cerr << "Task::Task(...) " << (void*)this << std::endl;
	
	m_Named = true;
	m_TaskOwner = true;
	m_Task = 0;
	for(int n=0; n < CONFIG_RTAI_CPUS;n++)
		m_CpuUse[n] = 0; 

	init(name,stack_size,priority,uses_fpu,use_signal,cpuid);
}

Task::Task(const char* name )
{
	std::cerr << "Task::Task(...) " << (void*)this << std::endl;
	
	m_Named = true;
	m_TaskOwner = false;
	m_Task = 0;
	for(int n=0; n < CONFIG_RTAI_CPUS;n++)
		m_CpuUse[n] = 0; 

	init( name );
}

bool Task::init(int stack_size,
	        int priority, 
                bool uses_fpu, 
                bool use_signal,
                unsigned int cpuid)
{
	rt_printk("Task::init(...) %p\n",this);

	if(m_Task != 0)
		return false;

	m_Named = false;
	m_TaskOwner = true;

	if(stack_size == 0)
		stack_size = 1024*4;

	if(use_signal)
	    rt_task_init(m_Task, ::entry_point,(int)this,
        	               stack_size,priority,uses_fpu,::signal_handler);
	else
		rt_task_init(m_Task, ::entry_point,(int)this,
        	               stack_size,priority,uses_fpu,0);
	return true;
}

bool Task::init(const char* name,
		int stack_size,
	        int priority, 
                bool uses_fpu, 
                bool use_signal,
                unsigned int cpuid)
{
	rt_printk("Task::init(...) %p\n",this);

	if(m_Task != 0)
		return false;

	m_Named = true;
	m_TaskOwner = true;

	if(stack_size == 0)
		stack_size = 1024*4;

	if(use_signal)
		m_Task = rt_named_task_init(name, ::entry_point,(int)this,
        	               stack_size,priority,uses_fpu,::signal_handler);
	else
		m_Task = rt_named_task_init(name, ::entry_point,(int)this,
        	               stack_size,priority,uses_fpu,0);

	if( m_Task == 0)
		return false;

	return true;
}

bool Task::init(const char* name)
{
	rt_printk("Task::init(...) %p\n",this);

	if(m_Task != 0)
		return false;

	m_Named = true;
	m_TaskOwner = false;

	unsigned long num = nam2num(name);
	if(  rt_get_type( num ) == IS_TASK ){
		m_Task = static_cast<RT_TASK*>( rt_get_adr( num  ) );
	}

	if( m_Task == 0)
		return false;

	return true;
}

#if 0
Task* Task::self(){
	struct rt_task_struct* task = rt_whoami();
	if(task == 0)
		return 0;

	return (Task*)__rt_tld_get_data(task,cpp_key);
}
#endif

void Task::suspend(){
	rt_task_suspend(m_Task);
}
	
void Task::resume(){
	rt_task_resume(m_Task);
}

void Task::yield(){
	rt_task_yield();
}

int Task::make_periodic(const Count& start_time, const Count& period){
	return rt_task_make_periodic(m_Task,start_time, period );
}

int Task::make_periodic_relative(const Time& start_delay, const Time& period){
	return rt_task_make_periodic_relative_ns(m_Task,start_delay, period);
}

void Task::set_runnable_on_cpus(unsigned int cpu_mask){
	rt_set_runnable_on_cpus(m_Task,cpu_mask);
}

void Task::set_runnable_on_cpuid(unsigned int cpuid){
	rt_set_runnable_on_cpuid(m_Task,cpuid);
}

void Task::wait_period(){
	rt_task_wait_period();
}

void Task::busy_sleep(const Time& time){
	rt_busy_sleep((long long)time);
}

void Task::sleep(const Count& delay){
	rt_sleep( delay );
}

void Task::sleep_until(const Count& count){
	rt_sleep_until( count );
}

int Task::use_fpu(bool use_fpu_flag){
	return rt_task_use_fpu(m_Task,use_fpu_flag);
}

bool Task::is_valid(){
	return (m_Task != 0);
}

void Task::inc_cpu_use(){
	m_CpuUse[ RTAI::hard_cpu_id() ]++;
}

void Task::dump_cpu_use()
{
	int cpuid;

	// Output some statistics about CPU usage 
	std::cerr << "\n\nCPU USE SUMMARY\n";
	for (cpuid = 0; cpuid < CONFIG_RTAI_CPUS; cpuid++) {
		std::cerr << "# " << cpuid << "  -> " << m_CpuUse[ cpuid ] << std::endl;
	}
	std::cerr << "END OF CPU USE SUMMARY\n\n";
}

}; // namespace RTAI
