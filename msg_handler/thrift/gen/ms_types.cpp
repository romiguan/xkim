/**
 * Autogenerated by Thrift Compiler (0.9.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "ms_types.h"

#include <algorithm>

namespace ms {

int _kResponseCodeValues[] = {
  ResponseCode::UNKNOWN,
  ResponseCode::OK,
  ResponseCode::ERROR
};
const char* _kResponseCodeNames[] = {
  "UNKNOWN",
  "OK",
  "ERROR"
};
const std::map<int, const char*> _ResponseCode_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(3, _kResponseCodeValues, _kResponseCodeNames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));

const char* MessageRequest::ascii_fingerprint = "743EA9734E69DEAC002639FDA3981D98";
const uint8_t MessageRequest::binary_fingerprint[16] = {0x74,0x3E,0xA9,0x73,0x4E,0x69,0xDE,0xAC,0x00,0x26,0x39,0xFD,0xA3,0x98,0x1D,0x98};

uint32_t MessageRequest::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I64) {
          xfer += iprot->readI64(this->msh_id);
          this->__isset.msh_id = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readBinary(this->msg_id);
          this->__isset.msg_id = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readBinary(this->msg);
          this->__isset.msg = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t MessageRequest::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("MessageRequest");

  if (this->__isset.msh_id) {
    xfer += oprot->writeFieldBegin("msh_id", ::apache::thrift::protocol::T_I64, 1);
    xfer += oprot->writeI64(this->msh_id);
    xfer += oprot->writeFieldEnd();
  }
  if (this->__isset.msg_id) {
    xfer += oprot->writeFieldBegin("msg_id", ::apache::thrift::protocol::T_STRING, 2);
    xfer += oprot->writeBinary(this->msg_id);
    xfer += oprot->writeFieldEnd();
  }
  if (this->__isset.msg) {
    xfer += oprot->writeFieldBegin("msg", ::apache::thrift::protocol::T_STRING, 3);
    xfer += oprot->writeBinary(this->msg);
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(MessageRequest &a, MessageRequest &b) {
  using ::std::swap;
  swap(a.msh_id, b.msh_id);
  swap(a.msg_id, b.msg_id);
  swap(a.msg, b.msg);
  swap(a.__isset, b.__isset);
}

const char* MessageResponse::ascii_fingerprint = "8F91EF8DF0BD202ABA85195A6109549D";
const uint8_t MessageResponse::binary_fingerprint[16] = {0x8F,0x91,0xEF,0x8D,0xF0,0xBD,0x20,0x2A,0xBA,0x85,0x19,0x5A,0x61,0x09,0x54,0x9D};

uint32_t MessageResponse::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          int32_t ecast0;
          xfer += iprot->readI32(ecast0);
          this->code = (ResponseCode::type)ecast0;
          this->__isset.code = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readBinary(this->msg);
          this->__isset.msg = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t MessageResponse::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("MessageResponse");

  if (this->__isset.code) {
    xfer += oprot->writeFieldBegin("code", ::apache::thrift::protocol::T_I32, 1);
    xfer += oprot->writeI32((int32_t)this->code);
    xfer += oprot->writeFieldEnd();
  }
  if (this->__isset.msg) {
    xfer += oprot->writeFieldBegin("msg", ::apache::thrift::protocol::T_STRING, 2);
    xfer += oprot->writeBinary(this->msg);
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(MessageResponse &a, MessageResponse &b) {
  using ::std::swap;
  swap(a.code, b.code);
  swap(a.msg, b.msg);
  swap(a.__isset, b.__isset);
}

} // namespace
