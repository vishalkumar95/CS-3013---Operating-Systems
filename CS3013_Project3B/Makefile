all: planes airplanes airplanes_semaphores airplanes_mutexes 

planes:
	gcc -g -o planes planes.c -pthread

airplanes:
	gcc -g -o airplanes airplanes.c -pthread

airplanes_semaphores:
	gcc -g -o airplanes_semaphores airplanes_semaphores.c -pthread

airplanes_mutexes:
	gcc -g -o airplanes_mutexes airplanes_mutexes.c -pthread

clean:
	rm -rf *.o planes airplanes airplanes_semaphores airplanes_mutexes
