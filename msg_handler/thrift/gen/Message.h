/**
 * Autogenerated by Thrift Compiler (0.9.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef Message_H
#define Message_H

#include <thrift/TDispatchProcessor.h>
#include "ms_types.h"

namespace ms {

class MessageIf {
 public:
  virtual ~MessageIf() {}
  virtual void dispatch(MessageResponse& _return, const MessageRequest& request) = 0;
};

class MessageIfFactory {
 public:
  typedef MessageIf Handler;

  virtual ~MessageIfFactory() {}

  virtual MessageIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(MessageIf* /* handler */) = 0;
};

class MessageIfSingletonFactory : virtual public MessageIfFactory {
 public:
  MessageIfSingletonFactory(const boost::shared_ptr<MessageIf>& iface) : iface_(iface) {}
  virtual ~MessageIfSingletonFactory() {}

  virtual MessageIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(MessageIf* /* handler */) {}

 protected:
  boost::shared_ptr<MessageIf> iface_;
};

class MessageNull : virtual public MessageIf {
 public:
  virtual ~MessageNull() {}
  void dispatch(MessageResponse& /* _return */, const MessageRequest& /* request */) {
    return;
  }
};

typedef struct _Message_dispatch_args__isset {
  _Message_dispatch_args__isset() : request(false) {}
  bool request;
} _Message_dispatch_args__isset;

class Message_dispatch_args {
 public:

  Message_dispatch_args() {
  }

  virtual ~Message_dispatch_args() throw() {}

  MessageRequest request;

  _Message_dispatch_args__isset __isset;

  void __set_request(const MessageRequest& val) {
    request = val;
  }

  bool operator == (const Message_dispatch_args & rhs) const
  {
    if (!(request == rhs.request))
      return false;
    return true;
  }
  bool operator != (const Message_dispatch_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Message_dispatch_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Message_dispatch_pargs {
 public:


  virtual ~Message_dispatch_pargs() throw() {}

  const MessageRequest* request;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Message_dispatch_result__isset {
  _Message_dispatch_result__isset() : success(false) {}
  bool success;
} _Message_dispatch_result__isset;

class Message_dispatch_result {
 public:

  Message_dispatch_result() {
  }

  virtual ~Message_dispatch_result() throw() {}

  MessageResponse success;

  _Message_dispatch_result__isset __isset;

  void __set_success(const MessageResponse& val) {
    success = val;
  }

  bool operator == (const Message_dispatch_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const Message_dispatch_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Message_dispatch_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Message_dispatch_presult__isset {
  _Message_dispatch_presult__isset() : success(false) {}
  bool success;
} _Message_dispatch_presult__isset;

class Message_dispatch_presult {
 public:


  virtual ~Message_dispatch_presult() throw() {}

  MessageResponse* success;

  _Message_dispatch_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class MessageClient : virtual public MessageIf {
 public:
  MessageClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  MessageClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void dispatch(MessageResponse& _return, const MessageRequest& request);
  void send_dispatch(const MessageRequest& request);
  void recv_dispatch(MessageResponse& _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class MessageProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<MessageIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (MessageProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_dispatch(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  MessageProcessor(boost::shared_ptr<MessageIf> iface) :
    iface_(iface) {
    processMap_["dispatch"] = &MessageProcessor::process_dispatch;
  }

  virtual ~MessageProcessor() {}
};

class MessageProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  MessageProcessorFactory(const ::boost::shared_ptr< MessageIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< MessageIfFactory > handlerFactory_;
};

class MessageMultiface : virtual public MessageIf {
 public:
  MessageMultiface(std::vector<boost::shared_ptr<MessageIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~MessageMultiface() {}
 protected:
  std::vector<boost::shared_ptr<MessageIf> > ifaces_;
  MessageMultiface() {}
  void add(boost::shared_ptr<MessageIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void dispatch(MessageResponse& _return, const MessageRequest& request) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->dispatch(_return, request);
    }
    ifaces_[i]->dispatch(_return, request);
    return;
  }

};

} // namespace

#endif
