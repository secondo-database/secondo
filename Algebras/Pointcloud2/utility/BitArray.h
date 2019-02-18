/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 BitArray class

Represents a memory-efficient array of n bits with the caller can get or
set randomly. Iterators allow to iterate either over all "true" bits, or
over a random selection of k "true" bits, providing their indices in ascending
order.

Example of usage:

----
// create BitArray for 1.000.000 entries (uses 1 bit of memory per entry);
BitArray ba = BitArray(1000000, false);
// as the second parameter, we used isOneBased = false, so entry positions
// will be interpreted as 0-based

// set all entries to "true"
ba.initializeAll(true);

// set an individual entry to "false"
ba.set(42, false);

// iterate over the indices of a random sample of 100000 "true" entries
// in ascending order
auto it = ba.getIterator(100000);
while (it->hasNext()) {
    size_t pointIndex = it->next();  // 0-based
}
delete it;
----

*/
#pragma once
#include <memory>

namespace pointcloud2 {

/*
The integer type used to store at least 32 bits for a BitArray.

*/
typedef uint32_t BitArrayT; // must not be uint64_t, since shifting with <<
                            // seems to be limited to 0..31

/*
The number of bits stored in a BitArrayT instance.

*/
constexpr size_t BITS_PER_ENTRY = 8 * sizeof(BitArrayT);

class BitArray {
/*
1.1 fields

*/
    /* the number of bits stored in this BitArray */
    const size_t _size;

    /* true if all index parameters and return values should be 1-based
     * (rather than 0-based which is used internally) */
    const bool _isOneBased;

    /* a value used to quickly get a random index in [0 .. size - 1] */
    const unsigned _randomIndexDenominator;

    /* the number of BitArrayT instances (i.e. integers) in _data */
    const size_t _entryCount;

    /* the bit data */
    const std::shared_ptr<BitArrayT> _data;

    bool _isInitialized;

    /* the number of "true" bits in this BitArray. This value is undefined
     * if !_isInitialized */
    size_t _trueCount;

public:
/*
1.2 constructor and destructor

1.2.1 constructor

Creates a new BitArray which can store (size) bits. If isOneBased
is true, every parameter value and return value will be 1-based
(rather than 0-based which is used internally).
Note that the BitArray contains random bits at first and needs
initialization with one of the initialize...() methods.

*/
    BitArray(const size_t size, const bool isOneBased);

/*
1.2.2 destructor

*/
    virtual ~BitArray() = default;

/*
1.3 public methods

1.3.1 clear

sets all bits in this BitArray to false (0)

*/
    void clear();

/*
1.3.2 initializeAll

sets all bits in this BitArray to the given value

*/
    void initializeAll(const bool value);

/*
1.3.3 initializeFirst

sets the first (count) bits in this BitArray to the given value
and all other bits to !value

*/
    void initializeFirst(const size_t count, const bool value);

/*
1.3.4 initializeRandom

sets (trueCount) randomly selected bits in this BitArray to true and
all other bits to false. This is done in O(n) or, for small trueCount
values in O(trueCount * trueCount)

*/
    void initializeRandom(const size_t trueCount);

/*
1.3.5 get

gets the bit at the given index

*/
    bool get(const size_t index) const;

/*
1.3.6 set

sets the bit at the given index

*/
    void set(const size_t index, const bool value);

/*
1.3.7 getSize

gets the total number of bits stored in this BitArray

*/
    size_t getSize() const;

/*
1.3.8 getTrueCount

returns the number of "true" bits (1-bits) in this BitArray in O(1)

*/
    size_t getTrueCount() const;

/*
1.3.9 toString

writes the first (maxCount) bits of the BitArray to a string
for testing purposes. Use maxCount = 0 to write all bits

*/
    std::string toString(const size_t maxCount) const;

/*
1.3.10 test

performs a test of BitArray, using the console as output. If
reproducible = true is set, the random number generator will
always be initialized with seed 0.

*/
    static void test(const bool reproducible);

/*
1.4 private methods

*/
private:
/*
1.4.1 getInternal

gets the bit at the given 0-based index

*/
    bool getInternal(const size_t index0) const;

/*
1.4.2 setInternal

sets the bit at the given 0-based index

*/
    void setInternal(const size_t index0, const bool value);

/*
1.4.3 mix

mixes all bits in the bit array in O(n)

*/
    void mix();

/*
1.4.4 getRandomIndex

returns a random value between 0 and \_size - 1

*/
    inline size_t getRandomIndex() const;

/*
1.4.5 reportTest

reports the given string and this-[>]toString() to the console

*/
    void reportTest(const std::string methodCalled) const;

/*
1.4.6 allocData

returns a pointer to an allocated space of (count) instances of the given
type T

*/
    template<typename T>
    static std::shared_ptr<T> allocData(size_t count) {
        std::allocator<T> allocator;
        std::shared_ptr<T> data(allocator.allocate(count), [count](T* p) {
            std::allocator<T> allocator;
            allocator.deallocate(p, count);
        });
        return data;
    }


public:

/*
2 BitArray::iterator

For a given BitArray, this iterator returns the index positions of all "true"
bits in ascending order. If constructed with a randomSelectionCount, only the
given number of randomly selected index positions will be returned (still in
ascending order).

*/
    class iterator {
/*
2.1 fields

*/
    private:
        /* the BitArray which is iterated over */
        const BitArray& _bitArray;

