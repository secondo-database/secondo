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

*/

#include "BitArray.h"

#include <limits>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <iostream>
#include <sstream>

#include "assert.h"
#include "MathUtils.h"

namespace pointcloud2 {

/*
1.2 constructor and destructor

1.2.1 constructor

Creates a new BitArray which can store (size) bits. If isOneBased
is true, every parameter value and return value will be 1-based
(rather than 0-based which is used internally).
Note that the BitArray contains random bits at first and needs
initialization with one of the initialize...() methods.

*/
BitArray::BitArray(size_t size, bool isOneBased) :
    _size(size),
    _isOneBased(isOneBased),
    _randomIndexDenominator((_size == 0) ? 1 : (RAND_MAX + 1u) / _size),
    _entryCount((size + BITS_PER_ENTRY - 1) / BITS_PER_ENTRY),
    _data(allocData<BitArrayT>(_entryCount))
{
    _isInitialized = false;
    _trueCount = 0;

    // specifically clear last entry since the upper bits of it
    // are expected to be 0; every bit in the normal range will be
    // manipulated by clear() etc.
    if (_entryCount > 0) {
        *(_data.get() + (_entryCount - 1)) = 0;
    }
}

/*
1.3 public methods

1.3.1 clear

sets all bits in this BitArray to false (0)

*/
void BitArray::clear() {
    initializeFirst(_size, false);
}

/*
1.3.2 initializeAll

sets all bits in this BitArray to the given value

*/
void BitArray::initializeAll(bool value) {
    initializeFirst(_size, value);
}

/*
1.3.3 initializeFirst

sets the first (count) bits in this BitArray to the given value
and all other bits to !value

*/
void BitArray::initializeFirst(size_t count, bool value) {
    assert (count <= _size);

    // bulk set entries to value
    size_t j = count / BITS_PER_ENTRY;
    BitArrayT entry = value ? std::numeric_limits<BitArrayT>::max() : 0;
    for (size_t i = 0; i < j; ++i)
        *(_data.get() + i) = entry;

    // set the entry between value and !value
    size_t valueBitCount = count % BITS_PER_ENTRY;
    if (valueBitCount > 0) {
        BitArrayT* entryPtr = (_data.get() + j);
        BitArrayT mask = (1 << valueBitCount) - 1;
        *entryPtr = value ? mask : ~mask;
        ++j;
    }

    // bulk set entries to !value
    entry = ~entry;
    while (j < _entryCount) {
        *(_data.get() + j) = entry;
        ++j;
    }

    // clear last bits of last entry
    size_t keepBitCount = _size % BITS_PER_ENTRY;
    if (keepBitCount > 0) {
        BitArrayT* entryPtr = (_data.get() + _entryCount - 1);
        BitArrayT mask = (1 << keepBitCount) - 1;
        *entryPtr = (*entryPtr) & mask;
    }

    _trueCount = value ? count : _size - count;
    _isInitialized = true;
}


/*
1.3.4 initializeRandom

sets (trueCount) randomly selected bits in this BitArray to true and
all other bits to false. This is done in O(n) or, for small trueCount
values in O(trueCount * trueCount)

*/
void BitArray::initializeRandom(size_t trueCount) {
    // depending on the parameter trueCount, select the fastest strategy
    size_t falseCount = _size - trueCount;
    if (trueCount == 0) {
        initializeAll(false);
    } else if (trueCount == _size) {
        initializeAll(true);
    } else if ((double)trueCount * trueCount < _size) {
        initializeAll(false);
        while (_trueCount < trueCount)
            setInternal(getRandomIndex(), true);
    } else if ((double)falseCount * falseCount < _size) {
        initializeAll(true);
        while (_trueCount > trueCount)
            setInternal(getRandomIndex(), false);
    } else if (trueCount * 2 < _size) {
        initializeFirst(trueCount, true);
        mix();
    } else {
        initializeFirst(_size - trueCount, false);
        mix();
    }
    _trueCount = trueCount;
    _isInitialized = true;
}

/*
1.3.5 get

gets the bit at the given index

*/
bool BitArray::get(const size_t index) const {
    return getInternal(_isOneBased ? index - 1 : index);
}

/*
1.3.6 set

sets the bit at the given index

*/
void BitArray::set(size_t index, bool value) {
    setInternal(_isOneBased ? index - 1 : index, value);
}

/*
1.3.7 getSize

gets the total number of bits stored in this BitArray

*/
size_t BitArray::getSize() const {
    return _size;
}

/*
1.3.8 getTrueCount

returns the number of "true" bits (1-bits) in this BitArray in O(1)

*/
size_t BitArray::getTrueCount() const {
    return _trueCount;
}

/*
1.3.9 toString

writes the first (maxCount) bits of the BitArray to a string
for testing purposes. Use maxCount = 0 to write all bits

*/
std::string BitArray::toString(const size_t maxCount) const {
    std::stringstream out;
    size_t max = _size;
    if (max > maxCount && maxCount != 0)
        max = maxCount;
    for (size_t i = 0; i < max; ++i) {
        if (i % BITS_PER_ENTRY == 0)
            out << "  ";
        else if (i % 8 == 0)
            out << ".";
        out << (getInternal(i) ? "1" : "0");
    }
    return out.str();
}

/*
1.3.10 test

performs a test of BitArray, using the console as output. If
reproducible = true is set, the random number generator will
always be initialized with seed 0.

*/
void BitArray::test(bool reproducible) {
    using std::cout;
    using std::endl;

    std::srand(reproducible ? 0 : std::time(nullptr));

    cout << "Starting BitArray::test()" << endl;
    cout << "BITS_PER_ENTRY = " << BITS_PER_ENTRY << endl;

    BitArray ba = BitArray(150, false);
    ba.reportTest("create BitArray (bits are random)");
    cout << "_size = " << formatInt(ba._size) << endl;
    cout << "_entryCount = " << formatInt(ba._entryCount) << endl;

    ba.initializeAll(true);
    ba.reportTest("initializeAll(true)");

    ba.initializeAll(false);
    ba.reportTest("initializeAll(false)");

    cout << "initializeFirst(0..150, true)" << endl;
    for (int i = 0; i <= 150; ++i) {
        ba.initializeFirst(i, true);
        ba.reportTest("");
    }

    cout << "initializeFirst(0..150, false)" << endl;
    for (int i = 0; i <= 150; ++i) {
        ba.initializeFirst(i, false);
        ba.reportTest("");
    }

    ba.clear();
    ba.reportTest("clear()");

    ba.set(0, true);
    ba.set(9, true);
    ba.set(63, true);
    ba.set(64, true);
    ba.set(127, true);
    ba.set(149, true);
    ba.reportTest("set(0 / 9 / 63 / 64 / 127 / 149, true)");

    cout << "Iterator(): ";
    auto it = ba.getIterator();
    int index;
    while ((index = it->next()) >= 0) {
        cout << index << " ";
    }
    cout << endl;

    ba.set(9, false);
    ba.set(63, false);
    ba.set(149, false);
    ba.reportTest("set(9 / 63 / 149, false)");

    ba.clear();
    ba.reportTest("clear()");

    // test various methods in initializeRandom(size_t)
    size_t testTrueCounts[] { 0, 5, 10, 20, 30, 120, 130, 140, 145, 150, 21 };
    // 10 * 10 < 150; 30 * 30 > 150; (150 - 120) * (150 - 20) > 150;
    // (150 - 140) * (150 - 140) < 150
    for (size_t testTrueCount : testTrueCounts) {
        ba.initializeRandom(testTrueCount);
        std::stringstream st;
        st << "initializeRandom(" << testTrueCount << ")";
        ba.reportTest(st.str());

        cout << "Iterator(all): ";
        size_t count = 0;
        it = ba.getIterator();
        while ((index = it->next()) >= 0) {
            cout << index << " ";
            ++count;
        }
        cout << " (" << formatInt(count) << " values)" << endl;
    }

    // size_t rndSelCounts[] { 0, 1, 5, 8, 19, 20, 21 };
    for (size_t rndSelCount = 0; rndSelCount < 22; ++rndSelCount) {
        cout << "Iterator(random " << rndSelCount << "): ";
        it = ba.getIterator(rndSelCount);
        while ((index = it->next()) >= 0) {
            cout << index << " ";
        }
        cout << endl;
    }

    // test performance
    double clocksPerMSec = CLOCKS_PER_SEC / 1000.0;

    double tStart = clock();
    BitArray ba2 = BitArray(7000000, false);
    double tStop = clock();
    cout << "BitArray(" << formatInt(7000000) << ": " <<
            (tStop - tStart) / clocksPerMSec << " ms" << endl;

    tStart = clock();
    ba2.initializeAll(true);
    tStop = clock();
    cout << "initializeAll(true): " <<
            (tStop - tStart) / clocksPerMSec << " ms" << endl;

    tStart = clock();
    it = ba2.getIterator();
    size_t sum = 0;
    while ((index = it->next()) >= 0) {
        sum += index;
    }
    tStop = clock();
    cout << "iterate over true bits (checksum = " << sum << ":) " <<
            (tStop - tStart) / clocksPerMSec << " ms" << endl;

    tStart = clock();
    ba2.initializeFirst(100000, true);
    tStop = clock();
    cout << "initializeFirst(" << formatInt(100000) << ", true): " <<
            (tStop - tStart) / clocksPerMSec << " ms" << endl;

    tStart = clock();
    ba2.initializeRandom(100000);
    tStop = clock();
    cout << "initializeRandom(" << formatInt(100000) << "): " <<
            (tStop - tStart) / clocksPerMSec << " ms" << endl;

    tStart = clock();
    size_t trueCount = ba2.getTrueCount();
    tStop = clock();
    cout << "getTrueCount() (returns " << trueCount << "): " <<
            (tStop - tStart) / clocksPerMSec << " ms" << endl;

    tStart = clock();
    it = ba2.getIterator();
    sum = 0;
    while ((index = it->next()) >= 0) {
        sum += index;
    }
    tStop = clock();
    cout << "iterate over true bits (checksum = " << sum << ":) " <<
            (tStop - tStart) / clocksPerMSec << " ms" << endl;

    ba2.initializeRandom(6000000);
    tStart = clock();
    it = ba2.getIterator(100000);
    sum = 0;
    while ((index = it->next()) >= 0) {
        sum += index;
    }
    tStop = clock();
    cout << "iterate over a sample of " << formatInt(100000) <<
            " out of " << formatInt(6000000) << " true bits "
            << "(checksum = " << sum << ":) " <<
            (tStop - tStart) / clocksPerMSec << " ms" << endl;

    cout << "getIterator(random 100): ";
    it = ba2.getIterator(100);
    while ((index = it->next()) >= 0) {
        cout << index << " ";
    }
    cout << endl;
}

/*
1.4 private methods

1.4.1 getInternal

gets the bit at the given 0-based index

*/
bool BitArray::getInternal(const size_t index0) const {
    assert (index0 >= 0 && index0 < _size);

    size_t entryIndex = index0 / BITS_PER_ENTRY;
    BitArrayT mask = 1 << (index0 % BITS_PER_ENTRY);
    return _data.get()[entryIndex] & mask;
}

/*
1.4.2 setInternal

sets the bit at the given 0-based index

*/
void BitArray::setInternal(size_t index0, bool value) {
    assert (index0 >= 0 && index0 < _size);
    assert (_isInitialized); // otherwise _trueCount is undefined

    BitArrayT* entryPtr = _data.get() + (index0 / BITS_PER_ENTRY);
    BitArrayT entry = (*entryPtr);
    BitArrayT mask = 1 << (index0 % BITS_PER_ENTRY);
    bool oldValue = (entry & mask);
    if (oldValue == value) {
        // nothing to change
    } else if (value) {
        *entryPtr = entry | mask;
        ++_trueCount;
    } else {
        *entryPtr = entry & ~mask;
        --_trueCount;
    }
}

/*
1.4.3 mix

mixes all bits in the bit array in O(n)

*/
void BitArray::mix() {
    for (size_t i = 0; i < _size; ++i) {
        size_t j = getRandomIndex();
        bool value1 = getInternal(i);
        bool value2 = getInternal(j);
        if (value1 != value2) {
            setInternal(i, value2);
            setInternal(j, value1);
        }
    }
}

/*
1.4.4 getRandomIndex

returns a random value between 0 and \_size - 1

*/
size_t BitArray::getRandomIndex() const {
    size_t result;
    do {
        // rand() % _size is biased, therefore:
        result = std::rand() / _randomIndexDenominator;
    } while (result >= _size);
    return result;
}

/*
1.4.5 reportTest

reports the given string and this-[>]toString() to the console

*/
void BitArray::reportTest(const std::string methodCalled) const {
    if (methodCalled.length() > 0)
        std::cout << methodCalled << std::endl;
    std::cout << toString(0) << std::endl;
}

/*
2 BitArray::iterator

For a given BitArray, this iterator returns the index positions of all "true"
bits in ascending order. If constructed with a randomSelectionCount, only the
given number of randomly selected index positions will be returned (still in
ascending order).

2.2 constructors and destructor

2.2.1 constructor for all "true" bits

creates an iterator which returns the index positions of
all the "true" bits in the BitArray in ascending order

*/
BitArray::iterator::iterator(const BitArray& bitArray)
    : _bitArray(bitArray) {
    initialize(bitArray.getSize());
}

/*
2.2.2 constructor for a random selection of the "true" bits

creates an iterator which returns the index position of
a random selection of (randomSelectionSize) "true" bits in the
BitArray in ascending order

*/
BitArray::iterator::iterator(const BitArray& bitArray,
        const size_t randomSelectionSize)
    : _bitArray(bitArray) {
    initialize(randomSelectionSize);
}

/*
2.3 public methods

2.3.1 next

searches the next "true" bit in the BitArray and returns its
index position (0-based or 1-based, depending the constructor
parameter of BitArray), or returns -1 if no such bit is found.

*/
int BitArray::iterator::next() {
    // move to the next "true" bit in the BitArray
    if (!_started) {
        _started = true;
        findFirst();
    } else if (_mask == 0) {
        return -1;
    } else if (isRandomSelection()) {
        findNextInRandomSelection();
    } else {
        findNext();
    }

    // return the position of the "true" bit or -1 if no such bit was found
    if (_mask == 0) {
        return -1;
    } else {
        size_t result = currentPosition();
        return _bitArray._isOneBased ? result + 1 : result;
    }
}

/*
2.3.2 nextUncached

this variation of the next() function allows to change the bits
in the BitArray during iteration; when searching the next "true"
bit, the iterator will always consider the most recent state of
the bits.

*/
int BitArray::iterator::nextUncached() {
    // refresh _entry in case the bits stored it were changed in the
    // meantime
    if (_mask != 0)
        _entry = _bitArray._data.get()[_index];
    return next();
}

/*
2.3.3 isRandomSelection

returns true if this iterator iterates over a random selection of the "true"
bits, or false if it iterates over all "true" bits.

*/
bool BitArray::iterator::isRandomSelection() const {
    return (_randomIter.get());
}

/*
2.4 private methods

2.4.1 initialize

initializes the iterator. If randomSelectionSize = \_bitArray.\_size,
the index positions of all "true" bits will be returned

*/
void BitArray::iterator::initialize(const size_t randomSelectionSize) {
    _index = 0;
    _entry = 0;
    _bitIndex = 0;
    _mask = 0;

    // calculate the number of "true" bits only if there is a chance that
    // a random selection will be necessary
    size_t trueCount = (randomSelectionSize < _bitArray._size) ?
            _bitArray.getTrueCount() : 0;

    if (randomSelectionSize < trueCount) {
        // create an inner BitArray with as many bits as there are "true" bits
        // in the outer BitArray
        _random = std::unique_ptr<BitArray>(new BitArray(trueCount, false));

        // create a random selection of (randomSelectionSize) "true" bits
        // in the inner BitArray
        _random.get()->initializeRandom(randomSelectionSize);

        // get an iterator over the inner BitArray
        _randomIter = std::unique_ptr<BitArray::iterator>(
                _random.get()->getIterator());

        _randomIndex = 0;
    } else {
        _random = std::unique_ptr<BitArray>(nullptr);
        _randomIter = std::unique_ptr<BitArray::iterator>(nullptr);
        _randomIndex = 0;
    }
    _started = false; // the first "true" bit will be searched for only
                      // when hasNext() is called
}

/*
2.4.2 findFirst

sets \_index, \_bitIndex and \_mask to the first "true" bit,
or sets \_mask = 0 if no such bit is found

*/
void BitArray::iterator::findFirst() {
    _entry = _bitArray._data.get()[_index];
    if (_bitArray._size == 0) {
        _mask = 0;
        return;
    }
    // unless the very first bit is "true", find the first "true" bit
    _mask = 1;
    if (!(_entry & _mask))
        findNext();

    // in a random selection, it may be needed to search further "true" bits
    if (isRandomSelection()) {
        _randomIndex = 0;
        findNextInRandomSelection();
    }
}

/*
2.4.3 findNextInRandomSelection

advances the iterator, possibly skipping several "true" bits
as required by the random selection

*/
void BitArray::iterator::findNextInRandomSelection() {
    // has the random selection finished?
    int nextRandomIndex = _randomIter.get()->next();
    if (nextRandomIndex < 0) {
        _mask = 0;
        return;
    }

    // find the next "true" bit or skip several "true" bits
    size_t nextIndex = static_cast<size_t>(nextRandomIndex);
    while (_randomIndex < nextIndex && _mask != 0) {
        findNext();
        ++_randomIndex;
    }
}

/*
2.4.4 findNext

moves the iterator (\_index, \_bitIndex and \_mask) to the next
"true" bit in the BitArray, or sets \_mask = 0 if no such bit is
found

*/
void BitArray::iterator::findNext() {
    do {
        ++_bitIndex;
        _mask <<= 1;

        // test whether the next entry must be fetched from _data
        if (_bitIndex >= BITS_PER_ENTRY) {
            do {
                ++_index;
                if (_index >= _bitArray._entryCount) {
                    _mask = 0;
                    return;
                }
                _entry = _bitArray._data.get()[_index];
            } while (_entry == 0); // no bit set in _entry
            _bitIndex = 0;
            _mask = 1;
        }

        // if this bit is set, ...
        if (_entry & _mask) {
            // ... test whether we have exceeded the bit array size
            if (currentPosition() >= _bitArray._size) {
                assert (false); // must not happen since the remaining bits
                                // in the last entry are expected to be 0
                _mask = 0;
            }
            return;
        }
    } while (true);
}

/*
2.4.5 currentPosition

returns the current index position of the simulated bit array

*/
inline size_t BitArray::iterator::currentPosition() const {
    return _index * BITS_PER_ENTRY + _bitIndex;
}

/*
2.5 BitArray getters for the iterator

2.5.1 getIterator

creates an iterator which returns the index positions of
all the "true" bits in this BitArray in ascending order.

*/
std::unique_ptr<BitArray::iterator> BitArray::getIterator() const {
    return std::unique_ptr<BitArray::iterator>(new iterator(*this));
}

/*
2.5.2 getIterator for a random selection

creates an iterator which returns the index position of
a random selection of (randomSelectionSize) "true" bits in the
this BitArray in ascending order.

*/
std::unique_ptr<BitArray::iterator> BitArray::getIterator(
        size_t randomSelectionSize) const {
    return std::unique_ptr<BitArray::iterator>(
            new iterator(*this, randomSelectionSize));
}


} /* namespace pointcloud2 */
