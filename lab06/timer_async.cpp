#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void Print(const boost::system::error_code& e) {
  std::cout << "Hello, world!" << std::endl;
}

int main(void)
{
  boost::asio::io_service io;

  boost::asio::deadline_timer t(io, boost::posix_time::seconds(2));
  t.async_wait(Print);
  printf("after async_wait\n");

  sleep(3);

  io.run();
  printf("after io.run()\n");

  return 0;
}
