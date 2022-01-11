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

/**
 * @brief write out of bounds to the region
 * (write outside the mmap region), and generate SIGBUS
 *
 */
TEST_F(mmap_test, mmap_test_with_file_size_greater_than_page_size) {
  int fd = open(mmap_test::kFileName, O_RDWR);

  ASSERT_NE(fd, -1);

  auto addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  int page_size = getpagesize();
  auto died = [&]() {
    auto read_overstep_position = static_cast<char *>(addr)[page_size * 2 + 1];
  };

  died();  // normal, the reading is not generated SIGBUS

  auto died1 = [&]() { static_cast<char *>(addr)[4097] = 1; };

  ASSERT_DEATH(died1(), ".*");  // write out of bounds, generate SIGBUS
  ASSERT_EQ(munmap(addr, 4096), 0);
  ASSERT_EQ(close(fd), 0);
}

/**
 * @brief Specify a region that is read-only , write in of bounds to the region
 * and generate SIGSEGV
 *
 */
TEST_F(mmap_test, mmap_test_with_file_size_less_than_page_size) {
  int fd = open(mmap_test::kFileName, O_RDONLY);

  ASSERT_NE(fd, -1);

  auto addr = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);

  auto died = [&]() { static_cast<char *>(addr)[4096 - 1] = 1; };

  ASSERT_DEATH(died(),
               ".*");  // Program received signal SIGSEGV, Segmentation fault.
}