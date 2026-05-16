#pragma once
#include <globals.hpp>
#include <jni.h>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <limits>
#include <dlfcn.h>

#include <string-utils.h>
#include <nlohmann/json.hpp>
#include <KittyInclude.hpp>
#include <il2cpp/il2cpp-headers.h>
#include "il2cpp/il2cpp-tabledefs.hpp"
#include <il2cpp/il2cpp-api-functions.h>
#include <il2cpp/il2cpp-exports.hpp>
#include <il2cpp/ext/modifier-util.hpp>
using namespace nlohmann;

class ScaffoldGen {
private:
    static inline const Il2CppType* getMethodParameterType(const MethodInfo* method, int index) {
        if (!method || index < 0 || index >= method->parameters_count) return nullptr;
        if (!method->parameters) return nullptr;
        return method->parameters[index];
    }

    static inline int getMethodParameterCount(const MethodInfo* method) {
        if (!method || method->parameters_count <= 0) return 0;
        if (!method->parameters) return 0;
        return method->parameters_count;
    }

    static inline std::string getTypeFullName(Il2CppClass* klass) {
        if (!klass) return "void";

        std::stringstream ss;
        if (klass->namespaze && strcmp(klass->namespaze, "") != 0) {
            ss << klass->namespaze << ".";
        }

        std::vector<std::string> chain;
        auto cur = klass;
        while (cur) {
            chain.emplace_back(cur->name ? cur->name : "unnamed");
            cur = cur->declaringType;
        }
        std::reverse(chain.begin(), chain.end());
        for (size_t i = 0; i < chain.size(); ++i) {
            ss << chain[i];
            if (i + 1 < chain.size()) ss << "/";
        }

        return ss.str();
    }

    static inline std::string getTypeFullName(const Il2CppClass* klass) {
        return getTypeFullName(const_cast<Il2CppClass*>(klass));
    }

    static inline std::string getTypeKind(const Il2CppType* type) {
        if (!type) return "unknown";
        switch (type->type) {
            case IL2CPP_TYPE_END: return "end";
            case IL2CPP_TYPE_VOID: return "void";
            case IL2CPP_TYPE_BOOLEAN: return "bool";
            case IL2CPP_TYPE_CHAR: return "char";
            case IL2CPP_TYPE_I1: return "i1";
            case IL2CPP_TYPE_U1: return "u1";
            case IL2CPP_TYPE_I2: return "i2";
            case IL2CPP_TYPE_U2: return "u2";
            case IL2CPP_TYPE_I4: return "i4";
            case IL2CPP_TYPE_U4: return "u4";
            case IL2CPP_TYPE_I8: return "i8";
            case IL2CPP_TYPE_U8: return "u8";
            case IL2CPP_TYPE_R4: return "r4";
            case IL2CPP_TYPE_R8: return "r8";
            case IL2CPP_TYPE_STRING: return "string";
            case IL2CPP_TYPE_PTR: return "ptr";
            case IL2CPP_TYPE_BYREF: return "byref";
            case IL2CPP_TYPE_VALUETYPE: return "valuetype";
            case IL2CPP_TYPE_CLASS: return "class";
            case IL2CPP_TYPE_VAR: return "var";
            case IL2CPP_TYPE_ARRAY: return "array";
            case IL2CPP_TYPE_GENERICINST: return "genericinst";
            case IL2CPP_TYPE_TYPEDBYREF: return "typedbyref";
            case IL2CPP_TYPE_I: return "nativeint";
            case IL2CPP_TYPE_U: return "nativeuint";
            case IL2CPP_TYPE_FNPTR: return "fnptr";
            case IL2CPP_TYPE_OBJECT: return "object";
            case IL2CPP_TYPE_SZARRAY: return "szarray";
            case IL2CPP_TYPE_MVAR: return "mvar";
            case IL2CPP_TYPE_CMOD_REQD: return "cmod_reqd";
            case IL2CPP_TYPE_CMOD_OPT: return "cmod_opt";
            case IL2CPP_TYPE_INTERNAL: return "internal";
            case IL2CPP_TYPE_MODIFIER: return "modifier";
            case IL2CPP_TYPE_SENTINEL: return "sentinel";
            case IL2CPP_TYPE_PINNED: return "pinned";
            case IL2CPP_TYPE_ENUM: return "enum";
            default: return "unknown";
        }
    }

