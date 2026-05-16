#pragma once
#include <string>
#include <il2cpp/il2cpp-headers.h>
#include <il2cpp/il2cpp-tabledefs.hpp>
#include <sstream>
#include <il2cpp/il2cpp-api-functions.h>

// creds to https://github.com/AndnixSH/Auto-Il2cppDumper/blob/master/app/src/main/jni/Il2Cpp/il2cpp_dump.cpp for some functions

std::string get_class_modifier(uint32_t flags) {
    std::stringstream ss;

    auto visibility = flags & TYPE_ATTRIBUTE_VISIBILITY_MASK;
    switch (visibility) {
        case TYPE_ATTRIBUTE_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_PUBLIC:
            ss << "public ";
            break;
        case TYPE_ATTRIBUTE_NOT_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_ASSEMBLY:
            ss << "internal ";
            break;
        case TYPE_ATTRIBUTE_NESTED_PRIVATE:
            ss << "private ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAMILY:
            ss << "protected ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM:
            ss << "private protected ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM:
            ss << "protected internal ";
            break;
    }

    if ( (flags & TYPE_ATTRIBUTE_ABSTRACT) && (flags & TYPE_ATTRIBUTE_SEALED) && !(flags & TYPE_ATTRIBUTE_INTERFACE)) {
        ss << "static ";
    } else {
        if ((flags & TYPE_ATTRIBUTE_ABSTRACT) && !(flags & TYPE_ATTRIBUTE_INTERFACE)) {
            ss << "abstract ";
        }
        if ((flags & TYPE_ATTRIBUTE_SEALED) && !(flags & TYPE_ATTRIBUTE_INTERFACE)) {
            ss << "sealed ";
        }
    }

    return ss.str();
}

std::string get_class_type_keyword(Il2CppClass *klass) {
    if (!klass) {
        return "class";
    }

    uint32_t flags = il2cpp_class_get_flags(klass);

    if (flags & TYPE_ATTRIBUTE_INTERFACE) {
        return "interface";
    }

    if (il2cpp_class_is_enum(klass)) {
        return "enum";
    }

    if (il2cpp_class_is_valuetype(klass)) {
        return "struct";
    }

    return "class";
}

std::string get_method_access_for_prop(uint32_t flags) {
    auto access = flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK;
    switch (access) {
        case METHOD_ATTRIBUTE_PRIVATE:
            return "private ";
            break;
        case METHOD_ATTRIBUTE_PUBLIC:
            return "";
            break;
        case METHOD_ATTRIBUTE_FAMILY:
            return "protected ";
            break;
        case METHOD_ATTRIBUTE_ASSEM:
        case METHOD_ATTRIBUTE_FAM_AND_ASSEM:
            return "internal ";
            break;
        case METHOD_ATTRIBUTE_FAM_OR_ASSEM:
            return "protected internal ";
            break;
    }
    return "";
}

std::string get_method_modifier(uint32_t flags) {
    std::stringstream ss;
    auto access = flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK;
    switch (access) {
        case METHOD_ATTRIBUTE_PRIVATE:
            ss << "private ";
            break;
        case METHOD_ATTRIBUTE_PUBLIC:
            ss << "public ";
            break;
        case METHOD_ATTRIBUTE_FAMILY:
            ss << "protected ";
            break;
        case METHOD_ATTRIBUTE_ASSEM:
        case METHOD_ATTRIBUTE_FAM_AND_ASSEM:
            ss << "internal ";
            break;
        case METHOD_ATTRIBUTE_FAM_OR_ASSEM:
            ss << "protected internal ";
            break;
    }
    if (flags & METHOD_ATTRIBUTE_STATIC) {
        ss << "static ";
    }
    if (flags & METHOD_ATTRIBUTE_ABSTRACT) {
        ss << "abstract ";
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT) {
            ss << "override ";
        }
    } else if (flags & METHOD_ATTRIBUTE_FINAL) {
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT) {
            ss << "sealed override ";
        }
    } else if (flags & METHOD_ATTRIBUTE_VIRTUAL) {
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_NEW_SLOT) {
            ss << "virtual ";
        } else {
            ss << "override ";
        }
    }
    if (flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) {
        ss << "extern ";
    }
    return ss.str();
}

std::string get_field_modifier(uint32_t attrs) {
    std::stringstream ss;
    auto access = attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK;
    switch (access) {
        case FIELD_ATTRIBUTE_PRIVATE:
            ss << "private ";
            break;
        case FIELD_ATTRIBUTE_PUBLIC:
            ss << "public ";
            break;
        case FIELD_ATTRIBUTE_FAMILY:
            ss << "protected ";
            break;
        case FIELD_ATTRIBUTE_ASSEMBLY:
        case FIELD_ATTRIBUTE_FAM_AND_ASSEM:
            ss << "internal ";
            break;
        case FIELD_ATTRIBUTE_FAM_OR_ASSEM:
            ss << "protected internal ";
            break;
    }
    if (attrs & FIELD_ATTRIBUTE_LITERAL) {
        ss << "const ";
    } else {
        if (attrs & FIELD_ATTRIBUTE_STATIC) {
            ss << "static ";
        }
        if (attrs & FIELD_ATTRIBUTE_INIT_ONLY) {
            ss << "readonly ";
        }
    }
    return ss.str();
}

bool _il2cpp_type_is_byref(const Il2CppType *type) {
    auto byref = type->byref;
    if (il2cpp_type_is_byref) {
        byref = il2cpp_type_is_byref(type);
    }
    return byref;
}