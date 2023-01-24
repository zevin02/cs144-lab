Lab 2 Writeup
=============

My name: [your name here]

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This lab took me about [n] hours to do. I [did/did not] attend the lab session.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[]

Implementation Challenges:
[]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with:
  - uint32_t->int32_t,uint64_t 之间的转化，很麻烦
- Optional: I think you could make this lab better by: [describe]
- Optional: I was surprised by: [describe]
- Optional: I'm not sure about: [describe]


* TCP 接收的数据只接收有效载荷
* ack表示下一个需要发送的字节号
* winddow：接收缓冲区剩余大小
* syn和fin都代表一个字节
* tcp序列号不从0开始，第一个序号一般都是一个随机32位数字，代表了syn的序列号，之后的序列号正常，第一个字节为syn+1
* tcp序列号是32位的
* 我们在流中的第一个字节不是syn，而是真正发送过来的第一个字节
* 如果发送的数据很多，所以32位序列号会发送循环，一个序列号可能对应在绝对序号可能有多个，所以需要我们进行判断该序列号真正对应的哪个绝对序号