    static inline std::string getTypeDisplayName(const Il2CppType* type) {
        if (!type) return "void";
        auto klass = shouldResolveClassForType(type) ? il2cpp_class_from_type(type) : nullptr;
        if (klass) return getTypeFullName(klass);
        return getTypeKind(type);
    }

    static inline bool shouldResolveClassForType(const Il2CppType* type) {
        if (!type) return false;
        switch (type->type) {
            case IL2CPP_TYPE_CLASS:
            case IL2CPP_TYPE_VALUETYPE:
            case IL2CPP_TYPE_OBJECT:
            case IL2CPP_TYPE_STRING:
            case IL2CPP_TYPE_ARRAY:
            case IL2CPP_TYPE_SZARRAY:
            case IL2CPP_TYPE_GENERICINST:
                return true;
            default:
                return false;
        }
    }

    static inline int getGenericArityFromName(const char* name) {
        if (!name) return 0;
        std::string n(name);
        auto backtick = n.find('`');
        if (backtick == std::string::npos || backtick + 1 >= n.size()) return 0;
        try {
            return std::stoi(n.substr(backtick + 1));
        } catch (...) {
            return 0;
        }
    }

    static inline int nextMappedGenericIndex(const std::unordered_map<uint32_t, int>& indexMap) {
        int maxIndex = -1;
        for (const auto& it : indexMap) {
            if (it.second > maxIndex) maxIndex = it.second;
        }
        return maxIndex + 1;
    }

    static inline int normalizeGenericParamIndex(uint32_t raw,
                                                 std::unordered_map<uint32_t, int>* indexMap,
                                                 int fallback) {
        if (!indexMap) {
            if (raw < 1024U) return static_cast<int>(raw);
            return fallback;
        }

        auto it = indexMap->find(raw);
        if (it != indexMap->end()) return it->second;

        int mapped = 0;
        if (raw < 1024U) {
            mapped = static_cast<int>(raw);
        } else {
            mapped = nextMappedGenericIndex(*indexMap);
        }

        (*indexMap)[raw] = mapped;
        return mapped;
    }

