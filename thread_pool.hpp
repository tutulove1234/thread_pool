#include <iostream>
#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <atomic>
#include <utility>
#include <memory>

template<typename taskqueue=std::queue<std::function<int(void*)>>>
class thread_pool {
	public:
		thread_pool(const int max_thread_count ,const int max_task_count) ;
		virtual ~thread_pool(){}

		thread_pool(const thread_pool& tp) = delete ;
		thread_pool(const thread_pool&& tp) = delete ;
		thread_pool& operator=(const thread_pool& src) = delete ;
		thread_pool& operator=(const thread_pool&& src) = delete ;

		bool run() ;
		bool add_task(std::function<int(void* arg)> task) ;
		int thread_routine(void * args) ;
	private:
		std::atomic<int> alive_thread_count ; 
		std::atomic<int> task_curr_count ; 
		std::atomic<int> task_count ;
		std::atomic<bool> terminated ;
		taskqueue task_queue ;
		std::mutex lock ;
		std::condition_variable  cond ; 
		std::unique_ptr<std::thread>  thread_ptr ;
} ;

template<typename taskqueue>
thread_pool<taskqueue>::thread_pool(const int max_thread_count , const int max_task_count) {
	task_count.store(max_task_count , std::memory_order_relaxed) ;	
	task_curr_count.store(0 , std::memory_order_relaxed) ;	
	terminated.store(false , std::memory_order_relaxed) ;
	
	thread_ptr = std::make_unique<std::thread>(new std::thread{thread_routine(this)}[max_thread_count])	 ;
	task_count.store(max_thread_count , std::memory_order_relaxed) ;
}

template <typename taskqueue>


