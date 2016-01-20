/**
 * Autogenerated by Thrift Compiler (0.9.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef ms_TYPES_H
#define ms_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>



namespace ms {

struct ResponseCode {
  enum type {
    UNKNOWN = 0,
    OK = 1,
    ERROR = 2
  };
};

extern const std::map<int, const char*> _ResponseCode_VALUES_TO_NAMES;

typedef struct _MessageRequest__isset {
  _MessageRequest__isset() : msh_id(false), msg_id(false), msg(false) {}
  bool msh_id;
  bool msg_id;
  bool msg;
} _MessageRequest__isset;

class MessageRequest {
 public:

  static const char* ascii_fingerprint; // = "743EA9734E69DEAC002639FDA3981D98";
  static const uint8_t binary_fingerprint[16]; // = {0x74,0x3E,0xA9,0x73,0x4E,0x69,0xDE,0xAC,0x00,0x26,0x39,0xFD,0xA3,0x98,0x1D,0x98};

  MessageRequest() : msh_id(0), msg_id(), msg() {
  }

  virtual ~MessageRequest() throw() {}

  int64_t msh_id;
  std::string msg_id;
  std::string msg;

  _MessageRequest__isset __isset;

  void __set_msh_id(const int64_t val) {
    msh_id = val;
    __isset.msh_id = true;
  }

  void __set_msg_id(const std::string& val) {
    msg_id = val;
    __isset.msg_id = true;
  }

  void __set_msg(const std::string& val) {
    msg = val;
    __isset.msg = true;
  }

  bool operator == (const MessageRequest & rhs) const
  {
    if (__isset.msh_id != rhs.__isset.msh_id)
      return false;
    else if (__isset.msh_id && !(msh_id == rhs.msh_id))
      return false;
    if (__isset.msg_id != rhs.__isset.msg_id)
      return false;
    else if (__isset.msg_id && !(msg_id == rhs.msg_id))
      return false;
    if (__isset.msg != rhs.__isset.msg)
      return false;
    else if (__isset.msg && !(msg == rhs.msg))
      return false;
    return true;
  }
  bool operator != (const MessageRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const MessageRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(MessageRequest &a, MessageRequest &b);

typedef struct _MessageResponse__isset {
  _MessageResponse__isset() : code(false), msg(false) {}
  bool code;
  bool msg;
} _MessageResponse__isset;

class MessageResponse {
 public:

  static const char* ascii_fingerprint; // = "8F91EF8DF0BD202ABA85195A6109549D";
  static const uint8_t binary_fingerprint[16]; // = {0x8F,0x91,0xEF,0x8D,0xF0,0xBD,0x20,0x2A,0xBA,0x85,0x19,0x5A,0x61,0x09,0x54,0x9D};

  MessageResponse() : code((ResponseCode::type)0), msg() {
  }

  virtual ~MessageResponse() throw() {}

  ResponseCode::type code;
  std::string msg;

  _MessageResponse__isset __isset;

  void __set_code(const ResponseCode::type val) {
    code = val;
    __isset.code = true;
  }

  void __set_msg(const std::string& val) {
    msg = val;
    __isset.msg = true;
  }

  bool operator == (const MessageResponse & rhs) const
  {
    if (__isset.code != rhs.__isset.code)
      return false;
    else if (__isset.code && !(code == rhs.code))
      return false;
    if (__isset.msg != rhs.__isset.msg)
      return false;
    else if (__isset.msg && !(msg == rhs.msg))
      return false;
    return true;
  }
  bool operator != (const MessageResponse &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const MessageResponse & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(MessageResponse &a, MessageResponse &b);

} // namespace

#endif
