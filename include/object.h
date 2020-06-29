#ifndef C_OBJECT_OBJECT_H
#define C_OBJECT_OBJECT_H

#include <stdbool.h>
#include <stddef.h>

// Typedef to represent a virtual method table.
// Also used as a type handle throughout the library.
typedef void** VTable;

// Represents a reference counted heap object.
typedef struct Object {
    VTable vtable;
    int ref_count;
} Object;

enum {
    OBJECT_PARENT,
    // Points to a list of interface implementation VTables (VTable*).

    OBJECT_INTERFACES,
    // The name of the "class" (char*).
    OBJECT_NAME,

    // virtual void free(Object* obj);
    // If overridden, make sure to call the base implementation (OBJ_BASE_CALL).
    OBJECT_METHOD_FREE,
    OBJECT_METHOD_MAX
};

extern VTable object_vtable;

// Initializes an object. Usually called from a child types constructor.
void object_init(Object* obj, VTable vtable);

// Frees an object. Be careful when calling this manually and bypassing the reference counter.
void object_free(Object* obj);

// Causes one VTable to inherit another.
void object_inherit(VTable parent_vtable, VTable child_vtable, unsigned int parent_max);

// Registers interface implementations with a VTable.
void object_implement(VTable type, VTable interfaces[]);

// Determines if an object is a specific type. Can be concrete or interface types.
bool object_is(Object* obj, VTable type);

static inline void object_ref_inc(Object* obj) {
    obj->ref_count++;
}

static inline void object_ref_dec(Object* obj) {
    if(--obj->ref_count <= 0)
        object_free(obj);
}

// A value that represents the concrete implementation of an interface.
typedef struct Interface {
    // The concrete vtable implementation for an interface.
    VTable vtable;

    // The instance that implements the interface.
    Object* obj;
} Interface;

enum {
    // Reserved, must always be NULL in the concrete VTable.
    INTERFACE_RESERVED,

    // Points to the interface definition VTable.
    INTERFACE_PARENT,

    // The name of the interface (char*).
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

// Designates a VTable as an implementation of a specific interface.
void interface_implementation(VTable interface, VTable implementation);

// Attempts to get an interface implementation from a concrete VTable.
//
//      type - The concrete type to get the interface implementation from.
// interface - The interface definition VTable, NOT the concrete implementation.
VTable type_get_interface(VTable type, VTable interface);

// Helper macro to cast a child type to an Object.
#define OBJ_AS_OBJ(value) ((Object*)(value))

// Helper macro to make it clear when a cast refers to an inheritance cast.
#define OBJ_AS(value, type) ((type)(value))

// Helper macro that can be used when downcasting a type to make sure the cast is legal.
// Results in NULL if the cast was illegal.
#define OBJ_CHECKED_AS(value, type) (object_is(value, type) ? (type)(value) : NULL)

// Converts an interface instance to the object instace that created the interface.
#define INTERFACE_AS_OBJ(value, type) ((type)interface_obj((Interface*)value))

// Converts an object to an interface that it implements.
// Only works when defining interfaces with INTERFACE_H and INTERFACE_C.
#define OBJ_AS_INTERFACE(value, type) ((type){ type_get_interface(*(VTable*)value, type ## _vtable), OBJ_AS_OBJ(value) })

// Gets a pointer to the function that implements a virtual method.
#define OBJ_VIRT_METHOD(obj, method) \
    ((*(VTable*)obj)[method])

// Gets a pointer to a function that implements a virtual method from a vtable.
// Usually used to get a base method implementation from a child type.
#define OBJ_BASE_METHOD(vtable, method) \
    (((VTable)vtable[OBJECT_PARENT])[method])

// Calls the concrete function of a virtual method.
//
//    obj - The object calling the method.
//    sig - The function signature in C syntax (e.g. for object_free: void (*)(Object*) ).
// method - The virtual method slot (e.g. for object_free: OBJECT_METHOD_FREE).
//    ... - The arguments used to call the method. Usually at least the calling object.
#define OBJ_VIRT_CALL(obj, sig, method, ...) \
    (((sig)OBJ_VIRT_METHOD(obj, method))(__VA_ARGS__))

// Calls the concrete function of a virtual method from a vtable.
// Usually used to call a base method from a child type.
//
// vtable - The vtable to get the method implementation from.
//    sig - The function signature in C syntax (e.g. for object_free: void (*)(Object*) ).
// method - The virtual method slot (e.g. for object_free: OBJECT_METHOD_FREE).
//    ... - The arguments used to call the method. Usually at least the calling object.
#define OBJ_BASE_CALL(vtable, sig, method, ...) \
    (((sig)OBJ_BASE_METHOD(vtable, method))(__VA_ARGS__))

// Gets the concrete type (VTable) from an object or interface.
#define OBJ_TYPEOF(value) (((*(VTable*)(value))[OBJECT_PARENT] == NULL && (*(VTable*)(value)) != object_vtable) ? (*(VTable*)(((Interface*)(value))->obj)) : (*(VTable*)(value)))

// Gets the concrete type name from an object or interface.
#define OBJ_NAMEOF(value) ((char*)(OBJ_TYPEOF(value)[OBJECT_NAME]))

// Helper macro to indicate that a type inherits another. 
// Must be the first item in a struct definition to work properply.
#define OBJ_INHERIT(type) type ___ ## type ## _base

// Defines a new interface type.
#define INTERFACE_H(name) \
    typedef struct name { \
        Interface base; \
    } name; \
 \
    extern VTable name ## _vtable; \

// Adds some implementation details to an interface type.
#define INTERFACE_C(name, method_slots) \
    void* name ## _vtable_impl[method_slots] = { [INTERFACE_NAME] = # name }; \
    VTable name ## _vtable = name ## _vtable_impl;

#endif