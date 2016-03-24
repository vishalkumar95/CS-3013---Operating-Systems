// Solving the aircraft synchronization problem through semaphores and mutexes

// Vishal Rathi and Kevin Martin


For this problem, we have 3 runways and 25 airplanes. These airplanes arrive randonmly in the airspace. 25 threads are created and put to sleep for a random time. When the thread wakes up, it means that the corresponding plane has entered the airspace. The plan requests to land once it arrives but, we only have 3 runways. The depletion rate is fixed for all of the planes and the landing time varies for each plane within a fixed range. The normal plane landing is done based off on the amount of fuel left. Plane with the lowest fuel at a given point is landed first. In the case of the emergency situation, all the planes are blocked and it is given the top priority. Once, the emergency plane is landed then, the normal priority process is continued.


ASSUMPTIONS:

1) If two emergency planes arrive at the same time, one which arrived last is given the priority.
2) The priorty is done based off on the amount of fuel left when the plane enters the airspace.
3) For the fuel and landing/clearing time, we used the rand function to randomize the numbers.

For the semaphore based approach, we have created three binary semaphores for the three runways. These semaphores can be used to check if a particular runway is blocked or open. To up and down the semaphores, we use the functions sem_wait() and sem_post. sem_wait() function is used to wait until the value of the semaphore is greater than 0. It essentially does the down operation and decrements the value of the sempahore by 1. sem_post() function does the up operation and increments the valaue of the sempahore by 1. If there are one or more threads waiting, it wakes up one thread. These functions are used to control the operations of the runway. At the end, the sempahores are destroyed.

For the mutex based approach, we have created three mutex locks, one for each runways. This way the other runways could still be in use while a plane is landing on one of them. For the mutex, we use the mutex_lock and mutex_unlock methods to lock or unlock a particular mutex. When a plane is checking if it can land, it checks to see if there are any emergency planes in the airspace. In the current system, an emergency plane is given priority above planes with low fuel so an emergency plane can cause the other plane to crash.