        /* true if the iteration has started, otherwise false */
        bool _started;

        /* the index of the current entry in _bitArray._data */
        size_t _index;

        /* the data entry at the current index */
        BitArrayT _entry;

        /* the bit index (0..63) which is set in _mask */
        unsigned _bitIndex;

        /* is set to 0 once the end has been reached */
        BitArrayT _mask;

        /* an inner BitArray providing the iterator with a random
         * selection. The size of _random equals the number of "true" bits
         * in the outer BitArray. */
        std::unique_ptr<BitArray> _random;

        /* an inner iterator over the _random BitArray */
        std::unique_ptr<iterator> _randomIter;

        /* the last index returned by _randomIter.getNext() */
        size_t _randomIndex;

/*
2.2 constructors and destructor

*/
    public:
/*
2.2.1 constructor for all "true" bits

creates an iterator which returns the index positions of
all the "true" bits in the BitArray in ascending order

*/
        iterator(const BitArray& bitArray);

/*
2.2.2 constructor for a random selection of the "true" bits

creates an iterator which returns the index position of
a random selection of (randomSelectionSize) "true" bits in the
BitArray in ascending order

*/
        iterator(const BitArray& bitArray, const size_t randomSelectionSize);

/*
2.2.3 destructor

*/
        ~iterator() = default;

/*
2.3 public methods

2.3.1 next

searches the next "true" bit in the BitArray and returns its
index position (0-based or 1-based, depending the constructor
parameter of BitArray), or returns -1 if no such bit is found.

*/
        int next();

/*
2.3.2 nextUncached

this variation of the next() function allows to change the bits
in the BitArray during iteration; when searching the next "true"
bit, the iterator will always consider the most recent state of
the bits.

*/
        int nextUncached();

/*
2.3.3 isRandomSelection

returns true if this iterator iterates over a random selection of the "true"
bits, or false if it iterates over all "true" bits.

*/
        bool isRandomSelection() const;

/*
2.4 private methods

*/
    private:
/*
2.4.1 initialize

initializes the iterator. If randomSelectionSize = \_bitArray.\_size,
the index positions of all "true" bits will be returned

*/
        void initialize(const size_t randomSelectionSize);

/*
2.4.2 findFirst

sets \_index, \_bitIndex and \_mask to the first "true" bit,
or sets \_mask = 0 if no such bit is found

*/
        void findFirst();

/*
2.4.3 findNextInRandomSelection

advances the iterator, possibly skipping several "true" bits
as required by the random selection

*/
        void findNextInRandomSelection();

/*
2.4.4 findNext

moves the iterator (\_index, \_bitIndex and \_mask) to the next
"true" bit in the BitArray, or sets \_mask = 0 if no such bit is
found

*/
        void findNext();

/*
2.4.5 currentPosition

returns the current index position of the simulated bit array

*/
        inline size_t currentPosition() const;
    };

/*
2.5 BitArray getters for the iterator

*/
public:
/*
2.5.1 getIterator

creates an iterator which returns the index positions of
all the "true" bits in this BitArray in ascending order.

*/
    std::unique_ptr<BitArray::iterator> getIterator() const;

/*
2.5.2 getIterator for a random selection

creates an iterator which returns the index position of
a random selection of (randomSelectionSize) "true" bits in the
this BitArray in ascending order.

*/
    std::unique_ptr<BitArray::iterator> getIterator(
            size_t randomSelectionSize) const;

    friend class iterator;
};


} /* namespace pointcloud2 */
