#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`
// 可变参数

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capcity(capacity){};

size_t ByteStream::write(const string &data) {
    // 往 buff里面写入data字节
    size_t input = 0;
    // 还要知道还能够写多少字节
    size_t remain=remaining_capacity();
    for (char s : data) {
        if (input >=remain)
            break;
        buff.push_back(s);
        input++;
        // 如果剩余空间不够
    }
    wcnt += input;

    return input;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    
    string ret;
    size_t cnt = 0;

    for (auto s : buff) {
        cnt++;
        ret += s;
        if (cnt == len)
            break;
    }

    return ret;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t cnt = 0;
    size_t size=buff.size();
    while (cnt != len&&cnt!=size) {
        cnt++;
        buff.erase(buff.begin());  // 头删
    }
    rcnt += cnt;

}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string ret = peek_output(len);
    pop_output(len);
    return ret;
}

// 写入达到了上限
void ByteStream::end_input() { _end = true; }

bool ByteStream::input_ended() const { return _end; }

size_t ByteStream::buffer_size() const { return buff.size(); }

bool ByteStream::buffer_empty() const { return buff.empty(); }

bool ByteStream::eof() const { return buffer_empty()&&input_ended(); }

size_t ByteStream::bytes_written() const { return wcnt; }

size_t ByteStream::bytes_read() const { return rcnt; }

size_t ByteStream::remaining_capacity() const { return _capcity-buff.size(); }
