#include <fcntl.h>  // for open ,flags: O_RDWR
#include <gtest/gtest.h>
#include <sys/mman.h>  // for mmap and munmap
#include <unistd.h>    // for close

#include <chrono>  // for std::chrono::seconds

class ReadABigFile : public ::testing::Test {
 public:
  static constexpr char const* const kFileName = "/tmp/2.txt";
  static void SetUpTestCase() {
    int fd = open(kFileName, O_RDWR | O_CREAT, 0644);

    int zero_fd = open("/dev/zero", O_RDONLY);
    ASSERT_NE(zero_fd, -1);
    auto page_size = getpagesize();
    assert(page_size == 4096);

    std::array<char, 4096> buf;
    for (size_t i = 0; i < 100; i++) {
      ASSERT_EQ(read(zero_fd, buf.data(), buf.size()), buf.size());

      auto ret = write(fd, buf.data(), buf.size());

      ASSERT_EQ(ret, buf.size());
    }

    ASSERT_NE(fd, -1);

    ASSERT_EQ(close(zero_fd), 0);

    ASSERT_EQ(close(fd), 0);
  }
  static void TearDownTestCase() {}
};

TEST_F(ReadABigFile, MMAPReadABigFile) {
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  int fd = open(ReadABigFile::kFileName, O_RDONLY);

  ASSERT_NE(fd, -1);

  char* p = static_cast<char*>(
      mmap(nullptr, 100 * 4096, PROT_READ, MAP_SHARED, fd, 0));
  close(fd);
  ASSERT_NE(p, MAP_FAILED);
  char buf[4096];
  for (size_t i = 0; i < 100; i++) {
    memcpy(buf, p, 4096);
    p += 4096;
  }

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  printf("MMAP, elapsed time: %f\n", elapsed_seconds.count());
}

TEST_F(ReadABigFile, READREADBIGFILE) {
  std::chrono::time_point<std::chrono::system_clock> start, end;

  start = std::chrono::system_clock::now();

  int fd = open(ReadABigFile::kFileName, O_RDONLY);

  ASSERT_NE(fd, -1);

  char buf[4096];
  for (size_t i = 0; i < 100; i++) {
    auto ret = read(fd, buf, 4096);
    // ASSERT_EQ(ret, 4096);
  }
  close(fd);

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;

  printf("READ, elapsed time: %f\n", elapsed_seconds.count());
}