#ifndef BUFFERH
#define BUFFERH
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>


class ByteStream
{
private:
    char* buffer;
    unsigned int buffSize, count;
    unsigned int readPos, writePos;

public:
    enum Origin {
        ORIGIN_SET,
        ORIGIN_CUR,
        ORIGIN_END
    };

//Constructors
    ByteStream();
    ByteStream(const ByteStream& data);
    ByteStream(const std::string& str);
    ByteStream(const char* str);
    ByteStream(const char* data, size_t dataSize);

//Template constructor
    template <class T> ByteStream(const T& data);

//Destructors
    ~ByteStream() {
        free(buffer);
    }

//ByteStream Operators
    ByteStream& operator=(const ByteStream& data);
    ByteStream& operator+=(const ByteStream& data);
    ByteStream& operator<<(const ByteStream& data);

    const char& operator[](int index) const {
        return buffer[index];
    }
    char& operator[](int index) {
        return buffer[index];
    }

//Template operators
    template <class T> ByteStream& operator>>(T& data);

//Functions

    std::string ToString() const {
        return std::string(buffer);
    } 

    char* Text() {
        return buffer;
    }

    const char* Text() const {
        return buffer;
    }

    unsigned int Length() const {
        return count;
    }

    void RewindRead() {
        readPos = 0U;
    }

    void SeekRead(long pos, Origin origin)
    {
        switch (origin)
        {
        case Origin::ORIGIN_CUR:
            readPos += pos;
            break;

        case Origin::ORIGIN_SET:
            readPos = pos;
            break;


        case Origin::ORIGIN_END:
            readPos = count - pos - 1;
            break;

        }

    }
    int compare(const ByteStream& stream) const {
        unsigned int len = Length() > stream.Length() ? stream.Length() : Length();
        return memcmp(Text(), stream.Text(), len);
    }

    unsigned int Write(const char* source, unsigned int length);
    unsigned int Write(const ByteStream& b);
    unsigned int Read(char* destination, unsigned int length);
    ByteStream Read(unsigned int length);
    std::string ReadString(unsigned int length);
    std::string ReadLine();
    std::string ReadString() {
        return ReadString(Length() - readPos);
    }
    void Clear(unsigned int size = 10);
    bool LoadFile(const std::string& fName);
    bool SaveFile(const std::string& fName) const;

    unsigned int BytesLeft() const {
        return count - readPos;
    }

    unsigned int TellRead() const {
        return readPos;
    }

    unsigned int TellWrite() const {
        return writePos;
    }
//Template functions
    template <class T> unsigned int Write(const T& source);
    template <class T> T Read();

};

template <class T>
ByteStream::ByteStream(const T& data)
{
    buffer = 0;
    Clear(sizeof(T));
    Write((char*)&data, sizeof(T));
}

template <class T>
ByteStream& ByteStream::operator>>(T& data)
{
    Read((char*)&data, sizeof(data));
    return *this;
}

template <class T>
unsigned int ByteStream::Write(const T& source)
{
    return Write((char*)&source, sizeof(T));
}

template <class T>
T ByteStream::Read()
{
    T retVal;
    Read((char*)&retVal, sizeof(T));
    return retVal;
}

bool operator==(const ByteStream& stream1, const ByteStream& stream2);
bool operator!=(const ByteStream& stream1, const ByteStream& stream2);
bool operator>(const ByteStream& stream1, const ByteStream& stream2);
bool operator>=(const ByteStream& stream1, const ByteStream& stream2);
bool operator<(const ByteStream& stream1, const ByteStream& stream2);
bool operator<=(const ByteStream& stream1, const ByteStream& stream2);
ByteStream operator+(const ByteStream& stream1, const ByteStream& stream2);
#endif // BUFFERH

