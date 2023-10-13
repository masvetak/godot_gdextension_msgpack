#ifndef MSGPACK_HPP
#define MSGPACK_HPP

#include "msgpack_common.hpp"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/stream_peer_buffer.hpp>

namespace godot {
    class Msgpack : public Object {
        GDCLASS(Msgpack, Object)

    private:
        StreamPeerBuffer pack_buffer;
        StreamPeerBuffer unpack_buffer;

        void _pack(const Variant& data, StreamPeerBuffer &buffer, Dictionary &error);
        Variant _unpack(StreamPeerBuffer &buffer, Dictionary &error);

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
