#include "task.h"
Task::Task(){
	m_pfnTask = 0;
	m_interval = TASK_DEFAULT_INTERVAL;
	m_lastTrigger = 0;
	m_enabled=false;
}
void Task::init(unsigned long us,task_func pfn){
	if(us<TASK_MIN_INTERVAL || pfn==0){
		m_enabled=false;
		return;
	}
	m_interval = us;
	m_pfnTask = pfn;
	m_enabled=true;
}

void Task::trigger(unsigned long currentUs)
{
	if(!m_enabled){
		return;
	}
	if(currentUs-m_lastTrigger > m_interval){
		if(m_pfnTask){
			(*m_pfnTask)();
		}
		m_lastTrigger = currentUs;
	}
}
bool Task::enabled()
{
	return m_enabled;
}
void Task::setEnabled(bool b)
{
	m_enabled = b;
}






