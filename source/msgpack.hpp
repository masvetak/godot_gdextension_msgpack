#ifndef MSGPACK_HPP
#define MSGPACK_HPP

#include <godot_cpp/classes/object.hpp>

namespace godot {
    class Msgpack : public Object {
        GDCLASS(Msgpack, Object)

    private:
        String m_data;

    protected:
        static void _bind_methods();

    public:
        Msgpack();
        ~Msgpack();
        void test(String data);

    };
}

#endif //MSGPACK_HPP
