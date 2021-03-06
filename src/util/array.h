#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <type_traits>
#include <algorithm>
#include <cstring>

/// very simple array class for IPC exchange

template <class T, int N>
class Array
{
//    static_assert(std::is_pod<T>::value, "Array<T>: T must be a POD type");
public:
    Array(int capacity =N) : mSize(0), mCapacity(capacity) {}
    
    int capacity() const { return mCapacity; }

    int size() const { return mSize; }
    bool empty() const { return mSize == 0; }
    bool full() const { return mSize == capacity(); }
    void clear() { mSize=0; }
    void push_back(const T &item)
    {
        assert(mSize < mCapacity);
        mData[mSize++] = item;
    }    
    void push_back_empty()
    {
        assert(mSize < mCapacity);
        ++mSize;
    }
    T &back() 
    { 
        assert(mSize > 0);
        return mData[mSize -1];
    }
    const T &back() const
    {
        assert(mSize > 0);
        return mData[mSize -1];
    }
    T pop_back()
    {
        assert(mSize>0);
        return mData[mSize--];
    }
    const T &operator[](int index) const
    {
        assert(index>=0 && index<mSize);
        return mData[index];
    }
    T &operator[](int index)
    {
        assert(index>=0 && index<mSize);
        return mData[index];
    }
    T *data() { return mData; }
    const T *data() const { return mData; }
    void set(const T *data, int size)
    {
        assert(size <= mCapacity);
        mSize = size;
        memcpy(mData, data, size * sizeof(T) );
    }
    void resize(int size)
    {
        assert(size <= mCapacity);
        mSize = size;
    }

protected:
    int mSize;
    int mCapacity;
    T mData[N];
};

////

template <class T>
class DynArray : public Array<T, 0>
{
public:
    DynArray(int capacity) : Array<T, 0>(capacity) {}
};

////

template <int N>
class String : public Array<char, N>
{
    typedef Array<char, N> Super;
public:
    static int capacity() { return N-1; }
    void set(const char *data, int size =-1)
    {
        if (size == -1) 
            size = std::min( capacity(), static_cast<int>(strlen(data)) );
        Super::mSize = size;
        memcpy(Super::mData, data, size);
        Super::mData[Super::mSize] = 0;
    }
    void clear()
    {
        Super::mData[0] = 0;
        Super::mSize = 0;
    }
    bool full() const { return Super::mSize == capacity(); }
};

#endif
