#include "main.hpp"

typedef void(*logprintf_t)(char* format, ...);
extern logprintf_t logprintf;

int TimersCount;
Timer::TimerMap _timer_map;

int Timer::CreateTimer(int function_id, int end_time, bool repeat)
{
	TimersCount++;

	Timer::TimerMapItem _new_timer{ function_id,end_time - 5,chrono::steady_clock::now(),repeat };
	_timer_map.insert(pair<int, Timer::TimerMapItem>(TimersCount, _new_timer));
	
	return TimersCount;
}

int Timer::CreateTimerEx(AMX *amx, int function_id, int end_time, bool repeat, const char *format, cell *params)
{
	TimersCount++;

	deque<cell> deq_params;
	cell *addr_params;

	for (int i = strlen(format) - 1, offset = 5; i != -1; i--) {
		switch (format[i])
		{
		case 'b':
		case 'B':
		case 'c':
		case 'C':
		case 'd':
		case 'D':
		case 'i':
		case 'I':
		case 'f':
		case 'F':
			amx_GetAddr(amx, params[offset + i], &addr_params);
			deq_params.push_back(*addr_params);
			break;
		case 's':
		case 'S':
			deq_params.push_back(params[offset + i]);
			break;
		default:
			logprintf("[Timer Fix] SetFixTimerEx: Unknown format specifer: '%c'", format[i]);
		}
	}

	Timer::TimerMapItem _new_timer{ function_id,end_time - 5,chrono::steady_clock::now(),repeat,deq_params };
	_timer_map.insert(pair<int, Timer::TimerMapItem>(TimersCount, _new_timer));
	return TimersCount;
}

void Timer::DeleteTimer(const int id)
{
	Timer::TimerMap::const_iterator iter = _timer_map.find(id);

	if (iter != _timer_map.end()) {
		_timer_map.erase(iter);
		return;
	}

	logprintf("[Timer Fix] KillTimer: cannot find timer with id %d", id);
}

void Timer::DeleteAllTimers()
{
	_timer_map.clear();
}

void Timer::ProcessTimer(AMX *amx)
{
	for (Timer::TimerMap::iterator iter = _timer_map.begin(); iter != _timer_map.end(); iter++) {
		if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - iter->second.start_time).count() > iter->second.end_time) {
			for (const auto params : iter->second.params) amx_Push(amx, params);
			amx_Exec(amx, nullptr, iter->second.function_id);
			if (iter->second.repeat) iter->second.start_time = chrono::steady_clock::now();
			else _timer_map.erase(iter);
		}
	}
}