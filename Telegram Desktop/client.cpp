#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>
#include <unistd.h>
#include <cstring>

int main(){
    const char* shm_name = "/rpc_shm";
    int shm_size = 4 * sizeof(int);

    const char* semName = "/rpc_sem";
    sem_t* sem = sem_open(semName, O_CREAT, 0666, 0);

    int fd = shm_open(shm_name, O_RDWR, 0);

    if(fd == -1){
        std::cerr << "Shared memory cannot be opened: " << std::strerror(errno) << std::endl;
        exit(errno);
    }

    int* adr = (int*) mmap(nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (adr == MAP_FAILED)
    {
        std::cerr << "MMAP FAILED: " << std::strerror(errno) << std::endl;
        exit(errno);
    }

    close(fd);

    int functionID;
    std::cout << "Please enter the function identifier (0: add, 1: sub, 2: mul, 3: div): ";
    std::cin >> functionID;

    int a, b;
    std::cout << "Please enter the arguments: ";
    std::cin >> a >> b;

    adr[0] = functionID;
    adr[1] = a;
    adr[2] = b;

    sem_post(sem);

    sem_wait(sem);
    int res = adr[3];

    std::cout << "The result is: " << res << std::endl;

    munmap(adr, shm_size);
    sem_close(sem);
    sem_unlink(semName);
    shm_unlink(shm_name);

    return 0;
}

