log/log_types.h

---------------------------

# LogType

定义了日志类型(级别)

```cpp
// 默认内置 6 种日志级别
MAKE_LOGLEVEL(TRACE,  0); // TRACE 权重为0
MAKE_LOGLEVEL(DEBUG, 10); // DEBUG 权重为10
MAKE_LOGLEVEL(INFO , 20); // INFO  权重为20
MAKE_LOGLEVEL(WARN , 30); // WARN  权重为30
MAKE_LOGLEVEL(ERROR, 40); // ERROR 权重为40
MAKE_LOGLEVEL(FATAL, 50); // FATAL 权重为50
```
(替代了原先 `enum` 的定义方式)

有关 `MAKE_LOGLEVEL` 的使用详情，移步这里 [here](./loglevel.md)

推荐的使用方式:
 
| 级别 | 含义                                        |
|------|-------------------------------------------- |
| TRACE| 跟踪                                        |
| DEBUG| 细粒度信息事件                              |
| INFO | 消息在粗粒度级别上突出强调应用程序的运行过程|
| WARN | 出现潜在错误的情形                          |
| ERROR| 虽然发生错误事件，但仍然不影响系统的继续运行|
| FATAL| 严重的错误事件将会导致应用程序的退出        |

# LogVal

日志记录，定义了每条日志所包含的内容.

```cpp
// 摘要
class LogVal {
public:
	typedef log_tools::ptime  ptime;
	typedef log_tools::tid_t  tid_t;
	class Extra;     // 扩展数据的抽象类
	class ExtraNone; // Extra抽象类的实现子类，被用在LogVal的缺省构造函数参数中

	ptime                  now;       // 当前时间
	LogType                log_type;  // 日志类型
	std::string            msg;       // 日志正文
	tid_t                  tid;       // 当前线程id
	std::string            func_name; // 所在函数
	std::string            file_name; // 所在文件
	unsigned int           line_num;  // 所在行号
	std::shared_ptr<Extra> extra;     // 备用(扩展)数据

	// 缺省构造 / 拷贝构造 / operator= / 析构
	virtual ~LogVal() {}
	LogVal(const LogVal::ptime& time      = log_tools::local_time(), 
		const LogType& logtype            = makelevel(WARN), 
		const std::string& message        = "",
		const LogVal::tid_t& thread_id    = log_tools::get_tid(),
		const std::string& function_name  = "UNKNOWN_FUNCTION",
		const std::string& filename       = "UNKNOWN_FILENAME",
		const unsigned int& line          = 0,
		std::shared_ptr<Extra> extra_data = std::make_shared<ExtraNone>());
	LogVal(const LogVal& that);
	LogVal(LogVal&& that);
	LogVal& operator= (const LogVal& that);
	LogVal& operator= (LogVal&& that);
};
```

如果 `LogVal` 字段未能满足需求，扩展的任务尽量交给 `LogVal::Extra` 的子类去完成，不推荐直接更改 `LogVal`， 见下面的 `LogVal::Extra`.

# LogVal::Extra

日志数据的扩展字段的抽象类

```cpp
// 接口比较简单
class LogVal::Extra {
public:
	virtual const std::string format(void) const = 0;
	virtual ~Extra() {}
	friend std::ostream& operator<< (
			std::ostream&         os, 
			const LogVal::Extra&  e) {
		return os << std::move(e.format());
	}
	friend std::ostream& operator<< (
			std::ostream&                  os, 
			std::shared_ptr<LogVal::Extra> ep) {
		return os << (ep->format());
	}
};
```

若 `LogVal` 未能满足需求，那么只需扩展 `LogVal::Extra` 即可，而且任何时候都推荐这样使用：

```cpp
// 扩展样例：
using std::string;
class LogValExtraExample : public LogVal::Extra {
	public:
		LogValExtraExample(const string& str, const int& i)
				: _str_test1(str), _int_test2(i) {}
		LogValExtraExample(string&& str, const int& i)
				: _str_test1(std::move(str)), _int_test2(i) {}
		// @overwrite
		virtual const string format const () {
			std::ostringstream oss;
			oss << " {extra_data:" << _str_test1 << ","	<< _int_test2 << "}"; 
			return  oss.str(); 
			// 可以返回临时对象而不用担心效率问题，原因可以查看父类中调用 format()位置的代码
		}
	private:
		string  _str_test1;
		int     _int_test2;
};
 
//
// 应用于容器的 push/pop
// push/pop 接受 一个 LogVal 类型的值
//

buff.push( { ..., // 最后一个参数填写一个std::shared_ptr<LogValExtraExample>对象即可
		std::make_shared< LogValExtraExample >("test1", 100)
} );
LogVal val;
buff.pop(val);
std::cout << log_tools::time2string(val.now)
	<< " [" << log_tools::logtype2string(val.log_type) 
	<< "] " << val.msg << " [t:" 
	<< val.tid << "] [F:" << val.func_name << "] " 
	<< val.file_name << ":" << val.line_num 
	<< val.extra  // <-- 附加数据, 重写了流输出操作符 
	// or 
	// << val.extra->format()
	// or
	// << *val.extra
	<< std::endl;
```

如果之前初始化时( push 时)没有给定参数 extra 任何值，则 val.extra 将被赋予默认值(类型为LogVal::ExtraNone)，对 *val.extra 流输出值为空字符串 ""， 即 val.extra->format() 为 ""。

附 `LogVal::ExtraNone` 的实现：

```cpp
// 仅仅是重写了 format() 而已
class LogVal::ExtraNone : public LogVal::Extra {
public:
	virtual const std::string format() const {
		return "";
	}
};
```

# 日志处理实用工具集

以下定义都被包含在 namespace log_tools 中。

```cpp
typedef boost::posix_time::ptime          ptime;
typedef boost::posix_time::microsec_clock microsec_clock;
typedef boost::thread::id                 tid_t;

// get the current time
const ptime local_time();

// boost::posix_time::ptime to std::string
const std::string time2string(const ptime& time_point);

// get the current thread id
const tid_t get_tid();

// global print lock
inline boost::mutex& print_lock(void) {
	static boost::mutex __print_lock;
	return __print_lock;
}
// 线程安全的，不经过日志仓库的 流输出函数，只要是 std::ostream 的子类都适用
void print_s(std::ostringstream&& oss, std::ostream& os = std::cout);

```

`log_tools::print_s` 函数一般使用在日志输出线程还未启动的情况下，`log_tools::print_s` 可以及时地将 oss 输出到指定的流对象 os。

特别地，`log_tools::print_s` 针对经常使用的 `std::cout` 和 `std::cerr` 设计了两个便捷宏函数。

比如： `_print_s("123" << 345 << 5.0 << std::endl); // 1233455.0`

* _print_s 

```cpp
// Thread-safe print, to std::cout
#define _print_s(msg)\
	do {\
		std::ostringstream oss;\
		oss << msg;\
		log_tools::print_s(std::move(oss));\
	} while(0) 

```
* _print_s_err

```cpp
// Thread-safe print, to std::cerr
#define _print_s_err(msg)\
	do {\
		std::ostringstream oss;\
		oss << msg;\
		log_tools::print_s(std::move(oss), std::cerr);\
	} while(0) 
```
