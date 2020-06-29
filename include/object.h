#ifndef C_OBJECT_OBJECT_H
#define C_OBJECT_OBJECT_H

#include <stdbool.h>
#include <stddef.h>

typedef void** VTable;

typedef struct Object {
    VTable vtable;
    int ref_count;
} Object;

enum {
    OBJECT_PARENT,
    OBJECT_INTERFACES,
    OBJECT_NAME,
    OBJECT_METHOD_FREE,
    OBJECT_METHOD_MAX
};

extern VTable object_vtable;

void object_init(Object* obj, VTable vtable);
void object_free(Object* obj);
void object_inherit(VTable parent_vtable, VTable child_vtable, unsigned int parent_max);
void object_implement(VTable type, VTable interfaces[]);
bool object_is(Object* obj, VTable type);

static inline void object_ref_inc(Object* obj) {
    obj->ref_count++;
}

static inline void object_ref_dec(Object* obj) {
    if(--obj->ref_count <= 0)
        object_free(obj);
}

typedef struct Interface {
    VTable vtable;
    Object* obj;
} Interface;

enum {
    INTERFACE_RESERVED,
    INTERFACE_PARENT,
    INTERFACE_NAME,
    INTERFACE_METHOD_MAX
};

static inline Object* interface_obj(Interface* interface) {
    return interface->obj;
}

static inline void interface_ref_inc(Interface* interface) {
    object_ref_inc(interface->obj);
}

static inline void interface_ref_dec(Interface* interface) {
    object_ref_dec(interface->obj);
}

void interface_implementation(VTable interface, VTable implementation);
VTable type_get_interface(VTable type, VTable interface);

#define OBJ_AS_OBJ(value) ((Object*)(value))
#define OBJ_AS(value, type) ((type)(value))
#define OBJ_CHECKED_AS(value, type) (object_is(value, type) ? (type)(value) : NULL)
#define INTERFACE_AS_OBJ(value, type) ((type)interface_obj((Interface*)value))
#define OBJ_AS_INTERFACE(value, type) ((type){ type_get_interface(*(VTable*)value, type ## _vtable), OBJ_AS_OBJ(value) })

#define OBJ_VIRT_METHOD(obj, method) \
    ((*(VTable*)obj)[method])

#define OBJ_BASE_METHOD(vtable, method) \
    (((VTable)vtable[OBJECT_PARENT])[method])

#define OBJ_VIRT_CALL(obj, sig, method, ...) \
    (((sig)OBJ_VIRT_METHOD(obj, method))(__VA_ARGS__))

#define OBJ_BASE_CALL(vtable, sig, method, ...) \
    (((sig)OBJ_BASE_METHOD(vtable, method))(__VA_ARGS__))

// Gets the concrete type from an object or interface.
#define OBJ_TYPEOF(value) (((*(VTable*)(value))[OBJECT_PARENT] == NULL && (*(VTable*)(value)) != object_vtable) ? (*(VTable*)(((Interface*)(value))->obj)) : (*(VTable*)(value)))

// Gets the concrete type name from an object or interface.
#define OBJ_NAMEOF(value) ((char*)(OBJ_TYPEOF(value)[OBJECT_NAME]))

#define OBJ_INHERIT(type) type ___ ## type ## _base

#define INTERFACE_H(name) \
    typedef struct name { \
        Interface base; \
    } name; \
 \
    extern VTable name ## _vtable; \

#define INTERFACE_C(name, method_slots) \
    void* name ## _vtable_impl[method_slots] = { [INTERFACE_NAME] = # name }; \
    VTable name ## _vtable = name ## _vtable_impl;

#endif