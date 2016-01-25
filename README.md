# xkim
xkim: A simple protobuf based distributed Instant Messaging(IM) server side implementation.

xkim includes two components: msg_handler and msg_server.
msg_handler is to handle network stream, currently only TCP is supported.
msg_server is to implment the busniess logic of the IM system.

msg_handler and msg_server are loosely coupled. We use pilot, a thrift based communication framework to do RPC betwen msg_handler and msg_server. Pilot support load balancing for multiple backend msg_server.


The message is defined in protobuf format.

To contact us:
   linucswf@163.com
   @romi1 at sina weibo
