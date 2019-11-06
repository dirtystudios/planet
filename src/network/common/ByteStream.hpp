//
//  ByteStream.hpp
//  dirtycommon
//
//  Created by Eugene Sturm on 1/29/19.
//

#pragma once

#include <stdint.h>
#include <string>
#include <vector>

class ByteStream {
public:
private:    
    std::vector<uint8_t> _data;
    size_t _wpos{0};
    mutable size_t _rpos{0};
    
public:
    ByteStream();
    ~ByteStream();
    
    template <class T>
    ByteStream& operator<<(const T& rhs);
    ByteStream& operator<<(const std::string& rhs);
    
    template <class T>
    ByteStream& operator<<(const std::vector<T>& rhs);
    
    template <class T>
    const ByteStream& operator>>(T& rhs) const;
    const ByteStream& operator>>(std::string& rhs) const;
    
    template <class T>
    const ByteStream& operator>>(std::vector<T>& rhs) const;
    
    void read(void* data, size_t len) const;
    void write(const void* data, size_t size);
    
    template <class T>
    T read() const;
    
    void reset();
    
    uint8_t* data();
    const uint8_t* data() const;
    size_t size() const;
    size_t readPos() const;
    size_t writePos() const;    
    size_t capacity() const;
private:
    void grow();
};


template <class T>
ByteStream& ByteStream::operator<<(const T& rhs)
{
    write(&rhs, sizeof(T));
    return *this;
}

template <class T>
ByteStream& ByteStream::operator<<(const std::vector<T>& rhs)
{
    size_t size = rhs.size();
    write(&size, sizeof(size_t));
    write(rhs.data(), sizeof(T));
    return *this;
}

template <class T>
const ByteStream& ByteStream::operator>>(T& rhs) const
{
    rhs = read<T>();
    return *this;
}

template <class T>
const ByteStream& ByteStream::operator>>(std::vector<T>& rhs) const
{
    size_t size = read<size_t>();
    rhs.resize(size);
    read(rhs.data(), size * sizeof(T));
    return *this;
}

template <class T>
T ByteStream::read() const
{
    T val;
    read(&val, sizeof(T));
    return val;
}
