#pragma once

#include <stdint.h>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>


class ByteBufferReader {
private:
    const size_t _len;
    size_t _readPos{0};
    const uint8_t* _data;
public:
    ByteBufferReader(const uint8_t* data, size_t len) : _len(len), _readPos(0), _data(data) {
        assert(_len > 0 && _data);
    }
    
    void Read(void* buffer, size_t bytes) {
        assert(_readPos + bytes <= _len);
        memcpy(buffer, _data + _readPos, bytes);
        _readPos += bytes;
    }
    
    size_t ReadPos() {
        return _readPos;
    }
    
    void Seek(size_t position) {
        assert(position <= _len && position >= 0);
        _readPos = position;
    }
    
    void Skip(size_t bytesToSkip) {
        assert(_readPos + bytesToSkip >= 0 && _readPos + bytesToSkip <= _len);
        _readPos += bytesToSkip;
    }
    
    void Reset() {
        Seek(0);
    }
    
    size_t Size() {
        return _len;
    }
};

class ByteBufferWriter {
private:
    uint8_t* _data;
    const size_t _len;
    size_t _writePos{0};
public:
    ByteBufferWriter(uint8_t* data, size_t len) : _data(data), _len(len), _writePos(0) {
        assert(_len > 0 && _data);
    }
    
    void Write(const void* buffer, size_t bytes) {
        assert(_writePos + bytes <= _len);
        memcpy(_data + _writePos, buffer, bytes);
        _writePos += bytes;
    }
    
    size_t WritePos() {
        return _writePos;
    }
    
    void Skip(size_t bytesToSkip) {
        assert(_writePos + bytesToSkip >= 0 && _writePos + bytesToSkip <= _len);
        _writePos += bytesToSkip;
    }
    
    void Seek(size_t position) {
        assert(position <= _len && position >= 0);
        _writePos = position;
    }
    
    void Reset() {
        Seek(0);
    }
    
    size_t Size() {
        return _len;
    }
};

class ByteBuffer {
public:
private:
    static constexpr size_t kDefaultBufferSize = 512;

    bool _owned{false};
    uint8_t* _data{nullptr};
    size_t _capacity{0};

    size_t _wpos{0};
    size_t _rpos{0};

public:
    ByteBuffer(size_t len = kDefaultBufferSize) : ByteBuffer((len > 0 ? new uint8_t[len] : nullptr), std::max<size_t>(0, len)) { _owned = true; }

    ByteBuffer(uint8_t* buffer, size_t len) : _data(buffer), _capacity(len), _wpos(0), _rpos(0) {
        assert(_data);
        assert(_capacity);
    }

    ~ByteBuffer() {
        if (_owned && _data) {
            delete[] _data;
            _data = nullptr;
        }
    }

    void Resize(size_t bytes) {
        assert(_owned);
        assert(bytes + 1 > _wpos);
        assert(bytes >= _rpos);
        assert(bytes > _capacity);

        uint8_t* newData = new uint8_t[bytes];
        memcpy(newData, _data, _capacity);
        delete[] _data;
        _data     = newData;
        _capacity = bytes;
    }

    template <class T> ByteBuffer& operator<<(const T& rhs) {
        Write(&rhs, sizeof(T));
        return *this;
    }

    template <class T> ByteBuffer& operator>>(T& rhs) {
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
        assert(data);
        memcpy(data, _data + _rpos, len);
        _rpos += len;
    }

    void Write(const void* data, size_t size) {
        assert(_wpos + size <= _capacity);
        assert(data);
        memcpy(_data + _wpos, data, size);
        _wpos += size;
    }

    template <class T> T Read() {
        T val;
        Read(sizeof(T), reinterpret_cast<uint8_t*>(&val));
        return val;
    }

    void Reset() {
        _wpos = 0;
        _rpos = 0;
    }
    
    uint8_t* GetDataPtr() { return _data; };

    size_t ReadPos() { return _rpos; }

    size_t WritePos() { return _wpos; }

    // negative bytes should be supported
    void WriteSkip(size_t bytes) {
        assert(_wpos + bytes < _capacity && _wpos + bytes >= _rpos);
        _wpos += bytes;
    }

    size_t Capacity() { return _capacity; }

    bool IsWithinRange(uint8_t* ptr) { return ptr >= _data && ptr < _data + _capacity; }
};
