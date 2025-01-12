/*************************************************************************
	> File Name: test*.cpp
	> Author: D_L
	> Mail: deel@d-l.top 
	> Created Time: 2015/8/8 8:23:09
 ************************************************************************/

#include <iostream>
#include "log/log_types.h"
#include "log/priority_queue.h"
#include "store/store.h"

using namespace std;

// log_tools::priority_queue 容器的元素为非指针类型 LogVal
void test() {
	
	typedef log_tools::priority_queue<LogVal, LogType> LogQueue;
	// 1 优先因子为空, 输出顺序同输入的顺序，FIFO
	//std::vector<LogType> vfactor;
	////assert(vfactor.empty());
	
	// 2 优先因子元素只有一个: FATAL, 即优先输出FATAL
	LogType factors[1] = {makelevel(FATAL)};
	std::vector<LogType> vfactor(factors, factors + 1);

	// 3 优先因子元素有2个: FATAL 和 ERROR, 
	// 且 FATAL的优先级大于ERROR, 即优先输出FATAL，然后再优先输出ERROR
	//LogType factors[2] = {makelevel(FATAL), makelevel(ERROR)};
	//std::vector<LogType> vfactor(factors, factors + 2);
	
	LogQueue::settings(&LogVal::log_type, vfactor);

	typedef Store<LogVal, LogQueue> LogStore;
	//typedef Store<LogVal, std::queue<LogVal> > LogStore;
	
	LogStore& logstore = LogStore::get_mutable_instance();
	logstore.push({ 
			log_tools::local_time(), makelevel(INFO), "1", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	logstore.push({
			log_tools::local_time(), makelevel(ERROR), "2", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	logstore.push({ 
			log_tools::local_time(), makelevel(FATAL), "3", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	logstore.push({ 
			log_tools::local_time(), makelevel(WARN), "4", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	logstore.push({ 
			log_tools::local_time(), makelevel(ERROR), "5", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	
	// output
	LogVal val;
	for (int i = 0; i < 5; ++i) {
		logstore.pop(val);
		std::cout << log_tools::time2string(val.now)
			<< " [" << val.log_type 
			<< "] " << val.msg << " [t:" 
			<< val.tid << "] [F:" << val.func_name << "] " 
			<< val.file_name << ":" << val.line_num 
			// << val.extra // default "", 可以省略
			<< std::endl;
	}

}



// log_tools::priority_queue 容器的元素为一级指针类型 LogVal*
void test_pointer() {
	
	typedef log_tools::priority_queue<LogVal*, LogType> LogQueue;
	// 1 优先因子为空, 输出顺序同输入的顺序，FIFO
	//std::vector<LogType> vfactor;
	////assert(vfactor.empty());
	
	// 2 优先因子元素只有一个: FATAL, 即优先输出FATAL
	LogType factors[1] = {makelevel(FATAL)};
	std::vector<LogType> vfactor(factors, factors + 1);

	// 3 优先因子元素有2个: FATAL 和 ERROR, 
	// 且 FATAL的优先级大于ERROR, 即优先输出FATAL，然后再优先输出ERROR
	//LogType factors[2] = {makelevel(FATAL), makelevel(ERROR)};
	//std::vector<LogType> vfactor(factors, factors + 2);
	
	LogQueue::settings(&LogVal::log_type, vfactor);

	typedef Store<LogVal*, LogQueue> LogStore;
	
	LogStore& logstore = LogStore::get_mutable_instance();
	logstore.push( new LogVal { 
			log_tools::local_time(), makelevel(INFO), "1", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	logstore.push( new LogVal {
			log_tools::local_time(), makelevel(ERROR), "2", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	logstore.push( new LogVal { 
			log_tools::local_time(), makelevel(FATAL), "3", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	logstore.push( new LogVal { 
			log_tools::local_time(), makelevel(WARN), "4", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	logstore.push( new LogVal { 
			log_tools::local_time(), makelevel(ERROR), "5", 
			log_tools::get_tid(),
			__func__, __FILE__, __LINE__ 
	});
	
	// output
	LogVal* val;
	for (int i = 0; i < 5; ++i) {
		logstore.pop(val);
		std::cout << log_tools::time2string(val->now)
			<< " [" << val->log_type 
			<< "] " << val->msg << " [t:" 
			<< val->tid << "] [F:" << val->func_name << "] " 
			<< val->file_name << ":" << val->line_num 
			// << val->extra // default "", 可以省略
			<< std::endl;
		delete val;
		val = NULL;
	}

}

int main() {
	test();
	cout << "----------" << endl;
	test_pointer();
	return 0;
}


