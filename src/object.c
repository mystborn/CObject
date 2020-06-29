#include "../include/object.h"

#include <stdlib.h>

static inline void object_free_impl(Object* obj) {
    free(obj);
}

void object_init(Object* obj, VTable vtable) {
    obj->ref_count = 0;
    obj->vtable = vtable;
}

void object_free(Object* obj) {
    if(OBJ_VIRT_METHOD(obj, OBJECT_METHOD_FREE) == object_free_impl)
        object_free_impl(obj);
    else
        OBJ_VIRT_CALL(obj, void (*)(Object*), OBJECT_METHOD_FREE, obj);
}

void object_inherit(VTable parent_vtable, VTable child_vtable, unsigned int parent_max) {
    child_vtable[OBJECT_PARENT] = parent_vtable;
    for(int i = 2; i < parent_max; i++) {
        if(!child_vtable[i])
            child_vtable[i] = parent_vtable[i];
    }
}

void object_implement(VTable type, VTable interfaces[]) {
    if(interfaces == NULL)
        return;
    type[OBJECT_INTERFACES] = interfaces;
}

bool object_is(Object* obj, VTable type) {
    VTable table = obj->vtable;

    // Check if the type is an interface. If it is, check against
    // the interface list on each VTable level.
    // Otherwise, walk up the inheritance tree normally.
    if(type[OBJECT_PARENT] == NULL && type != object_vtable) {
        return type_get_interface(table, type) != NULL;
    } else {
        do {
            if(table == type)
                return true;
            table = (VTable)table[OBJECT_PARENT];
        }
        while(table != NULL);
    }

    return false;
}

void interface_implementation(VTable interface, VTable implementation) {
    implementation[INTERFACE_PARENT] = interface;
    implementation[INTERFACE_NAME] = interface[INTERFACE_NAME];
}

VTable type_get_interface(VTable type, VTable interface) {
    do {
        VTable* interfaces = type[OBJECT_INTERFACES];
        while(interfaces != NULL && *interfaces != NULL) {
            if((*interfaces)[INTERFACE_PARENT] == interface)
                return *interfaces;
            interfaces++;
        }
        type = (VTable)type[OBJECT_PARENT];
    }
    while(type != NULL);

    return NULL;
}

static void* object_vtable_impl[OBJECT_METHOD_MAX] = {
    [OBJECT_PARENT] = NULL,
    [OBJECT_NAME] = "Object",
    [OBJECT_METHOD_FREE] = object_free_impl
};

VTable object_vtable = object_vtable_impl;