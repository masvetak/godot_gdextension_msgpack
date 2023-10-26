#include "msgpack.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

Msgpack::Msgpack() {
    ERR_FAIL_COND(msgpack != nullptr);
    msgpack = this;
}

Msgpack::~Msgpack() {
    ERR_FAIL_COND(msgpack != this);
    msgpack = nullptr;
}

Msgpack *Msgpack::get_singleton() {
    return msgpack;
}

PackedByteArray Msgpack::pack(const Variant& data) {
    StreamPeerBuffer *buffer = memnew(StreamPeerBuffer);
    buffer->set_big_endian(true);

    Dictionary error;
    error["error"] = Error::OK;
    error["error_message"] = "";

    _pack(data, buffer, error);
    PackedByteArray result = buffer->get_data_array();
    memdelete(buffer);

    if (int(error["error"]) != Error::OK) {
        UtilityFunctions::print(error["error_message"]);
    }
    return result;
}

Variant Msgpack::unpack(const PackedByteArray& data) {
    StreamPeerBuffer *buffer = memnew(StreamPeerBuffer);
    buffer->set_big_endian(true);
    buffer->set_data_array(data);

    Dictionary error;
    error["error"] = Error::OK;
    error["error_message"] = "";

    Variant result = _unpack(buffer, error);
    memdelete(buffer);

    if (int(error["error"]) != Error::OK) {
        UtilityFunctions::print(error["error_message"]);
    }
    return result;
}

void Msgpack::_bind_methods() {
    ClassDB::bind_method(D_METHOD("pack", "data"), &Msgpack::pack);
    ClassDB::bind_method(D_METHOD("unpack", "data"), &Msgpack::unpack);
}

Msgpack *Msgpack::msgpack = nullptr;

