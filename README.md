# Chlorum

A self-study project to learn more about POSIX sockets and
concurrent IO models. The main efforts were spent to create
safe wraps of C POSIX functions with exception handling in mind.

Event-loop with receive/send buffering used

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
<i>Server ⬅━━━━━━ </i>
</pre>
