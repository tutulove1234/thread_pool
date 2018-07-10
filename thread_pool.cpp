#include "thread_pool.hpp"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int main() {
	thread_pool<> th(10,100) ;	
	th.run() ;
	
	th.add_task([](){
		std::cout << "task one" << std::endl ;			
	}) ;

	th.add_task([](int i,std::string s){
		std::cout << "i :" << i << " s :" << s << std::endl ;
	},10,"hello world");
	th.add_task([](int i,std::string s,double d){
		std::cout << "i :" << i << " s :" << s << " d:" << d << std::endl ;
	},10,"hello world",12.3);


	th.waitall();	

	return 0; 
}

