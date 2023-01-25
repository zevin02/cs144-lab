#ifndef SPONGE_LIBSPONGE_TCP_HEADER_HH
#define SPONGE_LIBSPONGE_TCP_HEADER_HH

#include "parser.hh"
#include "wrapping_integers.hh"

//! \brief [TCP](\ref rfc::rfc793) segment header
//! \note TCP options are not supported ，这里不支持TCP选项
// TCP报头

struct TCPHeader {
    static constexpr size_t LENGTH = 20;  //!< [TCP](\ref rfc::rfc793) header length, not including options，固定的报头长度=20字节，不包含选项

    //! \struct TCPHeader
    //! ~~~{.txt}
    //!   0                   1                   2                   3
    //!   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //!  |          Source Port          |       Destination Port        |
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //!  |                        Sequence Number                        |
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //!  |                    Acknowledgment Number                      |
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //!  |  Data |           |U|A|P|R|S|F|                               |
    //!  | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
    //!  |       |           |G|K|H|T|N|N|                               |
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //!  |           Checksum            |         Urgent Pointer        |
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //!  |                    Options                    |    Padding    |
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //!  |                             data                              |
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //! ~~~

    //! \name TCP Header fields
    //!@{
    uint16_t sport = 0;         //!< source port
    uint16_t dport = 0;         //!< destination port
    WrappingInt32 seqno{0};     //!< sequence number
    WrappingInt32 ackno{0};     //!< ack number
    uint8_t doff = LENGTH / 4;  //!< data offset，4位片偏移量，以4字节为单位，这样操作就是不包含选项了，整个报头长度就是20字节
    bool urg = false;           //!< urgent flag
    bool ack = false;           //!< ack flag
    bool psh = false;           //!< push flag
    bool rst = false;           //!< rst flag
    bool syn = false;           //!< syn flag
    bool fin = false;           //!< fin flag
    uint16_t win = 0;           //!< window size
    uint16_t cksum = 0;         //!< checksum
    uint16_t uptr = 0;          //!< urgent pointer
    //!@}

    //! Parse the TCP fields from the provided NetParser
    ParseResult parse(NetParser &p);

    //! Serialize the TCP fields
    //将其序列化，转送给下一层
    std::string serialize() const;

    //! Return a string containing a header in human-readable format
    //把这个完整TCP报头以人可读的形式显示
    std::string to_string() const;

    //! Return a string containing a human-readable summary of the header
    // 简单的显示报头的关键信息
    std::string summary() const;

    bool operator==(const TCPHeader &other) const;
};

#endif  // SPONGE_LIBSPONGE_TCP_HEADER_HH
