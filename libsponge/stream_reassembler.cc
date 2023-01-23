#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

// template <typename... Targs>
// void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.

void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // DUMMY_CODE(data, index, eof);
    string cp(data);
    size_t _index = func(index, cp, false, eof);  // 先把新加进来的数据加进来,_index是已经组装的最新编号

    if (!unpushstr.empty()) {
        // 有未push的
        map<size_t, string> copy(unpushstr);
        for (auto &pair : copy) {
            if (pair.first > _index)
                break;
            _index = func(pair.first, pair.second, true, eof);
        }
    }

    // 未push的弄完了，这里弄新弄进来的
    if (_eof && empty()) {
        _output.end_input();
    }
}

size_t StreamReassembler::func(const size_t index,
                               string data,
                               bool isunpush,
                               const bool eof)  // 返回此时已经成功写入的位置
{
    if (eof) {
        _eof = true;
    }
    size_t now = _output.bytes_written();  // 统计写入了多少个字节了,now就是编号
    // 但是这个时候容器还可以添加数据

    if (index == now) {
        // 当前的坐标刚刚好到达了正确的位置
        if (_output.buffer_size() + data.size() <= _capacity) {
            _output.write(data);
            if (isunpush) {
                unpushstr.erase(index);
            }
            return index + data.size();

        } else {
            if (_output.buffer_size() <= _capacity) {
                string sub = data.substr(0, _capacity);
                if (isunpush) {
                    unpushstr.erase(index);
                }
                
                //同时还要保存起来后来的数据
                _output.write(sub);
                //这个地方添加多出来的数据
                
                return now+sub.size();
            }
            if (!isunpush)
                unpushstr[index] = data;
            return now;
        }

    } else if (index < now) {
        // 说明发送的字节发生了重叠
        if (index + data.size() <= now) {
            // 已经发送的字符串的子串
            // 可以被丢弃了
            if (isunpush) {
                unpushstr.erase(index);
            }
            return now;

        } else {
            // 说明前面重叠，但是，后面可以新加进去
            string sub = data.substr(now - index);  // 截取了
            if (_output.buffer_size() + sub.size() <= _capacity) {
                // 有重叠，并处理后写入
                if (isunpush) {
                    unpushstr.erase(index);
                }
                _output.write(sub);

                return index + data.size();

            } else {
                if (_output.buffer_size() <= _capacity) {
                    string sb = sub.substr(0, _capacity - _output.buffer_size());
                    _output.write(sb);
                    return _capacity;
                }

                // 有重叠，但是容量不够,放入未进入容器中
                if (isunpush) {
                    unpushstr.erase(index);
                }
                unpushstr[now - index] = sub;
                return now;
            }
        }
    } else {
        // 说明这个坐标在后面
        if (!isunpush) {
            unpushstr[index] = data;
        }
        return now;
    }
}
size_t StreamReassembler::unassembled_bytes() const {
    set<int> cnt;
    for (auto &pair : unpushstr) {
        size_t left = pair.first;
        size_t right = left + pair.second.size() - 1;
        for (size_t i = left; i <= right; i++) {
            cnt.insert(i);
        }
    }
    return cnt.size();
}

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
