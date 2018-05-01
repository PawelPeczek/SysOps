# C system programming
## Synchronization - SLEEPING BARBER PROBLEM
* using SVR4 semaphores and shared memory
* using POSIX semaphores and shared memory

## ISSUES
I cannot handle the problem with giving priority to checking Waiting Room. When clients and barber are waiting on Waiting Room semaphore there could be a situation (especially in high concurrency environment) when barber process is being signalled after (possibly many) clients processes having been signalled. I tried to solve this problem in SVR4 (ex 1), but this only moved the issue in another place. I was wandering about implementing queue somehow at the entrance to Waiting Room but this probably also require to synchronize the queue and the problem is still not solved. 
If you know the proper solution, please contact me.

