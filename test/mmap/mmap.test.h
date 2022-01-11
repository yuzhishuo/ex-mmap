/**
 * @file mmap.test.h
 * @author Yimin Liu (aoumeior@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-01-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <fcntl.h>        // for open ,flags: O_RDWR
#include <gtest/gtest.h>  // for TEST* and ASSERT_* macros
#include <stdio.h>        // for printf

#include <array>  // for std::array

class mmap_test : public ::testing::Test {
 protected:
  mmap_test() {}

  ~mmap_test() override {}

  static void SetUpTestCase() {
    fd = open(kFileName, O_RDWR | O_CREAT, 0644);

    int zero_fd = open("/dev/zero", O_RDONLY);
    ASSERT_NE(zero_fd, -1);

    std::array<char, 5000> buf;

    ASSERT_EQ(read(zero_fd, buf.data(), buf.size()), buf.size());

    auto ret = write(fd, buf.data(), buf.size());

    ASSERT_EQ(ret, buf.size());

    ASSERT_NE(fd, -1);

    ASSERT_EQ(close(zero_fd), 0);

    ASSERT_EQ(close(fd), 0);
  }

  static void TearDownTestCase() {}

 public:
  static constexpr char const* const kFileName = "/tmp/1.txt";
  static constexpr size_t kFileSize = 5000;

 private:
  static int fd;
};

int mmap_test::fd = -1;