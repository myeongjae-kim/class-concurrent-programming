#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>


#define NUM_THREAD_IN_POOL 4

bool IsPrime(int n) {
    if (n < 2) {
        return false;
    }

    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

boost::asio::io_service printing_io;
boost::thread_group printing_threadpool;

void print_result(int range_start, int range_end,
    int execution_number, long cnt_prime) {

  printf("(%d)number of primes in %d ~ %d is %ld\n",
      execution_number, range_start, range_end, cnt_prime);
}


void ThreadFunc(int range_start, int range_end, int execution_number) {
    
    // Split range for this thread
    int start = range_start;
    int end = range_end;
    
    long cnt_prime = 0;
    for (int i = start; i < end; i++) {
        if (IsPrime(i)) {
            cnt_prime++;
        }
    }

    // Send results to printing thread.
    printing_io.post( boost::bind(print_result, range_start,
          range_end, execution_number, cnt_prime) );
        
}

int main(void) {
  boost::asio::io_service io;
  boost::thread_group threadpool;
  boost::asio::io_service::work* work = new boost::asio::io_service::work(io);

  // About printing
  boost::asio::io_service::work* printing_work =
    new boost::asio::io_service::work(printing_io);

  printing_threadpool.create_thread(boost::bind(
        &boost::asio::io_service::run, &printing_io));
  // End


  int range_start;
  int range_end;
  int execution_number = 0;

  for (int i = 0; i < NUM_THREAD_IN_POOL; ++i) {
    threadpool.create_thread(boost::bind(
          &boost::asio::io_service::run, &io));
  }

  while (1) {
    // Input range
    scanf("%d", &range_start);
    if (range_start == -1) {
      break;
    }
    scanf("%d", &range_end);

    io.post( boost::bind(ThreadFunc, range_start,
          range_end, ++execution_number) );
  }

  // erase thread
  delete work;
  threadpool.join_all();
  io.stop();


  delete printing_work;
  printing_threadpool.join_all();
  printing_io.stop();


  return 0;
}

