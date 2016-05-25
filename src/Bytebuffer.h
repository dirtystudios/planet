#pragma once 

#include <stdint.h>
#include <cassert>
#include <string>
#include <vector>


class ByteBuffer {
private:
    static constexpr size_t kDefaultBufferSize = 256;
    
    std::vector<uint8_t> _buffer;
    size_t _wpos{0};
    size_t _rpos{0};
public:
    ByteBuffer(size_t len = kDefaultBufferSize) : _wpos(0), _rpos(0) {
        Resize(len);
    }
    
    void Resize(size_t size) {
        _buffer.resize(size);
    }
    
    template <class T>
    ByteBuffer& operator<<(const T& rhs) {
        Write(&rhs);
        return *this;
    }
    
    template <class T>
    ByteBuffer& operator>>(T& rhs) {
        rhs = Read<T>();
        return *this;
    }
    
    ByteBuffer& operator>>(std::string& rhs) {
        rhs.clear();
        while (_rpos < _wpos) {
            char c = Read<char>();
            if (c == 0) {
                break;
            }
            rhs += c;
        }
        return *this;
    }
    
    ByteBuffer& operator<<(const std::string& rhs) {
        Write(rhs.data(), rhs.length());
        (*this) << static_cast<char>(0);
        return *this;
    }
    
    void Read(size_t len, uint8_t* data) {
        assert(_rpos + len <= _wpos);
        memcpy(data, _buffer.data() + _rpos, len);
        _rpos += len;
    }
    
    template <class T>
    void Write(const T* data, uint32_t count = 1) {
        size_t len = sizeof(T) * count;
        assert(_wpos + len < _buffer.capacity());
        memcpy(_buffer.data() + _wpos, data, len);
        _wpos += len;
    }
    
    template <class T>
    T Read() {
        T val;
        Read(sizeof(T), reinterpret_cast<uint8_t*>(&val));
        return val;
    }
    
    void Reset() {
        _wpos = 0;
        _rpos = 0;
    }
    
    uint8_t* ReadPos() {
        return _buffer.data() + _rpos;
    }
    
    uint8_t* WritePos() {
        return _buffer.data() + _wpos;
    }
    
};