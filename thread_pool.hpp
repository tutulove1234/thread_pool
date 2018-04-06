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
#include <array>

template<typename taskqueue=std::queue<std::function<int(void*)>>>
class thread_pool {
	public:
		thread_pool(const int thread_max_count ,const int max_task_count) ;
		virtual ~thread_pool(){}

		thread_pool(const thread_pool& tp) = delete ;
		thread_pool(const thread_pool&& tp) = delete ;
		thread_pool& operator=(const thread_pool& src) = delete ;
		thread_pool& operator=(const thread_pool&& src) = delete ;

		bool run() ;
		bool add_task(std::function<int(void* arg)> task) ;
		int thread_routine() ;
	private:
		std::atomic<int> alive_thread_count ; 
		std::atomic<int> task_curr_count ; 
		std::atomic<bool> terminated ;
		taskqueue task_queue ;
		std::mutex lock ;
		std::condition_variable  cond ; 
		std::thread * threads ;

		int max_task_count ;
		int max_thread_count ;
} ;

template<typename taskqueue>
thread_pool<taskqueue>::thread_pool(const int thread_max_count , const int task_max_count) {
	max_thread_count = thread_max_count ;
	max_task_count = task_max_count ;	

	task_curr_count.store(0 , std::memory_order_relaxed) ;	
	terminated.store(false , std::memory_order_relaxed) ;
	alive_thread_count.store(0,std::memory_order_relaxed) ;
}

template <typename taskqueue>
bool thread_pool<taskqueue>::run() {
	threads = new std::thread[max_thread_count]{thread_routine()} ;	
	alive_thread_count.store(max_thread_count,std::memory_order_relaxed) ;
}

// add template 
template <typename taskqueue>
bool thread_pool<taskqueue>::add_task(std::function<int(void* arg)> task , ) {
	if ( task_curr_count.load(std::memory_order_acquire) >= max_task_count) {
		return false ;
	}
	std::unique_lock<std::mutex> l{lock} ;
	task_queue.emplace_back(task) ;
	cond.notify_one() ;
	task_curr_count.fetch_add(1 , std::memory_order_release) ;
	return true ;
}

// real thread routine 
template <typename taskqueue>
int thread_pool<taskqueue>::thread_routine() {
	// while terminated flag was set 
		
	while ( terminated.load(std::memory_order_relaxed)) {
		if ( task_curr_count.load(std::memory_order_acquire) > 0 ) {
			std::unique_lock<std::mutex> l{lock} ;			
			std::function<int (void*arg)> task = task_queue.emplace_front() ;
			task_curr_count.fetch_sub(1, std::memory_order_release) ;

		} else {
			cond.wait(lock) ;
		}
	}
}