    static inline json serializeType(const Il2CppType* type,
                                     std::unordered_map<uint32_t, int>* classGenericIndexMap = nullptr,
                                     std::unordered_map<uint32_t, int>* methodGenericIndexMap = nullptr) {
        json t;
        if (!type) {
            t["kind"] = "void";
            t["name"] = "void";
            return t;
        }

        t["kind"] = getTypeKind(type);
        t["type_enum"] = static_cast<int>(type->type);
        t["attrs"] = type->attrs;
        t["byref"] = static_cast<bool>(type->byref);
        t["pinned"] = static_cast<bool>(type->pinned);
        t["is_valuetype"] = static_cast<bool>(type->valuetype);
        t["name"] = getTypeDisplayName(type);

        if (type->type == IL2CPP_TYPE_VAR || type->type == IL2CPP_TYPE_MVAR) {
            auto raw = static_cast<uint32_t>(type->data.__genericParameterIndex);
            auto normalized = normalizeGenericParamIndex(raw,
                                                         type->type == IL2CPP_TYPE_VAR ? classGenericIndexMap : methodGenericIndexMap,
                                                         0);
            t["generic_param_index"] = normalized;
            t["generic_param_raw"] = raw;
            t["name"] = type->type == IL2CPP_TYPE_VAR
                        ? ("!" + std::to_string(normalized))
                        : ("!!" + std::to_string(normalized));
            return t;
        }

        auto klass = shouldResolveClassForType(type) ? il2cpp_class_from_type(type) : nullptr;
        if (klass) {
            t["full_name"] = getTypeFullName(klass);
            t["namespace"] = klass->namespaze ? klass->namespaze : "";
            t["class_name"] = klass->name ? klass->name : "";
            t["class_flags"] = il2cpp_class_get_flags(klass);
            t["class_token"] = il2cpp_class_get_type_token(klass);
            json classGeneric;
            classGeneric["is_generic_definition"] = il2cpp_class_is_generic(klass);
            classGeneric["is_inflated"] = klass->generic_class != nullptr;
            classGeneric["arity"] = getGenericArityFromName(klass->name);
            classGeneric["arguments"] = json::array();
            if (klass->generic_class && klass->generic_class->context.class_inst) {
                auto inst = klass->generic_class->context.class_inst;
                classGeneric["arity"] = inst->type_argc;
                for (uint32_t i = 0; i < inst->type_argc; ++i) {
                    classGeneric["arguments"].emplace_back(serializeType(inst->type_argv[i], classGenericIndexMap, methodGenericIndexMap));
                }
            }
            t["class_generic"] = classGeneric;
            t["is_enum"] = il2cpp_class_is_enum(klass);
        }

        if (type->type == IL2CPP_TYPE_PTR && type->data.type) {
            t["element_type"] = serializeType(type->data.type, classGenericIndexMap, methodGenericIndexMap);
        } else if (type->type == IL2CPP_TYPE_ARRAY) {
            int rank = 1;
            const Il2CppType* elementType = nullptr;
            if (type->data.array) {
                rank = static_cast<int>(type->data.array->rank);
                elementType = type->data.array->etype;
            }
            t["rank"] = rank;
            t["element_type"] = serializeType(elementType, classGenericIndexMap, methodGenericIndexMap);
        } else if (type->type == IL2CPP_TYPE_SZARRAY) {
            t["rank"] = 1;
            const Il2CppType* elementType = nullptr;
            if (klass) {
                auto elementKlass = il2cpp_class_get_element_class(klass);
                if (elementKlass) elementType = il2cpp_class_get_type(elementKlass);
            }
            t["element_type"] = serializeType(elementType, classGenericIndexMap, methodGenericIndexMap);
        }

        return t;
    }

    static inline json getGenericArrayFromInst(const Il2CppGenericInst* inst) {
        json arr = json::array();
        if (!inst) return arr;

        for (uint32_t i = 0; i < inst->type_argc; ++i) {
            arr.emplace_back(serializeType(inst->type_argv[i]));
        }
        return arr;
    }

    static inline json getClassGenericInfo(const Il2CppClass* klass) {
        json g;
        g["is_generic_definition"] = klass && il2cpp_class_is_generic(klass);
        g["is_inflated"] = klass && klass->generic_class != nullptr;
        g["arity"] = klass ? getGenericArityFromName(klass->name) : 0;
        g["arguments"] = json::array();

        if (klass && klass->generic_class && klass->generic_class->context.class_inst) {
            g["arguments"] = getGenericArrayFromInst(klass->generic_class->context.class_inst);
            g["arity"] = klass->generic_class->context.class_inst->type_argc;
        }

        return g;
    }

    static inline json getMethodGenericInfo(const MethodInfo* method, int inferredArity = -1) {
        json g;
        g["is_generic_definition"] = method && il2cpp_method_is_generic(method);
        g["is_inflated"] = method && il2cpp_method_is_inflated(method);
        g["arity"] = method ? getGenericArityFromName(method->name) : 0;
        g["arguments"] = json::array();

        if (method && method->is_inflated && method->genericMethod && method->genericMethod->context.method_inst) {
            g["arguments"] = getGenericArrayFromInst(method->genericMethod->context.method_inst);
            g["arity"] = method->genericMethod->context.method_inst->type_argc;
        }

        if (inferredArity > g["arity"].get<int>()) {
            g["arity"] = inferredArity;
        }

        return g;
    }

