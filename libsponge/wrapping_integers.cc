#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    // DUMMY_CODE(n, isn);
    return isn+n;//绝对转相对
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`，最接近这个checkpoint的绝对序号
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    //checkpoint是最近解封装的绝对序号
    WrappingInt32 check32= wrap(checkpoint,isn);
    int32_t diff=n-check32;//这个相减是个负数
    if(diff<0&&checkpoint+diff>checkpoint)//差值小于0,同时这个diff相加后还变大了，说明这个diff在变成64位的，整形提升
    {
        return checkpoint+uint32_t(diff);//这里实际上我们只需要提升到32位即可
    }

    //把n转化成64位的
    uint64_t absolute_seq=checkpoint+diff;

    return absolute_seq;
}
