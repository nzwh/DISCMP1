# Prime Finder

Four different approaches to finding prime numbers using multithreading in C++.

## What's Here

- **Variant 1**: Prints primes immediately as they're found
- **Variant 2**: Collects all results first, then prints everything at the end
- **Variant 3**: Each thread buffers its own output, prints when done
- **Variant 4**: Linear search, but uses threads to test divisibility of each number

## Building

```bash
clang++ -std=c++11 -pthread -o prime_v1 v1.cpp
clang++ -std=c++11 -pthread -o prime_v2 v2.cpp
clang++ -std=c++11 -pthread -o prime_v3 v3.cpp
clang++ -std=c++11 -pthread -o prime_v4 v4.cpp
```

Or use `g++` instead of `clang++` if you prefer.

## Running

Create a `config.txt` file one directory up from where you run the programs:

```
threads=4
limit=256
```

Then run any variant:

```bash
./prime_v1
./prime_v2
./prime_v3
./prime_v4
```

## Performance Notes

Variant 4 is significantly slower than the others because it spawns threads for every single number it checks. The first three variants split the search range among threads, which is much more efficient.

Try different limits to see how they compare:
- `limit=256`
- `limit=65536`