#ifndef SPONGE_LIBSPONGE_WRAPPING_INTEGERS_HH
#define SPONGE_LIBSPONGE_WRAPPING_INTEGERS_HH

#include <cstdint>
#include <ostream>

//! \brief A 32-bit integer, expressed relative to an arbitrary initial sequence number (ISN)
//! \note This is used to express TCP sequence numbers (seqno) and acknowledgment numbers (ackno)
class WrappingInt32 {//这个就是tcp里面使用的序列号，但是我们需要把这个转化成绝对序列号（绝对序列号是64位的）和流索引（在streeam里面组装，64位的）
  private:
    uint32_t _raw_value;  //!< The raw 32-bit stored integer

  public:
    //! Construct from a raw 32-bit unsigned integer
    explicit WrappingInt32(uint32_t raw_value) : _raw_value(raw_value) {}

    uint32_t raw_value() const { return _raw_value; }  //!< Access raw stored value
};

//! Transform a 64-bit absolute sequence number (zero-indexed) into a 32-bit relative sequence number
//! \param n the absolute sequence number
//! \param isn the initial sequence number
//! \returns the relative sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn);//绝对序列号转化成tcp中的相对序列号

//! Transform a 32-bit relative sequence number into a 64-bit absolute sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute sequence number
//! \returns the absolute sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
//checkpoint把最后一个重新组装的字节当作checkpoint
//
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint);//将一个相对序列号转化成绝对序列号

//! \name Helper functions
//!@{

//! \brief The offset of `a` relative to `b`
//! \param b the starting point
//! \param a the ending point
//! \returns the number of increments needed to get from `b` to `a`,
//! negative if the number of decrements needed is less than or equal to
//! the number of increments
inline int32_t operator-(WrappingInt32 a, WrappingInt32 b) { return a.raw_value() - b.raw_value(); }

//! \brief Whether the two integers are equal.
inline bool operator==(WrappingInt32 a, WrappingInt32 b) { return a.raw_value() == b.raw_value(); }

//! \brief Whether the two integers are not equal.
inline bool operator!=(WrappingInt32 a, WrappingInt32 b) { return !(a == b); }

//! \brief Serializes the wrapping integer, `a`.
inline std::ostream &operator<<(std::ostream &os, WrappingInt32 a) { return os << a.raw_value(); }

//! \brief The point `b` steps past `a`.
inline WrappingInt32 operator+(WrappingInt32 a, uint32_t b) { return WrappingInt32{a.raw_value() + b}; }

//! \brief The point `b` steps before `a`.
inline WrappingInt32 operator-(WrappingInt32 a, uint32_t b) { return a + -b; }
//!@}

#endif  // SPONGE_LIBSPONGE_WRAPPING_INTEGERS_HH