    static inline json getLiteralFieldValue(FieldInfo* field, const Il2CppType* overrideType = nullptr) {
        json val = nullptr;
        if (!field) return val;

        auto flags = il2cpp_field_get_flags(field);
        if (!(flags & FIELD_ATTRIBUTE_LITERAL)) return val;

        auto readType = overrideType ? overrideType : field->type;
        if (!readType) return val;

        switch (readType->type) {
            case IL2CPP_TYPE_BOOLEAN: {
                bool v = false;
                il2cpp_field_static_get_value(field, &v);
                val = v;
                break;
            }
            case IL2CPP_TYPE_I1: {
                int8_t v = 0;
                il2cpp_field_static_get_value(field, &v);
                val = static_cast<int32_t>(v);
                break;
            }
            case IL2CPP_TYPE_U1: {
                uint8_t v = 0;
                il2cpp_field_static_get_value(field, &v);
                val = static_cast<uint32_t>(v);
                break;
            }
            case IL2CPP_TYPE_I2: {
                int16_t v = 0;
                il2cpp_field_static_get_value(field, &v);
                val = static_cast<int32_t>(v);
                break;
            }
            case IL2CPP_TYPE_U2: {
                uint16_t v = 0;
                il2cpp_field_static_get_value(field, &v);
                val = static_cast<uint32_t>(v);
                break;
            }
            case IL2CPP_TYPE_I4: {
                int32_t v = 0;
                il2cpp_field_static_get_value(field, &v);
                val = v;
                break;
            }
            case IL2CPP_TYPE_U4: {
                uint32_t v = 0;
                il2cpp_field_static_get_value(field, &v);
                val = v;
                break;
            }
            case IL2CPP_TYPE_I8: {
                int64_t v = 0;
                il2cpp_field_static_get_value(field, &v);
                val = v;
                break;
            }
            case IL2CPP_TYPE_U8: {
                uint64_t v = 0;
                il2cpp_field_static_get_value(field, &v);
                if (v <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
                    val = static_cast<int64_t>(v);
                } else {
                    val = std::to_string(v);
                }
                break;
            }
            case IL2CPP_TYPE_R4: {
                float v = 0.0f;
                il2cpp_field_static_get_value(field, &v);
                val = v;
                break;
            }
            case IL2CPP_TYPE_R8: {
                double v = 0.0;
                il2cpp_field_static_get_value(field, &v);
                val = v;
                break;
            }
            case IL2CPP_TYPE_CHAR: {
                uint16_t v = 0;
                il2cpp_field_static_get_value(field, &v);
                val = static_cast<uint32_t>(v);
                break;
            }
            default:
                break;
        }

        return val;
    }

    static inline std::string getMethodSignatureKey(const MethodInfo* method) {
        if (!method) return "";

        std::stringstream ss;
        auto klass = il2cpp_method_get_class(method);
        ss << getTypeFullName(klass) << "::" << (method->name ? method->name : "unnamed") << "(";
        int paramCount = getMethodParameterCount(method);
        for (int i = 0; i < paramCount; ++i) {
            ss << getTypeDisplayName(getMethodParameterType(method, i));
            if (i < paramCount - 1) ss << ",";
        }
        ss << ")->" << getTypeDisplayName(method->return_type);
        return ss.str();
    }

    static inline json getGenericArray(const Il2CppClass* klass) {
        if (!klass || !klass->generic_class || !klass->generic_class->context.class_inst) {
            return json::array();
        }

        return getGenericArrayFromInst(klass->generic_class->context.class_inst);
    }


    static inline void Dump(std::ofstream& out) {
        if (!out.is_open()) return;
        LOGI("[ScaffoldGen] Dump start");

        auto domain = il2cpp_domain_get();
        if (!domain) {
            LOGI("Failed to generate scaffold.json: il2cpp domain is null");
            return;
        }
        LOGI("[ScaffoldGen] Domain ready");

        size_t assemblyCount;
        auto assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);
        if (!assemblies) {
            LOGI("Failed to generate scaffold.json: assemblies are null");
            return;
        }
        LOGI("[ScaffoldGen] Assemblies loaded: %zu", assemblyCount);

