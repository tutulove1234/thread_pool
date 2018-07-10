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

template<typename taskqueue=std::queue<std::function<void(void)>>>
class thread_pool {
	public:
		thread_pool(const int thread_max_count ,const int max_task_count) ;
		virtual ~thread_pool();

		thread_pool(const thread_pool& tp) = delete ;
		thread_pool(const thread_pool&& tp) = delete ;
		thread_pool& operator=(const thread_pool& src) = delete ;
		thread_pool& operator=(const thread_pool&& src) = delete ;

		bool run() ;
		bool stop() ;
		void waitall() ;
		template<typename func , typename ...Args>
		bool add_task(func&& function, Args&& ...arg) ;
		bool add_task(std::function<void(void)> func) ;
		int thread_routine() ;
	private:
		std::atomic<int> alive_thread_count ; 
		std::atomic<int> task_curr_count ; 
		std::atomic<bool> terminated ;
		taskqueue task_queue ;
		std::mutex lock ;
		std::condition_variable  cond ; 
		std::vector<std::thread *> threads ;

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

template<typename taskqueue>
thread_pool<taskqueue>::~thread_pool() {
	for(auto t : threads) {
		delete t ;
	}
}

template <typename taskqueue>
bool thread_pool<taskqueue>::run() {
	for (size_t i = 0 ; i < max_thread_count ; i ++ ) {
		threads.push_back(new std::thread([this](){
				thread_routine() ;
		})) ;
	}
}

// add template 
template <typename taskqueue>
template <typename func , typename ...Args>
bool thread_pool<taskqueue>::add_task(func&& f , Args&& ...arg) {
	if ( task_curr_count.load(std::memory_order_acquire) >= max_task_count) {
		return false ;
	}
	auto pkg = std::make_shared<std::packaged_task<void(void)>>(std::bind(std::forward<func>(f), std::forward<Args>(arg)...));
	auto _f = std::function<void(void)>([pkg]() {
		(*pkg)();
	}) ;
	std::unique_lock<std::mutex> lck{lock} ;
	task_queue.emplace(_f) ;
	cond.notify_one() ;
	task_curr_count.fetch_add(1 , std::memory_order_release) ;
	return true ;
}

template <typename taskqueue>
bool thread_pool<taskqueue>::add_task(std::function<void(void)> task) {
	if ( task_curr_count.load(std::memory_order_acquire) >= max_task_count) {
		return false ;
	}
	std::unique_lock<std::mutex> lck{lock};		
	task_queue.emplace(task) ;
	cond.notify_one() ;

	return true ;
}

// real thread routine 
template <typename taskqueue>
int thread_pool<taskqueue>::thread_routine() {
	// while terminated flag was set 
	
	std::cout << "thread running ..." << std::endl ;
	while ( !terminated.load(std::memory_order_relaxed)) {
		std::unique_lock<std::mutex> lck{lock} ;			
		if ( task_curr_count.load(std::memory_order_acquire) > 0 ) {
			std::function<void (void)> task = task_queue.front() ;
			task_queue.pop() ;
			lck.unlock() ;
			task_curr_count.fetch_sub(1, std::memory_order_release) ;
			task();
		} else {
			cond.wait(lck) ;
		}
	}
	return 0 ;
}

template<typename taskqueue>
bool thread_pool<taskqueue>::stop () {
	terminated.store(true ,std::memory_order_release) ;
	cond.notify_all() ;
	for( int i = 0 ; i < max_thread_count ; i++ ) {
		threads[i]->join() ;
	}

}

template<typename taskqueue>
void thread_pool<taskqueue>::waitall () {
	for( int i = 0 ; i < max_thread_count ; i++ ) {
		threads[i]->join() ;
	}
}
