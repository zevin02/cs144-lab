#include "sender_harness.hh"
#include "wrapping_integers.hh"

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

using namespace std;

int main() {
    try {
        auto rd = get_random_generator();

        //     {
        //         TCPConfig cfg;
        //         WrappingInt32 isn(rd());
        //         cfg.fixed_isn = isn;//这里给配置一个isn，随机生成一个序列号

        //         TCPSenderTestHarness test{"SYN sent test", cfg};//在这个地方发送数据，填充滑动窗口
        //         test.execute(ExpectState{TCPSenderStateSummary::SYN_SENT});//TCP发送段处于syn_sent状态
        //         test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
        //         test.execute(ExpectBytesInFlight{1});//预期这个未被接收的数据为1
        //     }
        // // cout<<"28"<<endl;//在这个地方挂掉了
        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;

            TCPSenderTestHarness test{"SYN acked test", cfg};
            test.execute(ExpectState{TCPSenderStateSummary::SYN_SENT});
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(
                isn));  // 这个地方发送了一个报文
            test.execute(ExpectBytesInFlight{1});
            test.execute(AckReceived{WrappingInt32{isn + 1}});  // 这个地方挂掉了
            test.execute(ExpectState{TCPSenderStateSummary::SYN_ACKED});
            cout << __LINE__ << endl;

            test.execute(ExpectNoSegment{});
            cout << __LINE__ << endl;

            test.execute(ExpectBytesInFlight{0});
        }
        cout << __LINE__ << endl;

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;

            TCPSenderTestHarness test{"SYN -> wrong ack test", cfg};
            test.execute(ExpectState{TCPSenderStateSummary::SYN_SENT});
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(ExpectBytesInFlight{1});
            test.execute(AckReceived{WrappingInt32{isn}});
            test.execute(ExpectState{TCPSenderStateSummary::SYN_SENT});
            test.execute(ExpectNoSegment{});
            test.execute(ExpectBytesInFlight{1});
        }
        cout << "59" << endl;

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;

            TCPSenderTestHarness test{"SYN acked, data", cfg};
            test.execute(ExpectState{TCPSenderStateSummary::SYN_SENT});
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(ExpectBytesInFlight{1});
            test.execute(AckReceived{WrappingInt32{isn + 1}});
            test.execute(ExpectState{TCPSenderStateSummary::SYN_ACKED});
            test.execute(ExpectNoSegment{});
            test.execute(ExpectBytesInFlight{0});
            test.execute(WriteBytes{"abcdefgh"});
            test.execute(Tick{1});
            test.execute(ExpectState{TCPSenderStateSummary::SYN_ACKED});
            test.execute(ExpectSegment{}.with_seqno(isn + 1).with_data("abcdefgh"));
            test.execute(ExpectBytesInFlight{8});
            test.execute(AckReceived{WrappingInt32{isn + 9}});
            test.execute(ExpectState{TCPSenderStateSummary::SYN_ACKED});
            test.execute(ExpectNoSegment{});
            test.execute(ExpectBytesInFlight{0});
            cout << __LINE__ << endl;

            test.execute(ExpectSeqno{WrappingInt32{isn + 9}});
        }
        cout << __LINE__ << endl;

    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    return EXIT_SUCCESS;
}
