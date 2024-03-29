#ifndef __SRPC_CODEC_H__
#define __SRPC_CODEC_H__
#include "fluent.hpp"
#include "vsjson.hpp"
namespace srpc {

class Codec {
public:
    bool verify(fluent::Buffer &buffer) const {
        if(buffer.unread() < sizeof(uint32_t)) return false;
        uint32_t beLength = *reinterpret_cast<uint32_t*>(buffer.readBuffer());
        uint32_t length = ::ntohl(beLength);
        return buffer.unread() >= sizeof(uint32_t) + length;
    }

    vsjson::Json decode(fluent::Buffer &buffer) const {
        uint32_t beLength = *reinterpret_cast<uint32_t*>(buffer.readBuffer());
        uint32_t length = ntohl(beLength);
        buffer.hasRead(sizeof(uint32_t));
        vsjson::Json json = vsjson::parse(buffer.readBuffer());
        buffer.hasRead(length);
        return json;
    }
};

} // srpc
#endif