#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

int _add(int num1, int num2)
{
    return num1 + num2;
}

int _sub(int num1, int num2)
{
    return num1 - num2;
}

int _mul(int num1, int num2)
{
    return num1 * num2;
}

int _div(int num1, int num2)
{
    if (num2 != 0)
        return num1 / num2;
    else
        throw std::runtime_error("Division by zero");
}

int main()
{
    const char* semName = "/rpc_sem";
    sem_t* sem = sem_open(semName, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED)
    {
        std::cerr << "sem_open error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    const char* filename = "/rpc_shm";
    int fd = shm_open(filename, O_RDWR | O_CREAT, 0666);
    if (fd == -1)
    {
        std::cerr << "shm_open error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    int shm_size = 4 * sizeof(int);
    if (ftruncate(fd, shm_size) == -1)
    {
        std::cerr << "ftruncate error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    int* adr = static_cast<int*>(mmap(nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (adr == MAP_FAILED)
    {
        std::cerr << "mmap error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    close(fd);

    while (true)
    {
        sem_wait(sem);
        try
        {
            switch (adr[0])
            {
                case 0:
                    adr[3] = _add(adr[1], adr[2]);
                    break;
                case 1:
                    adr[3] = _sub(adr[1], adr[2]);
                    break;
                case 2:
                    adr[3] = _mul(adr[1], adr[2]);
                    break;
                case 3:
                    adr[3] = _div(adr[1], adr[2]);
                    break;
                default:
                    throw std::invalid_argument("Invalid operation code");
            }
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Error: " << ex.what() << std::endl;
            exit(EXIT_FAILURE);
        }

        sem_post(sem);
    }

    munmap(adr, shm_size);
    shm_unlink(filename);
    sem_close(sem);
    sem_unlink(semName);

    return 0;
}
