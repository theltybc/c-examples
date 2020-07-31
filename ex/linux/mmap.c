#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>

#define FILE_NAME ".test_nmap"
const char text[] = "asdfghjkl";
#define text_len sizeof(text) - 1
const char new_text[] = "123456789123456789";
#define new_text_len sizeof(new_text) - 1
#define MEM_LEN new_text_len

void mmap1(void);
void mmap2(void);

int main() {
  mmap1();
  mmap2();
  return EXIT_SUCCESS;
}

void mmap2(void) {
  // file not change if use MAP_PRIVATE
  char file_buf[new_text_len + 1];

  int fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("can`t open file");
  }
  // if not lseek and not write been get error "Bus error"
  if (lseek(fd, MEM_LEN, SEEK_SET) < 0) {
    perror("fail lseek");
  }
  if (write(fd, "", 1) != 1) {
    perror("fail write file");
  }

  void *file_mem = mmap(0, MEM_LEN, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (file_mem == MAP_FAILED) {
    perror("fail mmap create");
  }

  strncpy(file_mem, new_text, new_text_len);
  assert(strncmp(file_mem, new_text, new_text_len) == 0);
  if (msync(file_mem, MEM_LEN, MS_SYNC) != 0) {
    perror("fail msync");
  }

  if (lseek(fd, 0, SEEK_SET) < 0) {
    perror("fail lseek");
  }

  if (read(fd, file_buf, new_text_len) < 0) {
    perror("fail read file");
  }

  assert(strncmp(file_buf, new_text, text_len) != 0);

  unlink(FILE_NAME);
  close(fd);
  munmap(file_mem, MEM_LEN);
}

void mmap1(void) {
  char file_buf[new_text_len + 1];

  int fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("can`t open file");
  }
  if (write(fd, text, text_len) != text_len) {
    perror("fail write file");
  }
  if (fsync(fd) != 0) {
    perror("fail fsync");
  }

  void *file_mem = mmap(0, MEM_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (file_mem == MAP_FAILED) {
    perror("fail mmap create");
  }

  assert(strncmp(file_mem, text, text_len) == 0);

  strncpy(file_mem, new_text, new_text_len);
  assert(strncmp(file_mem, new_text, new_text_len) == 0);
  if (msync(file_mem, MEM_LEN, MS_SYNC) != 0) {
    perror("fail msync");
  }

  if (lseek(fd, 0, SEEK_SET) < 0) {
    perror("fail lseek");
  }
  if (read(fd, file_buf, new_text_len) < 0) {
    perror("fail read file");
  }

  assert(strncmp(file_buf, new_text, new_text_len) != 0);
  assert(strncmp(file_buf, new_text, text_len) == 0);

  unlink(FILE_NAME);
  close(fd);
  munmap(file_mem, MEM_LEN);
}

// int ff(int argc, char *argv[]) {
//   int fdin, fdout;
//   void *src, *dst;
//   struct stat statbuf;
//   if (argc != 3)
//     err_quit("Использование: %s <fromfile> <tofile>", argv[0]);
//   if ((fdin = open(argv[1], O_RDONLY)) < 0)
//     err_sys("невозможно открыть %s для чтения", argv[1]);
//   if ((fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) < 0)
//     err_sys("невозможно создать %s для записи", argv[2]);
//   if (fstat(fdin, &statbuf) < 0) /* определить размер входного файла */
//     err_sys("fstat error");
//   /* установить размер выходного файла */
//   if (lseek(fdout, statbuf.st_size - 1, SEEK_SET) == -1)
//     err_sys("ошибка вызова функции lseek");
//   if (write(fdout, "", 1) != 1)
//     err_sys("ошибка вызова функции write");
//   if ((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED)
//     err_sys("ошибка вызова функции mmap для входного файла");
//   if ((dst = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) == MAP_FAILED)
//     err_sys("ошибка вызова функции mmap для выходного файла");
//   memcpy(dst, src, statbuf.st_size); /* сделать копию файла */
//   exit(0);
// }
