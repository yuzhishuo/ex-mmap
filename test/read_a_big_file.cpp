#include <fcntl.h>  // for open ,flags: O_RDWR
#include <gtest/gtest.h>
#include <sys/mman.h>  // for mmap and munmap
#include <unistd.h>    // for close

#include <chrono>  // for std::chrono::seconds

class ReadABigFile : public ::testing::Test {
 public:
  static constexpr char const* const kFileName = "/tmp/ReadABigFile.txt";
  static constexpr size_t kFileSize = 10000 * 4096 * 10;
  static void SetUpTestCase() {
    int fd = open(kFileName, O_RDWR | O_CREAT, 0644);

    int zero_fd = open("/dev/random", O_RDONLY);
    ASSERT_NE(zero_fd, -1);
    auto page_size = getpagesize();
    assert(page_size == 4096);

    std::array<char, 40960> buf;
    for (size_t i = 0; i < 10000; i++) {
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

  int fd = open(ReadABigFile::kFileName, O_RDONLY);

  ASSERT_NE(fd, -1);

  size_t current_position = 0;
  start = std::chrono::system_clock::now();
  char buf[4096];
  while (current_position < ReadABigFile::kFileSize) {
    char* p = static_cast<char*>(
        mmap(nullptr, 4096, PROT_READ, MAP_SHARED, fd, current_position));
    ASSERT_NE(p, MAP_FAILED);
    memcpy(buf, p, 4096);
    munmap(p, 4096);
    current_position += 4096;
  }
  close(fd);
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  printf("MMAP, elapsed time: %f\n", elapsed_seconds.count());
}

TEST_F(ReadABigFile, READREADBIGFILE) {
  std::chrono::time_point<std::chrono::system_clock> start, end;

  int fd = open(ReadABigFile::kFileName, O_RDONLY);
  start = std::chrono::system_clock::now();

  ASSERT_NE(fd, -1);

  char buf[40960];
  for (size_t i = 0; i < 100000; i++) {
    auto ret = read(fd, buf, 40960);
  }
  close(fd);
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;

  printf("READ, elapsed time: %f\n", elapsed_seconds.count());
}