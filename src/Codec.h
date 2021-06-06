#ifndef __RPC_CODEC_H__
#define __RPC_CODEC_H__
#include "mutty.hpp"
#include "vsjson.hpp"
using namespace mutty;
using namespace vsjson;

class Codec {
public:
    bool verify(Buffer &buffer) {
        if(buffer.unread() < sizeof(uint32_t)) return false;
        uint32_t beLength = *reinterpret_cast<uint32_t*>(buffer.readBuffer());
        uint32_t length = ntohl(beLength);
        return buffer.unread() >= sizeof(uint32_t) + length;
    }

    Json decode(Buffer &buffer) {
        uint32_t beLength = *reinterpret_cast<uint32_t*>(buffer.readBuffer());
        uint32_t length = ntohl(beLength);
        buffer.hasRead(sizeof(uint32_t));
        // char ch;
        // bool overload = buffer.readBuffer() + length != buffer.end();
        // if(overload) {
        //     char &backup = buffer.readBuffer()[length];
        //     ch = backup;
        //     backup = '\0';
        // }
        Json json = parse(/*std::string(*/buffer.readBuffer()/*, length)*/);
        // if(overload) {
        //     buffer.readBuffer()[length] = ch;
        // }
        buffer.hasRead(length);
        return json;
    }
};
#endif