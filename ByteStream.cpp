/*
    Byte Stream
    Luke Graham (39ster@gmail.com)
*/

#include <cstring>
#include "ByteStream.h"

ByteStream::ByteStream()
{
    buffer = 0;
    Clear(200);
}

ByteStream::ByteStream(const ByteStream& data)
{
    buffer = 0;
    Clear(data.Length());
    Write(data.Text(), data.Length());
}

ByteStream::ByteStream(const std::string& str)
{
    buffer = 0;
    Clear(str.length());
    Write(str.data(), str.length());
}

ByteStream::ByteStream(const char* str)
{
    size_t s = strlen(str);
    buffer = 0;
    Clear(s);
    Write(str, s);
}

ByteStream::ByteStream(const char* data, size_t dataSize)
{
    buffer = 0;
    Clear(dataSize);
    Write(data, dataSize);
}

ByteStream operator+(const ByteStream& stream1, const ByteStream& stream2)
{
    return ByteStream(stream1) << stream2;
}

ByteStream& ByteStream::operator=(const ByteStream& data)
{
    if(this != &data)
    {
        Clear(data.Length());
        Write(data.Text(), data.Length());
    }
    return *this;
}

ByteStream& ByteStream::operator+=(const ByteStream& data)
{
    return *this << data;
}

ByteStream& ByteStream::operator<<(const ByteStream& data)
{
    if(this == &data)
        return *this << ByteStream(data);

    Write(data.Text(), data.Length());
    return *this;
}

bool operator==(const ByteStream& stream1, const ByteStream& stream2)
{
    if(stream1.Length() == stream2.Length())
        return stream1.compare(stream2) == 0;
    return false;
}

bool operator!=(const ByteStream& stream1, const ByteStream& stream2)
{
    if(stream1.Length() == stream2.Length())
        return stream1.compare(stream2) != 0;
    return true;
}

bool operator<(const ByteStream& stream1, const ByteStream& stream2)
{
    return stream1.compare(stream2) < 0;
}

bool operator<=(const ByteStream& stream1, const ByteStream& stream2)
{
    return stream1.compare(stream2) <= 0;
}

bool operator>(const ByteStream& stream1, const ByteStream& stream2)
{
    return stream1.compare(stream2) > 0;
}

bool operator>=(const ByteStream& stream1, const ByteStream& stream2)
{
    return stream1.compare(stream2) >= 0;
}

void ByteStream::Clear(unsigned int size)
{
    free(buffer);
    count = readPos = writePos = 0;
    buffSize = (size > 0 ? size : 10);
    buffer = (char*)malloc(buffSize + 1);
    buffer[0] = 0;
}

unsigned int ByteStream::Write(const char* source, unsigned int length)
{
    if(writePos + length >= buffSize)
    {
        buffSize = writePos + length + 50 + (count / 2);
        buffer = (char*)realloc(buffer, buffSize + 1);
    }

    memcpy(&buffer[writePos], source, length);
    writePos += length;
    count = (writePos > count ? writePos : count);
    buffer[count] = 0;
    return length;
}

unsigned int ByteStream::Write(const ByteStream& b)
{
    return Write(&b[0], b.Length());
}

unsigned int ByteStream::Read(char* destination, unsigned int length)
{
    unsigned int len = (count - readPos < length ? count - readPos : length);
    if(len)
    {
        memcpy((void*)destination, (void*)&buffer[readPos], len);
        readPos += len;
    }
    return len;

}

ByteStream ByteStream::Read(unsigned int length)
{
    ByteStream retVal;
    unsigned int len = (count - readPos < length ? count - readPos : length);
    if(len)
    {
        retVal.Write(&buffer[readPos], len );
        readPos += len ;
    }
    return retVal;
}

std::string ByteStream::ReadString(unsigned int length)
{
    auto bytesLeft = this->Length() - TellRead();

    length = std::min(length, bytesLeft);

    if (length > 0)
    {
        std::string retval(&buffer[readPos], length);
        readPos += length;
        return retval;

    }
    return "";
    
}

std::string ByteStream::ReadLine()
{
    std::string retVal;
    while(readPos < count)
    {
        if(buffer[readPos] != '\n')
            retVal += buffer[readPos++];
        else {
            ++readPos;
            break;
        }
    }
    return retVal;
}

bool ByteStream::LoadFile(const std::string& fName)
{
    char buffer[0xFFFF];
    FILE* fHandle = fopen(fName.c_str(), "rb");
    if(fHandle)
    {
        int size = 0;
        Clear();
        while((size = fread(buffer, 1, sizeof(buffer), fHandle)) > 0)
            Write(buffer, size);

        fclose(fHandle);
        return true;
    }
    return false;
}

bool ByteStream::SaveFile(const std::string& fName) const
{
    FILE* fHandle = fopen(fName.c_str(), "wb");
    if(fHandle)
    {
        fwrite(buffer, 1, Length(), fHandle);
        fclose(fHandle);
        return true;
    }
    return false;
}
