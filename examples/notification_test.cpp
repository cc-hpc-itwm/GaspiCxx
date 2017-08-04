#include <segment/NotificationManager.hpp>
#include <segment/Allocator.hpp>
#include <iostream>

#include <vector>

int
main
  ( int /*argc*/
  , char *[] /*argv*/) {

  gaspi::segment::NotificationManager notifyMan(101);

//  for( int i(0)
//     ;     i<100
//     ;   ++i) {
//    std::cout << "[" << i << "] " << notifyMan.allocate(1) << std::endl;
//
//    if(i==50) {
//      notifyMan.deallocate(30);
//    }
//  }

  char * const rawmem(new char[1000]);

  gaspi::segment::MemoryManager * manmem
    ( new gaspi::segment::MemoryManager
        (rawmem, 1000) );

  gaspi::segment::Allocator<char> alloc(manmem);

  std::vector<int,gaspi::segment::Allocator<int>> vec1(alloc);

  for(int i(0);i<10;++i) {
    vec1.push_back(i);
  }

  for(auto & i : vec1 ) {
    std::cout << i << std::endl;
  }

  delete manmem;



  return EXIT_SUCCESS;
}
