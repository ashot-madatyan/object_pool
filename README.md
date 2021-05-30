# Description
Create a small C/C++ library that implements a generic object pool.
This is a small, header only library that implements the functionality of generic object pool.

## API's
The functionality of the library is exposed through its public API listed below
1. CTORs - default and initialization list
2. Add - adds an object to the pool, given its arguments (similar to CTOR)
3. Remove - removes the given object from the pool if the object was acquired through the pool.
   Otherwise the 'Remove' call is ignored.
4. Get - gets a unique pointer wrapped object from the pool.

The libary is thread safe and can be used in concurrent 

## Requirements

Using this library, we are able to:
1. Add healthy objects into the pool.
2. Remove objects from the pool (either on demand, of if unhealthy).
3. Acquire 1 object from the pool to be used.
4. Send the used object back to the pool to be healthchecked before being added back into it.
5. See the size of the pool at any given time.
6. Be thread safe and fit for concurrent use. 

## Usage
For sample usage, please see the file src/app_main.cpp
