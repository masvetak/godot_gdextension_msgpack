#include "register_types.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/classes/engine.hpp>

#include "msgpack.hpp"

using namespace godot;

static Msgpack *msgpack;

void initialize_msgpack(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ClassDB::register_class<Msgpack>();

    msgpack = memnew(Msgpack);
    Engine::get_singleton()->register_singleton("Msgpack", Msgpack::get_singleton());
}

void uninitialize_msgpack(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_EDITOR) {
        return;
    }

    Engine::get_singleton()->unregister_singleton("Msgpack");
    memdelete(msgpack);
}

extern "C" {
    GDExtensionBool GDE_EXPORT msgpack_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_msgpack);
        init_obj.register_terminator(uninitialize_msgpack);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
