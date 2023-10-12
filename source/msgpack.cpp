#include "msgpack.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Msgpack::_bind_methods() {

}

Msgpack::Msgpack() {
    UtilityFunctions::print("[MSGPACK] constructor");
}

Msgpack::~Msgpack() {
    UtilityFunctions::print("[MSGPACK] destructor");
}

void Msgpack::test(String data) {
    UtilityFunctions::print("[MSGPACK] test data: " + data);
}