void Msgpack::_pack(const Variant& data, StreamPeerBuffer *buffer, Dictionary &error) {
    switch (data.get_type()) {
        case Variant::Type::NIL: {
            buffer->put_u8(MSGPACK_FORMAT_NIL);
            break;
        }
        case Variant::Type::BOOL: {
            bool p_data = bool(data);

            if (p_data) {
                buffer->put_u8(MSGPACK_FORMAT_TRUE);
            } else {
                buffer->put_u8(MSGPACK_FORMAT_FALSE);
            }
            break;
        }
        case Variant::Type::INT: {
            auto p_data = int64_t(data);

            if (-(1 << 5) <= p_data && p_data <= (1 << 7) - 1) {
                buffer->put_8(int8_t(p_data));
            } else if (-(1 << 7) <= p_data and p_data <= (1 << 7)) {
                buffer->put_u8(MSGPACK_FORMAT_INT_8);
                buffer->put_8(int8_t(p_data));
            } else if (-(1 << 15) <= p_data and p_data <= (1 << 15)) {
                buffer->put_u8(MSGPACK_FORMAT_INT_16);
                buffer->put_16(int16_t(p_data));
            } else if (-(int64_t(1) << 31) <= p_data and p_data <= (int64_t(1) << 31)) {
                buffer->put_u8(MSGPACK_FORMAT_INT_32);
                buffer->put_32(int32_t(p_data));
            } else {
                buffer->put_u8(MSGPACK_FORMAT_INT_64);
                buffer->put_64(int64_t(p_data));
            }
            break;
        }
        case Variant::Type::FLOAT: {
            auto p_data = float(data);

            buffer->put_u8(MSGPACK_FORMAT_FLOAT_32);
            buffer->put_float(p_data);
            break;
        }
        case Variant::Type::STRING: {
            PackedByteArray p_data = data.operator String().to_utf8_buffer();
            int64_t p_data_size = p_data.size();

            if (p_data_size <= (1 << 5) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_FIXSTR | p_data_size);
            } else if (p_data_size <= (1 << 8) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_STR_8);
                buffer->put_u8(p_data_size);
            } else if (p_data_size <= (1 << 16) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_STR_16);
                buffer->put_u16(p_data_size);
            } else if (p_data_size <= (int64_t(1) << 32) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_STR_32);
                buffer->put_u32(p_data_size);
            } else {
                error["error"] = Error::FAILED;
                error["error_message"] = "PackedByteArray size out of range!";
            }
            buffer->put_data(p_data);
            break;
        }
        case Variant::Type::ARRAY: {
            Array p_data = data.operator Array();
            int64_t p_data_size = p_data.size();

            if (p_data_size <= 15) {
                buffer->put_u8(0x90 | p_data_size);
            } else if (p_data_size <= (1 << 16) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_ARRAY_16);
                buffer->put_u16(p_data_size);
            } else if (p_data_size <= (int64_t(1) << 32) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_ARRAY_32);
                buffer->put_u32(p_data_size);
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
                buffer->put_u8(MSGPACK_FORMAT_BIN_8);
                buffer->put_u8(p_data_size);
            } else if (p_data_size <= (1 << 16) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_BIN_16);
                buffer->put_u8(p_data_size);
            } else if (p_data_size <= (int64_t(1) << 32) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_BIN_32);
                buffer->put_u8(p_data_size);
            } else {
                error["error"] = Error::FAILED;
                error["error_message"] = "PackedByteArray size out of range!";
            }
            buffer->put_data(p_data);
            break;
        }
        case Variant::Type::DICTIONARY: {
            Dictionary p_data = data.operator Dictionary();
            int64_t p_data_size = p_data.size();

            if (p_data_size <= 15) {
                buffer->put_u8(MSGPACK_FORMAT_FIXMAP | p_data_size);
            } else if (p_data_size <= (1 << 16) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_MAP_16);
                buffer->put_u16(p_data_size);
            } else if (p_data_size <= (int64_t(1) << 32) - 1) {
                buffer->put_u8(MSGPACK_FORMAT_MAP_32);
                buffer->put_u32(p_data_size);
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

Variant Msgpack::_unpack(StreamPeerBuffer *buffer, Dictionary &error) {
    if (buffer->get_position() == buffer->get_size()) {
        error["error"] = Error::FAILED;
        error["error_message"] = "Unexpected end of input!";
        return nullptr;
    }
    uint8_t head = buffer->get_u8();

    if (head == MSGPACK_FORMAT_NIL) {
        return nullptr;
    } else if (head == MSGPACK_FORMAT_FALSE) {
        return false;
    } else if (head == MSGPACK_FORMAT_TRUE) {
        return true;
    } else if ((head & MSGPACK_FORMAT_FIXMAP) == 0) {
        return head;
    } else if (((~head) & MSGPACK_FORMAT_NEGATIVE_FIXINT) == 0) {
        return head - 256;
    } else if (head == MSGPACK_FORMAT_UINT_8) {
        if (buffer->get_size() - buffer->get_position() < 1) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for UINT_8!";
            return nullptr;
        }
        return buffer->get_u8();
    } else if (head == MSGPACK_FORMAT_UINT_16) {
        if (buffer->get_size() - buffer->get_position() < 2) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for UINT_16!";
            return nullptr;
        }
        return buffer->get_u16();
    } else if (head == MSGPACK_FORMAT_UINT_32) {
        if (buffer->get_size() - buffer->get_position() < 4) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for UINT_32!";
            return nullptr;
        }
        return buffer->get_u32();
    } else if (head == MSGPACK_FORMAT_UINT_64) {
        if (buffer->get_size() - buffer->get_position() < 8) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for UINT_64!";
            return nullptr;
        }
        return buffer->get_u64();
    } else if (head == MSGPACK_FORMAT_INT_8) {
        if (buffer->get_size() - buffer->get_position() < 1) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for INT_8!";
            return nullptr;
        }
        return buffer->get_8();
    } else if (head == MSGPACK_FORMAT_INT_16) {
        if (buffer->get_size() - buffer->get_position() < 2) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for INT_16!";
            return nullptr;
        }
        return buffer->get_16();
    } else if (head == MSGPACK_FORMAT_INT_32) {
        if (buffer->get_size() - buffer->get_position() < 4) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for INT_32!";
            return nullptr;
        }
        return buffer->get_32();
    } else if (head == MSGPACK_FORMAT_INT_64) {
        if (buffer->get_size() - buffer->get_position() < 8) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for INT_64!";
            return nullptr;
        }
        return buffer->get_64();
    } else if (head == MSGPACK_FORMAT_FLOAT_32) {
        if (buffer->get_size() - buffer->get_position() < 4) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for FLOAT_32!";
            return nullptr;
        }
        return buffer->get_float();
    } else if (head == MSGPACK_FORMAT_FLOAT_64) {
        if (buffer->get_size() - buffer->get_position() < 4) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for FLOAT_64!";
            return nullptr;
        }
        return buffer->get_double();
    } else if (((~head) & MSGPACK_FORMAT_FIXSTR) == 0) {
        int32_t size = head & 0x1f;
        if (buffer->get_size() - buffer->get_position() < size) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for FIXSTR!";
            return nullptr;
        }
        return buffer->get_utf8_string(size);
    } else if (head == MSGPACK_FORMAT_STR_8) {
        if (buffer->get_size() - buffer->get_position() < 1) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for STR_8 size!";
            return nullptr;
        }
        int32_t size = buffer->get_u8();
        if (buffer->get_size() - buffer->get_position() < size) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for STR_8!";
            return nullptr;
        }
        return buffer->get_utf8_string(size);
    } else if (head == MSGPACK_FORMAT_STR_16) {
        if (buffer->get_size() - buffer->get_position() < 2) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for STR_16 size!";
            return nullptr;
        }
        int32_t size = buffer->get_u16();
        if (buffer->get_size() - buffer->get_position() < size) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for STR_16!";
            return nullptr;
        }
        return buffer->get_utf8_string(size);
    } else if (head == MSGPACK_FORMAT_STR_32) {
        if (buffer->get_size() - buffer->get_position() < 4) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for STR_32 size!";
            return nullptr;
        }
        int64_t size = buffer->get_u32();
        if (buffer->get_size() - buffer->get_position() < size) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for STR_32!";
            return nullptr;
        }
        return buffer->get_utf8_string(int32_t(size));
    } else if (head == MSGPACK_FORMAT_BIN_8) {
        if (buffer->get_size() - buffer->get_position() < 1) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for BIN_8 size!";
            return nullptr;
        }
        int32_t size = buffer->get_u8();
        if (buffer->get_size() - buffer->get_position() < size) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for BIN_8!";
            return nullptr;
        }
        Array res = buffer->get_data(size);
