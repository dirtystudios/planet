//
//  ByteStream.cpp
//  dirtycommon
//
//  Created by Eugene Sturm on 1/29/19.
//

#include "ByteStream.hpp"

ByteStream::ByteStream()
{
    _data.reserve(8);
}

ByteStream::~ByteStream()
{}


const ByteStream& ByteStream::operator>>(std::string& rhs) const
{
    rhs.clear();
    while (_rpos < _wpos) {
        char c = read<char>();
        if (c == 0) {
            break;
        }
        rhs += c;
    }
    return *this;
}

ByteStream& ByteStream::operator<<(const std::string& rhs)
{
    write(rhs.data(), rhs.length());
    (*this) << static_cast<char>(0);
    return *this;
}

void ByteStream::read(void* data, size_t len) const {
    assert(_rpos + len <= _wpos);
    assert(data);
    memcpy(data, _data.data() + _rpos, len);
    _rpos += len;
}

void ByteStream::write(const void* data, size_t size)
{
    assert(data);
    _data.resize(_wpos + size);
    memcpy(_data.data() + _wpos, data, size);
    _wpos += size;
}

void ByteStream::reset()
{
    _wpos = 0;
    _rpos = 0;
}

const uint8_t* ByteStream::data() const { return _data.data(); }
uint8_t* ByteStream::data() { return _data.data(); };
size_t ByteStream::readPos() const { return _rpos; }
size_t ByteStream::writePos() const { return _wpos; }
size_t ByteStream::size() const { return _data.size(); }
size_t ByteStream::capacity() const { return _data.capacity(); }

void ByteStream::grow()
{
}

