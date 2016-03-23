// Vishal Rathi

I implemented the following page replacement algorithms:

1) Random 
2) LRU	(keeps track of history)
3) LIFO

The tests for all the three algorithms are in my test_methdology file. The outputs for my tests are stored in the output files. Here are the time statistics for my tests:

For random algorithm:

MemoryMaxer time:	26849 ms
testmethod time:	26846 ms
testmethod2 time:	48796 ms

For LRU algorithm:

MemoryMaxer time:	26834 ms
testmethod time:	26848 ms
testmethod2 time:	48801 ms

For LIFO algorithm:

MemoryMaxer time:	26856 ms
testmethod time:	26878 ms
testmethod2 time:	48812 ms

According to the time statistics, LRU seems to be the most efficient one which is expected but, it is pretty close to the random algorithm. LIFO seems to be the least efficient one which should not be the case since, random should be the least efficient one ideally. For this assignment, it should be acceptable because this is not what happens in the actual system. 

Let's discuss these statistics during the demo :)

I have also implemented the semaphores and multi-threaded program. I have four semaphores for each user API function. My multithread program makes sure that another thread is not accessing the same function and page_table entry when a particular thread is accessing it.


