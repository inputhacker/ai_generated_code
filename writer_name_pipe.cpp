#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

int main() {
 const char* fifo_path = "/tmp/my_named_pipe";

 // FIFO 생성 (존재하지 않을 경우)
 if (mkfifo(fifo_path, 0666) == -1) {
 perror("mkfifo");
 // 이미 존재할 수도 있으니 종료하지는 않음
 }

 // FIFO를 쓰기 전용으로 오픈
 int fd = open(fifo_path, O_WRONLY);
 if (fd == -1) {
 perror("open");
 return 1;
 }

 const char* messages[] = {
 "Hello from writer process!",
 "This is a message sent through Named Pipe.",
 "end" // 종료 문자열
 };

 for (const char* msg : messages) {
 ssize_t n = write(fd, msg, strlen(msg));
 if (n == -1) {
 perror("write");
 close(fd);
 return 1;
 }

 // 각 메시지 후에 개행문자 전달 (옵션)
 write(fd, "\n", 1);

 std::cout << "Sent: " << msg << std::endl;

 if (strcmp(msg, "end") == 0)
 break;
 
 sleep(1); // 메시지 간 간격
 }

 close(fd);

 // 필요시 FIFO 파일 삭제 (보통은 유지)
 // unlink(fifo_path);

 return 0;
}
