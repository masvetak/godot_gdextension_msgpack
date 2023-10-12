#ifndef MSGPACK_HPP
#define MSGPACK_HPP

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/stream_peer_buffer.hpp>

namespace godot {
    class Msgpack : public Object {
        GDCLASS(Msgpack, Object)

    private:
        enum MsgpackFormat {
            POSITIVE_FIXINT = 0x00,
            FIXMAP = 0x80,
            FIXARRAY = 0x90,
            FIXSTR = 0xA0,
            NIL = 0xC0,
            NEVER_USED = 0xC1,
            FALSE = 0xC2,
            TRUE = 0xC3,
            BIN_8 = 0xC4,
            BIN_16 = 0xC5,
            BIN_32 = 0xC6,
            EXT_8 = 0xC7,
            EXT_16 = 0xC8,
            EXT_32 = 0xC9,
            FLOAT_32 = 0xCA,
            FLOAT_64 = 0xCB,
            UINT_8 = 0xCC,
            UINT_16 = 0xCD,
            UINT_32 = 0xCE,
            UINT_64 = 0xCF,
            INT_8 = 0xD0,
            INT_16 = 0xD1,
            INT_32 = 0xD2,
            INT_64 = 0xD3,
            FIXEXT_1 = 0xD4,
            FIXEXT_2 = 0xD5,
            FIXEXT_4 = 0xD6,
            FIXEXT_8 = 0xD7,
            FIXEXT_16 = 0xD8,
            STR_8 = 0xD9,
            STR_16 = 0xDA,
            STR_32 = 0xDB,
            ARRAY_16 = 0xDC,
            ARRAY_32 = 0xDD,
            MAP_16 = 0xDE,
            MAP_32 = 0xDF,
            NEGATIVE_FIXINT = 0xE0
        };

        void _pack(const Variant& data, StreamPeerBuffer &buffer, Dictionary &error);
        Variant _unpack(StreamPeerBuffer *buffer, Dictionary *error);

    protected:
        static void _bind_methods();

    public:
        Msgpack();
        ~Msgpack() override;

        PackedByteArray pack(const Variant& data);
        Variant unpack(const PackedByteArray& data);

    };
}

#endif //MSGPACK_HPP
