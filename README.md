# LRU_Cache
Implementation to perform cache replacement strategies, random and least recently used (LRU). The property of the cache can be modified with total capacity, associativity, block size, and replacement strategies. Testcases are composed of <a href="https://people.engr.tamu.edu/djimenez/classes/614/hw2/traces/index.html">SPEC 2006</a> to test the miss rate of the overal performance.


## Command
```cpp
➜ g++ -o cache cache.cpp (compile)
➜ gzip -dc -testcases_filename | ./cache -capacity -associativity -block_size -replacement_strategy

Example:
➜ gzip -dc testcases/429.mcf-184B.trace.txt.gz | ./cache 2048 64 64 l
total miss: 55752
total miss percentage: 5.5752%
read miss: 55703
read miss percentage: 5.61015%
write miss: 49
write miss percentage: 0.689752%
```
