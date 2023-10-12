#include "msgpack.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Msgpack::_bind_methods() {
    ClassDB::bind_method(D_METHOD("pack", "data"), &Msgpack::pack);
    ClassDB::bind_method(D_METHOD("unpack", "data"), &Msgpack::unpack);
}

Msgpack::Msgpack() = default;
Msgpack::~Msgpack() = default;

void Msgpack::_pack(const Variant& data, StreamPeerBuffer &buffer, Dictionary &error) { // NOLINT(*-no-recursion)
    switch (data.get_type()) {
        case Variant::Type::NIL: {
            buffer.put_u8(MsgpackFormat::NIL);
            break;
        }
        case Variant::Type::BOOL: {
            bool p_data = bool(data);

            if (p_data) {
                buffer.put_u8(MsgpackFormat::TRUE);
            } else {
                buffer.put_u8(MsgpackFormat::FALSE);
            }
            break;
        }
        case Variant::Type::INT: {
            int64_t p_data = int64_t(data);

            if (-(1 << 5) <= p_data && p_data <= (1 << 7) - 1) {
                buffer.put_8(int8_t(p_data));
            } else if (-(1 << 7) <= p_data and p_data <= (1 << 7)) {
                buffer.put_u8(MsgpackFormat::INT_8);
                buffer.put_8(int8_t(p_data));
            } else if (-(1 << 15) <= p_data and p_data <= (1 << 15)) {
                buffer.put_u8(MsgpackFormat::INT_16);
                buffer.put_16(int16_t(p_data));
            } else if (-(int64_t(1) << 31) <= p_data and p_data <= (int64_t(1) << 31)) {
                buffer.put_u8(MsgpackFormat::INT_32);
                buffer.put_32(int32_t(p_data));
            } else {
                buffer.put_u8(MsgpackFormat::INT_64);
                buffer.put_64(int64_t(p_data));
            }
            break;
        }
        case Variant::Type::FLOAT: {
            float p_data = float(data);

            buffer.put_u8(MsgpackFormat::FLOAT_32);
            buffer.put_float(p_data);
            break;
        }
        case Variant::Type::STRING: {
            PackedByteArray p_data = data.operator String().to_utf8_buffer();
            int64_t p_data_size = p_data.size();

            if (p_data_size <= (1 << 5) - 1) {
                buffer.put_u8(MsgpackFormat::FIXSTR | p_data_size);
            } else if (p_data_size <= (1 << 8) - 1) {
                buffer.put_u8(MsgpackFormat::STR_8);
                buffer.put_u8(p_data_size);
            } else if (p_data_size <= (1 << 16) - 1) {
                buffer.put_u8(MsgpackFormat::STR_16);
                buffer.put_u16(p_data_size);
            } else if (p_data_size <= (int64_t(1) << 32) - 1) {
                buffer.put_u8(MsgpackFormat::STR_32);
                buffer.put_u32(p_data_size);
            } else {
                error["error"] = Error::FAILED;
                error["error_message"] = "PackedByteArray size out of range!";
            }
            buffer.put_data(p_data);
            break;
        }
        case Variant::Type::ARRAY: {
            Array p_data = data.operator Array();
            int64_t p_data_size = p_data.size();

            if (p_data_size <= 15) {
                buffer.put_u8(0x90 | p_data_size);
            } else if (p_data_size <= (1 << 16) - 1) {
                buffer.put_u8(MsgpackFormat::ARRAY_16);
                buffer.put_u16(p_data_size);
            } else if (p_data_size <= (int64_t(1) << 32) - 1) {
                buffer.put_u8(MsgpackFormat::ARRAY_32);
                buffer.put_u32(p_data_size);
            } else {
                error["error"] = Error::FAILED;
                error["error_message"] = "Array size out of range!";
            }
            for (int idx = 0; idx < p_data_size; idx++) {
                _pack(p_data[idx], buffer, error);
                if (int(error.get("error", Error::FAILED)) != Error::OK) {
                    return;
                }
            }
            break;
        }
        case Variant::Type::PACKED_BYTE_ARRAY: {
            PackedByteArray p_data = data.operator PackedByteArray();
            int64_t p_data_size = p_data.size();

            if (p_data_size <= (1 << 8) - 1) {
                buffer.put_u8(MsgpackFormat::BIN_8);
                buffer.put_u8(p_data_size);
            } else if (p_data_size <= (1 << 16) - 1) {
                buffer.put_u8(MsgpackFormat::BIN_16);
                buffer.put_u8(p_data_size);
            } else if (p_data_size <= (int64_t(1) << 32) - 1) {
                buffer.put_u8(MsgpackFormat::BIN_32);
                buffer.put_u8(p_data_size);
            } else {
                error["error"] = Error::FAILED;
                error["error_message"] = "PackedByteArray size out of range!";
            }
            buffer.put_data(p_data);
            break;
        }
        case Variant::Type::DICTIONARY: {
            Dictionary p_data = data.operator Dictionary();
            int64_t p_data_size = p_data.size();

            if (p_data_size <= 15) {
                buffer.put_u8(MsgpackFormat::FIXMAP | p_data_size);
            } else if (p_data_size <= (1 << 16) - 1) {
                buffer.put_u8(MsgpackFormat::MAP_16);
                buffer.put_u16(p_data_size);
            } else if (p_data_size <= (int64_t(1) << 32) - 1) {
                buffer.put_u8(MsgpackFormat::MAP_32);
                buffer.put_u32(p_data_size);
            } else {
                error["error"] = Error::FAILED;
                error["error_message"] = "Dictionary size out of range!";
            }
            Array p_data_keys = p_data.keys();
            Array p_data_values = p_data.values();
            for (int idx = 0; idx < p_data_size; idx++) {
                _pack(p_data_keys[idx], buffer, error);
                if (int(error.get("error", Error::FAILED)) != Error::OK) {
                    return;
                }

                _pack(p_data_values[idx], buffer, error);
                if (int(error.get("error", Error::FAILED)) != Error::OK) {
                    return;
                }
            }
            break;
        }
        default: {
            error["error"] = Error::ERR_INVALID_DATA;
            error["error_message"] = "Unsupported data type: " + Variant::get_type_name(data.get_type());
            break;
        }
    }
}

Variant Msgpack::_unpack(StreamPeerBuffer *buffer, Dictionary *error) {
    return nullptr;
}

PackedByteArray Msgpack::pack(const Variant& data) {
    StreamPeerBuffer buffer;
    buffer.set_big_endian(true);

    Dictionary error;
    error["error"] = Error::OK;
    error["error_message"] = "";

    _pack(data, buffer, error);

    return buffer.get_data_array();
}

Variant Msgpack::unpack(const PackedByteArray& data) {
    UtilityFunctions::print("[MSGPACK] unpack");
    return nullptr;
}
