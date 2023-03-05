#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)

Timer::Timer(unsigned int rto) : retransmission_timeout(rto), _rto(rto) {}  // 构造函数最好按照顺序进行初始化
void Timer::doubletime() {
    if (isopen()) {
        retransmission_timeout *= 2;
        consecutive_times++;  // 因为过了一段时间了所以时间需要添加了
        times = 0;            // 进行再一次重传，所以等待的时间需要归零
    }
}
void Timer::close() {
    opentick = false;
    retransmission_timeout = _rto;
    times = 0;
    consecutive_times = 0;
}

TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , ti(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return flighted_bytes; }

void TCPSender::fill_window() {
    // 填充一个发送滑动窗口
    if (!syn) {
        // 还没有建立连接，
        syn = true;
        TCPSegment seg;
        seg.header().syn = true;
        sendsegment(seg);
        return;
    }
    // 在这里尽可能多的填充窗口
    size_t flowwindow =
        win_size ? win_size : 0;  // 制造一个滑动窗口，如果此时的滑动窗口为0,我们也要设置一个位置，进行对窗口进行探测psh
    size_t remain;
    while ((remain = flowwindow - (_next_seqno - recvd_ack)) != 0) {
        // 制作TCP报文
        if (!_stream.eof()) {
            TCPSegment seg;
            size_t length = min(remain, TCPConfig::MAX_PAYLOAD_SIZE);  // 在窗口可容纳的空间和最大值进行取较小的值

            string str = _stream.read(length);  // 从流里面读取数据
            seg.payload() = Buffer(move(str));  // 这个buffer是一个引用计数的字符串，只读
            if (seg.length_in_sequence_space() == 0) {
                // 都没有数据了，说明这个报文没有用
                break;
            } else {
                //否则就是这个报文有数据
                sendsegment(seg);  // 把这个报文发送出去
            }
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    //  DUMMY_CODE(ackno, window_size);
    // 根据已经收到的这个ack来把之前的数据给删除掉
    // 首先检查这个ackno是否有效
    uint64_t absack = unwrap(ackno, _isn, _next_seqno);
    if (absack > _next_seqno) {
        // 无效的ack
        // 我们应该直接返回
        return;
    }
    recvd_ack = absack;
    win_size = window_size;  // 远程建议我们把滑动窗口设置为这个值
    while (!unackseg.empty()) {
        TCPSegment front = unackseg.front();
        if (unwrap(front.header().seqno, _isn, _next_seqno) + front.length_in_sequence_space() <= recvd_ack) {
            // 整个数据包的数据都小于这个ack就要进行去掉未发送的数据
            flighted_bytes -= front.length_in_sequence_space();
            unackseg.pop();
        } else {
            break;
        }
    }
    fill_window();  // 窗口变大了，需要进行继续填充窗口
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    // DUMMY_CODE(ms_since_last_tick);
    if (ti.isopen()) {
        // 如果这个定时器开着的
        ti.times += ms_since_last_tick;
        if (!unackseg.empty() && ti.times > ti.retransmission_timeout)  // 未确认的数据报里面还有数据
        {
            // 如果此时的时间已经大于了重传的时间
            // 就需要进行一个重传
            ti.doubletime();
            TCPSegment front = unackseg.front();
            _segments_out.push(front);  // 进行把报文进行重传
        }
    }
    if (unackseg.empty()) {
        ti.close();
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return ti.consecutive_times; }  // 返回的就是重传的次数

void TCPSender::send_empty_segment() {
    // 构建一个空报文
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    seg.header().ack = true;
    string payload = "";
    seg.payload() = Buffer(move(payload));
}

void TCPSender::sendsegment(TCPSegment &seg) {
    seg.header().seqno = next_seqno();  // 装载序列号
    // seg的payload都已经装载好了
    _segments_out.push(seg);  // 往滑动窗口进行发送
    unackseg.push(seg);
    flighted_bytes += seg.length_in_sequence_space();  // 添加等待确认的数据
    // 更新一下接下来要发送的序列号的序号
    _next_seqno += seg.length_in_sequence_space();  // 下一次重这个地方开始发送
    // 启动重传定时器
    if (!ti.isopen()) {
        ti.start();
    }
}