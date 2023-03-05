#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>

//! \brief The "sender" part of a TCP implementation.

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.

// 重传定时器类
class Timer {
  public:
    unsigned int retransmission_timeout{};
    size_t consecutive_times = 0;
    bool opentick = false;  // 用来判断定时器是否开启,有未发送的数据的话，就要打开,没有的话就关闭
    size_t times = 0;  // 记录过了多少的时间
    unsigned int _rto = 0;

  public:
    Timer(unsigned int rto);
    void doubletime();
    bool isopen() { return opentick; }
    void start() {
        opentick = true;
        retransmission_timeout = _rto;
        times=0;
        consecutive_times=0;
    }  // 重新启动
    void close();
};

class TCPSender {
  private:
    //! our initial sequence number, the number for our SYN.
    WrappingInt32 _isn;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};

    //! retransmission timer for the connection
    unsigned int _initial_retransmission_timeout;

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;
    size_t flighted_bytes = 0;          // 未被ack的字节
    std::queue<TCPSegment> unackseg{};  // 未被对方ack的报文
    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};
    Timer ti;             // 计时器
    size_t win_size = 0;  // 记录此时的滑动窗口的大小
    bool syn = false;
    bool fin = false;
    uint64_t recvd_ack = 0;  // 已经收到的ack,用来构建滑动窗口

  public:
    //! Initialize a TCPSender
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    //! \name "Input" interface for the writer
    //!@{
    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }
    //!@}

    //! \name Methods that can cause the TCPSender to send a segment
    //!@{

    //! \brief A new acknowledgment was received
    void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    //! \brief create and send segments to fill as much of the window as possible
    void fill_window();

    //! \brief Notifies the TCPSender of the passage of time
    void tick(const size_t ms_since_last_tick);
    //!@}

    //! \name Accessors
    //!@{

    //! \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //! \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    //! \brief TCPSegments that the TCPSender has enqueued for transmission.
    //! \note These must be dequeued and sent by the TCPConnection,
    //! which will need to fill in the fields that are set by the TCPReceiver
    //! (ackno and window size) before sending.
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    //!@}

    //! \name What is the next sequence number? (used for testing)
    //!@{

    //! \brief absolute seqno for the next byte to be sent
    uint64_t next_seqno_absolute() const { return _next_seqno; }  // 获得下一个需要发送的绝对序列号

    //! \brief relative seqno for the next byte to be sent
    WrappingInt32 next_seqno() const {
        return wrap(_next_seqno, _isn);
    }  // 使用这个可以获得下一个需要发送的序列号的相对号码
    //!@}
    void sendsegment(TCPSegment &seg);  // 发送数据报到网络里面
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
