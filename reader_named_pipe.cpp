#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

int main() {
 const char* fifo_path = "/tmp/my_named_pipe";

 // FIFO를 읽기 전용으로 오픈
 int fd = open(fifo_path, O_RDONLY);
 if (fd == -1) {
 perror("open");
 return 1;
 }

 char buffer [256] ;
 while (true) {
 ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
 if (n == -1) {
 perror("read");
 close(fd);
 return 1;
 } else if (n == 0) {
 // 쓰기 쪽이 닫힘, EOF
 break;
 }

 buffer[n] = '\0'; // null 종료
 std::cout << "Received: " << buffer;

 // 종료 신호 감지
 if (strstr(buffer, "end") != nullptr) {
 std::cout << "Received end signal. Exiting.\n";
 break;
 }
 }

 close(fd);

 return 0;
}
