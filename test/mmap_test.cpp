#include <fcntl.h>        // for open ,flags: O_RDWR
#include <gtest/gtest.h>  // for TEST* and ASSERT_* macros
#include <malloc.h>       // for memcpy
#include <sys/mman.h>     // for mmap and munmap
#include <unistd.h>       // for getpagesize

#include <filesystem>  // for std::filesystem::path and std::filesystem::exists

#include "mmap/mmap.test.h"  // for mmap_test

TEST(mmap_base_test, mmap_test) {
  int fd = open("/dev/zero", O_RDWR);
  ASSERT_NE(fd, -1);

  void *addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  ASSERT_NE(addr, MAP_FAILED);

  ASSERT_EQ(munmap(addr, 4096), 0);
  ASSERT_EQ(close(fd), 0);
}
// test method:  https://www.cnblogs.com/huxiao-tee/p/4660352.html  example 1
TEST_F(mmap_test, mmap_test_2) {
  int fd =
      open(mmap_test::kFileName, O_RDWR | O_CREAT, 0644);  // require: O_RDWR

  auto addr = mmap(NULL, 5000, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                   0);  // require: PROT_READ | PROT_WRITE

  ASSERT_NE(addr, MAP_FAILED);

  // read
  for (size_t i = 0; i < 500; i++) {
    ASSERT_EQ(static_cast<char *>(addr)[i], 0);
  }

  printf("page size: %d\n", getpagesize());
  auto read_overstep_position =
      static_cast<char *>(addr)[getpagesize() * 3 + 1];
  printf("read_overstep_position: %d\n", read_overstep_position);

  std::array<char, 20> buff;

  memcpy(buff.data(), static_cast<char *>(addr) + getpagesize() * 3 + 1,
         buff.size());

  // write
  auto died = [&]() {
    static_cast<char *>(addr)[10] = 'a';
    static_cast<char *>(addr)[getpagesize() * 2] =
        'a';  // error: wirte overstep
  };

  EXPECT_DEATH(died(), ".*");
  msync(addr, 4096, MS_SYNC);
  ASSERT_EQ(munmap(addr, 4096), 0);
  ASSERT_EQ(close(fd), 0);
}

TEST_F(mmap_test, mmap_test_3) {
  int fd =
      open(mmap_test::kFileName, O_RDWR | O_CREAT, 0644);  // require: O_RDWR

  auto addr = mmap(NULL, 15000, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                   0);  // require: PROT_READ | PROT_WRITE

  ASSERT_NE(addr, MAP_FAILED);

  // read
  auto died2 = [&]() {  // error: SIGBUS
    for (size_t i = 11; i < 15000; i++) {
      ASSERT_EQ(static_cast<char *>(addr)[i], 0);
    }
  };

  EXPECT_DEATH(died2(), ".*");

  printf("page size: %d\n", getpagesize());

  auto died1 = [&]() {
    auto read_overstep_position =
        static_cast<char *>(addr)[15000 + 1];  // error: SIGBUS
    printf("read_overstep_position: %d\n", read_overstep_position);
  };

  EXPECT_DEATH(died1(), ".*");

  auto died = [&]() { static_cast<char *>(addr)[15000 + 1] = 'a'; };

  EXPECT_DEATH(died(), ".*");  // error:  SIGBUS  , 不知道为什么没触发 SIGSEGV
}