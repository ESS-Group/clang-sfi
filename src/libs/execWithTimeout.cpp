/*
    Source: https://gist.github.com/fferri/0a3138d15d96744d3680
*/
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct execTimeoutReturn{
    bool timeout;
    int exitCode;
    int errnumber;
};
execTimeoutReturn exec_with_timeout(char * const *argv, int timeout, int kill_signal = SIGKILL)//changed
{
    execTimeoutReturn *ret = (execTimeoutReturn *)mmap(NULL,sizeof(execTimeoutReturn), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    ret->exitCode = 0;
    ret->timeout = false;
    ret->errnumber = 0;
    pid_t intermediate_pid = fork();
    if(intermediate_pid == 0) {
        pid_t worker_pid = fork();
        if(worker_pid == 0) {
            int exitCode = execv(argv[0], argv);//changed
            ret->errnumber = errno;
            _exit(127);
        }

        pid_t timeout_pid = fork();
        if(timeout_pid == 0) {
            usleep(timeout*1000);//changed
            ret->timeout = true;//chagned
            ret->exitCode = 1337;//changed
            msync(ret,sizeof(execTimeoutReturn), MS_SYNC|MS_INVALIDATE);
            _exit(0);
        }
        
        
        int status = 0;//changed
        pid_t exited_pid = waitpid(-1,&status,0);//changed
        if(exited_pid == worker_pid) {
            ret->timeout = false;//chagned
            ret->exitCode = status;//changed
            msync(ret,sizeof(execTimeoutReturn), MS_SYNC|MS_INVALIDATE);//changed
            kill(timeout_pid, SIGKILL);
        } else {
            kill(worker_pid, kill_signal);
        }
        wait(NULL); // Collect the other process //changed
        _exit(status); // Or some more informative status //changed
    }
    int exitCode = 0;//changed
    waitpid(intermediate_pid, &exitCode, 0);//changed


    execTimeoutReturn  _ret;
    _ret.exitCode = ret->exitCode;
    _ret.timeout = ret->timeout;
    _ret.errnumber = ret->errnumber;
    munmap(ret, sizeof(execTimeoutReturn));

    return _ret;//changed
}
