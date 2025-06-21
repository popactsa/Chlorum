# Chlorum

A self-study project to learn more about POSIX sockets and
concurrent IO models. The main efforts were spent to create
safe wraps of C POSIX functions with exception handling in mind.

Event-loop server with non-blocking receive/send buffering used.

## Building
- mkdir build
- cd build && cmake ..
- cd src && ./chlorum_run

You can create a debug-build: cmake .. --config Build

## Demo
A single server and multiple clients executed in separate threads
are talking to each other: when server receives a interpreted
message it sends echo back to the sender.
Then all client threads are joined and after 5 seconds created again.

## Class relationship overview:
<pre>
⇨ - template parameter
→ - inheritance
➡ - operates with
<i>Socket</i>
↓
<i>TcpSocket</i> ―――――――――――――――╮
↓                        │
<i>ListeningSocket</i> ⇦════════│═══════════ <i>Packet type</i>
┃           ↘ (produces) ↓     ⬃
┃              <i>ConnectionSocket</i>
⬇              ⬋
<i>Server / Client</i>
</pre>

## Packet format
Transferred packet is structured as follows: first few bytes(default : `qHeaderLen` = 4 bytes)
contain `msg_sz_`. They are followed with `msg_sz_` bytes containing
raw data.
Packet is stored as a std::vector<char> with `qHeaderLen` + `msg_sz_` elements.

## Server recv/send data buffers
All data read/written through a connection socket is being buffered and interpreted/sent
when possible.

## TcpConnection templated class
It can be used with any packet format with message size passed as a packet prefix:
it redirects packet writing to packet ctors, buffer are just flushed/filled regardless
of packet format.

## Tests
Not provided yet.
