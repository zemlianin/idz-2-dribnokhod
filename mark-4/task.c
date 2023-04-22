#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define MEM_SIZE 4096 // размер разделяемой памяти
#define MEM_NAME "/my_memory" // имя разделяемой памяти

int main() {
  int fd; // дескриптор разделяемой памяти
  char *ptr; // указатель на разделяемую память
  pid_t pid; // идентификатор процесса

  fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, 0666); // создание разделяемой памяти
  if (fd == -1) { // проверка успешности создания
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  ftruncate(fd, MEM_SIZE); // задание размера памяти

  ptr = (char *)mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // отображение памяти в адресное пространство процесса
  if (ptr == MAP_FAILED) { // проверка успешности отображения
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  pid = fork(); // создание дочернего процесса
  if (pid < 0) { // проверка успешности создания
    perror("fork");
    exit(EXIT_FAILURE);
  } else if (pid == 0) { // код дочернего процесса
    printf("Child process writing to shared memory...\n");
    sprintf(ptr, "Hello from child process!"); // запись в разделяемую память
    exit(EXIT_SUCCESS);
  } else { // код родительского процесса
    wait(NULL); // ожидание завершения дочернего процесса
    printf("Parent process reading from shared memory...\n");
    printf("Message: %s\n", ptr); // чтение из разделяемой памяти
    shm_unlink(MEM_NAME); // удаление разделяемой памяти
    munmap(ptr, MEM_SIZE); // отмена отображения памяти
    close(fd); // закрытие дескриптора памяти
    exit(EXIT_SUCCESS);
  }
}
