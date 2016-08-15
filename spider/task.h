#ifndef _TASK_H_
#define _TASK_H_

#define TASK_DEFAULT_INTERVAL 50000 //50ms
#define TASK_MIN_INTERVAL 5000 //2ms

typedef void (*task_func)();
class Task{
public:
	Task();
	void init(unsigned long us, task_func pfn);
	void trigger(unsigned long currentUs);
	bool enabled();
	void setEnabled(bool b);
protected:
	unsigned long m_interval;
	unsigned long m_lastTrigger;
	task_func m_pfnTask;
	bool m_enabled;
};

#endif