//        assert(int(res[0]) == Error::OK);
        return res[1];
    } else if (head == MSGPACK_FORMAT_BIN_16) {
        if (buffer->get_size() - buffer->get_position() < 2) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for BIN_16 size";
            return nullptr;
        }
        int32_t size = buffer->get_u16();
        if (buffer->get_size() - buffer->get_position() < size) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for BIN_16!";
            return nullptr;
        }
        Array res = buffer->get_data(size);
//        assert(int(res[0]) == Error::OK);
        return res[1];
    } else if (head == MSGPACK_FORMAT_BIN_32) {
        if (buffer->get_size() - buffer->get_position() < 4) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for BIN_32 size!";
            return nullptr;
        }
        int64_t size = buffer->get_u32();
        if (buffer->get_size() - buffer->get_position() < size) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for BIN_32!";
            return nullptr;
        }
        Array res = buffer->get_data(int32_t(size));
//        assert(res[0] == OK);
        return res[1];
    } else if ((head & 0xF0) == MSGPACK_FORMAT_FIXARRAY) {
        int32_t size = head & 0x0f;
        Array res;
        for (int i = 0; i < size; i++) {
            res.append(_unpack(buffer, error));
            if (int(error.get("error", Error::FAILED)) != Error::OK) {
                return nullptr;
            }
        }
        return res;
    } else if (head == MSGPACK_FORMAT_ARRAY_16) {
        if (buffer->get_size() - buffer->get_position() < 2) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for ARRAY_16 size!";
            return nullptr;
        }
        int32_t size = buffer->get_u16();
        Array res;
        for (int i = 0; i < size; i++) {
            res.append(_unpack(buffer, error));
            if (int(error.get("error", Error::FAILED)) != Error::OK) {
                return nullptr;
            }
        }
        return res;
    } else if (head == MSGPACK_FORMAT_ARRAY_32) {
        if (buffer->get_size() - buffer->get_position() < 4) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for ARRAY_32 size!";
            return nullptr;
        }
        int64_t size = buffer->get_u32();
        Array res;
        for (int i = 0; i < size; i++) {
            res.append(_unpack(buffer, error));
            if (int(error.get("error", Error::FAILED)) != Error::OK) {
                return nullptr;
            }
        }
        return res;
    } else if ((head & 0xF0) == MSGPACK_FORMAT_FIXMAP) {
        int32_t size = head & 0x0f;
        Dictionary res;
        for (int i = 0; i < size; i++) {
            Variant k = _unpack(buffer, error);
            if (int(error.get("error", Error::FAILED)) != Error::OK) {
                return nullptr;
            }
            Variant v = _unpack(buffer, error);
            if (int(error.get("error", Error::FAILED)) != Error::OK) {
                return nullptr;
            }
            res[k] = v;
        }
        return res;
    } else if (head == MSGPACK_FORMAT_MAP_16) {
        if (buffer->get_size() - buffer->get_position() < 2) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for MAP16 size!";
            return nullptr;
        }
        int32_t size = buffer->get_u16();
        Dictionary res;
        for (int i = 0; i < size; i++) {
            Variant k = _unpack(buffer, error);
            if (int(error.get("error", Error::FAILED)) != Error::OK) {
                return nullptr;
            }
            Variant v = _unpack(buffer, error);
            if (int(error.get("error", Error::FAILED)) != Error::OK) {
                return nullptr;
            }
            res[k] = v;
        }
        return res;
    } else if (head == MSGPACK_FORMAT_MAP_32) {
        if (buffer->get_size() - buffer->get_position() < 4) {
            error["error"] = Error::FAILED;
            error["error_message"] = "Not enough buffer for MAP_32 size!";
            return nullptr;
        }
        int64_t size = buffer->get_u32();
        Dictionary res = {};
        for (int i = 0; i < size; i++) {
            Variant k = _unpack(buffer, error);
            if (int(error.get("error", Error::FAILED)) != Error::OK) {
                return nullptr;
            }
            Variant v = _unpack(buffer, error);
            if (int(error.get("error", Error::FAILED)) != Error::OK) {
                return nullptr;
            }
            res[k] = v;
        }
        return res;
    } else {
        error["error"] = Error::FAILED;
        error["error_message"] = "Invalid byte tag!";
        return nullptr;
    }
}
