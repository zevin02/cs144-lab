#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    // 传过来的syn的序列号就是isn
    size_t diff = 0;
    if (seg.header().syn) {
        isn = seg.header().seqno;
        diff++;
    }
    if (isn && seg.header().fin) {  // 在建立连接之前的fin，发过来才有用
        fin = true;
        diff++;
    }
    if (isn) {
        // 设置了syn才能够发送数据
        // size_t payloadsize = seg.length_in_sequence_space() - diff;  // 这个是真正的有效载荷

        uint64_t seqno = unwrap(seg.header().seqno, *isn, checkpoint);  // 先将发送过来的序列号转化成绝对序列号
        uint64_t index = seg.header().syn ? 0 : seqno - 1;  // 如果设置了syn，那么就从0开始，否则就是正常的，-1即可
        checkpoint = seqno;

        // if (index + payloadsize <= _capacity) {
        // 负载的大小必须在窗口范围之内

        // 如果有syn，这个就-1,如果没有就正常
        // 获得段的长度

        _reassembler.push_substring(seg.payload().copy(), index, fin);
        // }
    }
    // isn还没有被设置，没有建立连接，发过来的数据都不需要
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!isn)  // 没有发送
    {
        return nullopt;
    } else {
        // 设置ack的值
        int op = 0;
        // if(isn)
        // op++;
        if (fin)
            op++;
        // 因为fin也是一个字节，所以ack确认也需要加上这个
        if(unassembled_bytes()==0)
        return wrap(static_cast<uint64_t>(stream_out().bytes_written()) + 1, *isn) + op;  // 加1是为了加上syn
        else
        return wrap(static_cast<uint64_t>(stream_out().bytes_written()) + 1, *isn);
    }
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