        out << "{\n";
        out << "  \"schema_version\": 1,\n";
        out << "  \"package\": " << json(GetPackageName()).dump() << ",\n";
        out << "  \"images\": [\n";

        bool firstImage = true;

        for (size_t assemblyIndex = 0; assemblyIndex < assemblyCount; assemblyIndex++) {
            LOGI("[ScaffoldGen] Assembly loop start: index=%zu", assemblyIndex);
            auto assembly = assemblies[assemblyIndex];
            auto image = il2cpp_assembly_get_image(assembly);
            if (!image) continue;

            json imageDump;
            imageDump["index"] = assemblyIndex;
            auto imageName = il2cpp_image_get_name(image);
            imageDump["name"] = imageName ? imageName : "";
            imageDump["assembly"] = assembly && assembly->aname.name ? assembly->aname.name : "";
            imageDump["classes"] = json::array();
            LOGI("[ScaffoldGen] Image ready: index=%zu name=%s", assemblyIndex, imageName ? imageName : "<null>");

            for (int classIndex = 0; classIndex < il2cpp_image_get_class_count(image); classIndex++) {
                LOGI("[ScaffoldGen] Class loop start: image=%zu class=%d", assemblyIndex, classIndex);
                auto klass = const_cast<Il2CppClass*>(il2cpp_image_get_class(image, classIndex));
                if (!klass) continue;

                json classDump;
                classDump["index"] = classIndex;
                classDump["namespace"] = klass->namespaze ? klass->namespaze : "";
                classDump["class_name"] = klass->name ? klass->name : "";
                classDump["name"] = getTypeFullName(klass);
                classDump["full_name"] = getTypeFullName(klass);

                bool isVT = il2cpp_class_is_valuetype(klass);
                bool isEnum = il2cpp_class_is_enum(klass);
                bool isInterface = (il2cpp_class_get_flags(klass) & TYPE_ATTRIBUTE_INTERFACE) != 0;
                std::string classType = "class";
                if (isInterface)
                    classType = "interface";
                else if (isVT && !isEnum)
                    classType = "struct";
                else if (isEnum)
                    classType = "enum";

                classDump["kind"] = classType;
                classDump["is_enum"] = isEnum;
                classDump["is_value_type"] = isVT;
                classDump["is_interface"] = isInterface;
                classDump["flags"] = il2cpp_class_get_flags(klass);
                classDump["token"] = il2cpp_class_get_type_token(klass);
                classDump["generic"] = getClassGenericInfo(klass);
                json extensions = json::array();

                if (klass->parent) {
                    extensions.emplace_back(getTypeFullName(klass->parent));
                }

                json interfaces = json::array();
                if (klass->interfaces_count > 0) {
                    void* interIter = nullptr;
                    while (auto inter = il2cpp_class_get_interfaces(const_cast<Il2CppClass*>(klass), &interIter)) {
                        extensions.emplace_back(getTypeFullName(inter));
                        interfaces.emplace_back(getTypeFullName(inter));
                    }
                }

                classDump["extensions"] = extensions;
                classDump["interfaces"] = interfaces;
                classDump["parent"] = klass->parent ? getTypeFullName(klass->parent) : "";
                classDump["type"] = serializeType(il2cpp_class_get_type(klass));
                classDump["enum_underlying_type"] = nullptr;

                const Il2CppType* enumUnderlyingType = nullptr;
                if (isEnum) {
                    void* enumFieldIter = nullptr;
                    while (auto enumField = il2cpp_class_get_fields(klass, &enumFieldIter)) {
                        if (enumField->name && strcmp(enumField->name, "value__") == 0) {
                            enumUnderlyingType = enumField->type;
                            classDump["enum_underlying_type"] = serializeType(enumField->type);
                            break;
                        }
                    }
                }

                json fieldsArray = json::array();
                void* fieldIter = nullptr;
                LOGI("[ScaffoldGen] Fields start: class=%s", classDump["name"].get<std::string>().c_str());
                while (auto field = il2cpp_class_get_fields(klass, &fieldIter)) {
                    json fieldDump;
                    fieldDump["name"] = field->name ? field->name : "";
                    fieldDump["flags"] = il2cpp_field_get_flags(field);
                    fieldDump["offset"] = field->offset;
                    fieldDump["token"] = field->token;
                    fieldDump["is_static"] = (il2cpp_field_get_flags(field) & FIELD_ATTRIBUTE_STATIC) != 0;
                    fieldDump["is_literal"] = (il2cpp_field_get_flags(field) & FIELD_ATTRIBUTE_LITERAL) != 0;
                    fieldDump["type"] = serializeType(field->type);
                    fieldDump["literal_value"] = getLiteralFieldValue(field, enumUnderlyingType);

                    if (isEnum && field->name && strcmp(field->name, "value__") == 0) {
                        classDump["enum_underlying_type"] = serializeType(field->type);
                    }

                    fieldsArray.emplace_back(fieldDump);
                }
                classDump["fields"] = fieldsArray;

                json methodsArray = json::array();
                void* methodIter = nullptr;
                LOGI("[ScaffoldGen] Methods start: class=%s", classDump["name"].get<std::string>().c_str());
                while (auto method = il2cpp_class_get_methods(klass, &methodIter)) {
                    std::unordered_map<uint32_t, int> classGenericIndexMap;
                    std::unordered_map<uint32_t, int> methodGenericIndexMap;
                    json methodDump;
                    methodDump["name"] = method->name ? method->name : "";
                    methodDump["flags"] = method->flags;
                    methodDump["iflags"] = method->iflags;
                    methodDump["slot"] = method->slot;
                    methodDump["token"] = method->token;
                    int paramCount = getMethodParameterCount(method);
                    methodDump["parameter_count"] = paramCount;
                    methodDump["is_static"] = (method->flags & METHOD_ATTRIBUTE_STATIC) != 0;
                    methodDump["is_abstract"] = (method->flags & METHOD_ATTRIBUTE_ABSTRACT) != 0;
                    methodDump["is_virtual"] = (method->flags & METHOD_ATTRIBUTE_VIRTUAL) != 0;
                    methodDump["return_type"] = serializeType(method->return_type, &classGenericIndexMap, &methodGenericIndexMap);

                    json parameters = json::array();
                    LOGI("[ScaffoldGen] Method start: class=%s method=%s", classDump["name"].get<std::string>().c_str(), method->name ? method->name : "<null>");
                    for (int p = 0; p < paramCount; ++p) {
                        json paramDump;
                        auto ptype = getMethodParameterType(method, p);
                        auto pname = ptype ? il2cpp_method_get_param_name(method, p) : nullptr;
                        paramDump["index"] = p;
                        paramDump["name"] = pname ? pname : "";
                        paramDump["type"] = serializeType(ptype, &classGenericIndexMap, &methodGenericIndexMap);
                        parameters.emplace_back(paramDump);
                    }
                    methodDump["generic"] = getMethodGenericInfo(method, nextMappedGenericIndex(methodGenericIndexMap));
                    methodDump["parameters"] = parameters;
                    methodDump["signature_key"] = getMethodSignatureKey(method);

                    uintptr_t methodPtr = reinterpret_cast<uintptr_t>(method->methodPointer);
                    methodDump["method_pointer"] = methodPtr;
                    methodDump["rva"] = (proc_map.isValid() && methodPtr > 0 && methodPtr >= proc_map.startAddress)
                            ? (methodPtr - proc_map.startAddress)
                            : 0;

                    methodsArray.emplace_back(methodDump);
                }
                classDump["methods"] = methodsArray;

                json propertiesArray = json::array();
                void* propIter = nullptr;
                LOGI("[ScaffoldGen] Properties start: class=%s", classDump["name"].get<std::string>().c_str());
                while (auto prop = il2cpp_class_get_properties(klass, &propIter)) {
                    json propDump;
                    propDump["name"] = prop->name ? prop->name : "";
                    propDump["flags"] = il2cpp_property_get_flags(const_cast<PropertyInfo*>(prop));
                    propDump["token"] = prop->token;
                    propDump["getter_token"] = prop->get ? prop->get->token : 0;
                    propDump["setter_token"] = prop->set ? prop->set->token : 0;
                    propDump["getter_signature_key"] = prop->get ? getMethodSignatureKey(prop->get) : "";
                    propDump["setter_signature_key"] = prop->set ? getMethodSignatureKey(prop->set) : "";
                    propDump["has_getter"] = prop->get != nullptr;
                    propDump["has_setter"] = prop->set != nullptr;

                    std::unordered_map<uint32_t, int> classGenericIndexMap;
                    std::unordered_map<uint32_t, int> methodGenericIndexMap;

                    const Il2CppType* propType = nullptr;
                    if (prop->get) {
                        propType = prop->get->return_type;
                    } else if (prop->set && getMethodParameterCount(prop->set) > 0) {
                        propType = getMethodParameterType(prop->set, getMethodParameterCount(prop->set) - 1);
                    }
                    propDump["type"] = serializeType(propType, &classGenericIndexMap, &methodGenericIndexMap);

                    json indexParams = json::array();
                    if (prop->get) {
                        int getterParamCount = getMethodParameterCount(prop->get);
                        for (int i = 0; i < getterParamCount; ++i) {
                            indexParams.emplace_back(serializeType(getMethodParameterType(prop->get, i), &classGenericIndexMap, &methodGenericIndexMap));
                        }
                    } else if (prop->set && getMethodParameterCount(prop->set) > 1) {
                        int setterParamCount = getMethodParameterCount(prop->set);
                        for (int i = 0; i < setterParamCount - 1; ++i) {
                            indexParams.emplace_back(serializeType(getMethodParameterType(prop->set, i), &classGenericIndexMap, &methodGenericIndexMap));
                        }
                    }
                    propDump["index_parameters"] = indexParams;

                    propertiesArray.emplace_back(propDump);
                    LOGI("[ScaffoldGen] Property done: class=%s property=%s", classDump["name"].get<std::string>().c_str(), prop->name ? prop->name : "<null>");
                }
                classDump["properties"] = propertiesArray;

                imageDump["classes"].emplace_back(classDump);
                LOGI("[ScaffoldGen] Class done: image=%zu class=%d name=%s", assemblyIndex, classIndex, classDump["name"].get<std::string>().c_str());
            }

            if (!firstImage) out << ",\n";
            out << imageDump.dump(2);
            firstImage = false;
            LOGI("[ScaffoldGen] Image done: index=%zu", assemblyIndex);
        }

        out << "\n  ]\n";
        out << "}\n";
        LOGI("[ScaffoldGen] Dump finished");
    }

public:
    static inline void Gen() {
        LOGI("Generating scaffold.json");
        auto domain = il2cpp_domain_get();
        if (!domain) {
            LOGI("Failed to generate scaffold.json: il2cpp domain is null");
            return;
        }
        LOGI("[ScaffoldGen] Attaching thread");
        il2cpp_thread_attach(domain);
        LOGI("[ScaffoldGen] Thread attached");

        std::string outputDir = "/storage/emulated/0/Android/data/" + GetPackageName() + "/dump";
        std::filesystem::create_directories(outputDir);
        std::string outputPath = outputDir + "/scaffold.json";
        std::ofstream out(outputPath);
        if (out.is_open()) {
            LOGI("[ScaffoldGen] Output file open: %s", outputPath.c_str());
            Dump(out);
            out.close();
            LOGI("Output scaffold.json at %s", outputPath.c_str());
        } else {
            LOGI("Failed to write scaffold.json at %s", outputPath.c_str());
        }
    }
};
