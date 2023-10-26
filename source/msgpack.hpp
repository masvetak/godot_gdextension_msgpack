#ifndef MSGPACK_HPP
#define MSGPACK_HPP

#include "msgpack_common.hpp"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/stream_peer_buffer.hpp>

namespace godot {
    class Msgpack : public RefCounted {
        GDCLASS(Msgpack, RefCounted)

    public:
        Msgpack();
        ~Msgpack() override;

        static Msgpack *get_singleton();

        PackedByteArray pack(const Variant& data);
        Variant unpack(const PackedByteArray& data);

    protected:
        static void _bind_methods();

    private:
        static Msgpack *msgpack;

        void _pack(const Variant& data, StreamPeerBuffer *buffer, Dictionary &error);
        Variant _unpack(StreamPeerBuffer *buffer, Dictionary &error);
    };
}

#endif //MSGPACK_HPP
