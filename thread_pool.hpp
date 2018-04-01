#include <iostream>
#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <atomic>

template<typename T=std::function<int(void*)> , typename taskqueue=std::queue<T>>
class thread_pool {
	public:
		thread_pool(const int max_thread_count ,const int max_task_count) ;
		virtual ~thread_pool();

		thread_pool(const thread_pool& tp) = delete ;
		thread_pool(const thread_pool&& tp) = delete ;
		thread_pool& operator=(const thread_pool& src) = delete ;
		thread_pool& operator=(const thread_pool&& src) = delete ;

		bool run() ;
		bool add_task(std::function<int(void* arg)> task) ;
	private:
		std::atomic<int> alivecount ; 
		std::atomic<int> task_count ;
		std::atomic<bool> terminated ;
		taskqueue task_queue ;
		// std::queue<std::packaged_task<int(void* arg)>> task_queue;	
		std::mutex lock ;
		std::condition_variable  cond ; 
		std::thread * thread_ptr ;
} ;

