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
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
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

class StructGen {
private:
    static inline const std::string header = R"(
struct Il2CppClass_1 {
    void* image;
    void* gc_desc;
    const char* name;
    const char* namespaze;
    struct Il2CppType byval_arg;
    struct Il2CppType this_arg;
    struct Il2CppClass* element_class;
    struct Il2CppClass* castClass;
    struct Il2CppClass* declaringType;
    struct Il2CppClass* parent;
    void *generic_class;
    void* typeMetadataHandle;
    void* interopData;
    struct Il2CppClass* klass;
    void* fields;
    void* events;
    void* properties;
    void* methods;
    struct Il2CppClass** nestedTypes;
    struct Il2CppClass** implementedInterfaces;
    struct Il2CppRuntimeInterfaceOffsetPair* interfaceOffsets;
};

struct Il2CppClass_2 {
    struct Il2CppClass** typeHierarchy;
    void *unity_user_data;
    uint32_t initializationExceptionGCHandle;
    uint32_t cctor_started;
    uint32_t cctor_finished;
    size_t cctor_thread;
    void* genericContainerHandle;
    uint32_t instance_size;
    uint32_t actualSize;
    uint32_t element_size;
    int32_t native_size;
    uint32_t static_fields_size;
    uint32_t thread_static_fields_size;
    int32_t thread_static_fields_offset;
    uint32_t flags;
    uint32_t token;
    uint16_t method_count;
    uint16_t property_count;
    uint16_t field_count;
    uint16_t event_count;
    uint16_t nested_type_count;
    uint16_t vtable_count;
    uint16_t interfaces_count;
    uint16_t interface_offsets_count;
    uint8_t typeHierarchyDepth;
    uint8_t genericRecursionDepth;
    uint8_t rank;
    uint8_t minimumAlignment;
    uint8_t naturalAligment;
    uint8_t packingSize;
    uint8_t bitflags1;
    uint8_t bitflags2;
};

struct Il2CppClass {
    struct Il2CppClass_1 _1;
    void* static_fields;
    union Il2CppRGCTXData* rgctx_data;
    struct Il2CppClass_2 _2;
    struct VirtualInvokeData vtable[255];
};

struct MethodInfo {
    Il2CppMethodPointer methodPointer;
    Il2CppMethodPointer virtualMethodPointer;
    void* invoker_method;
    const char* name;
    struct Il2CppClass *klass;
    const struct Il2CppType *return_type;
    const struct Il2CppType** parameters;
    union {
        const union Il2CppRGCTXData* rgctx_data;
        const void* methodMetadataHandle;
    };
    union {
        const void* genericMethod;
        const void* genericContainerHandle;
    };
    uint32_t token;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint8_t parameters_count;
    uint8_t bitflags;
};)";
    static inline const std::string genericHeader = R"(
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef signed long long intptr_t;
typedef unsigned long long uintptr_t;
typedef unsigned long long size_t;
typedef uint8_t _BOOL;

typedef struct Il2CppClass Il2CppClass;
typedef struct Il2CppType Il2CppType;
typedef struct MethodInfo MethodInfo;
typedef struct Il2CppImage Il2CppImage;
typedef struct Il2CppArray Il2CppArray;
typedef struct Il2CppObject Il2CppObject;
typedef union Il2CppRGCTXData Il2CppRGCTXData;
typedef struct Il2CppClass_1 Il2CppClass_1;
typedef struct Il2CppClass_2 Il2CppClass_2;
typedef struct VirtualInvokeData VirtualInvokeData;
typedef struct Il2CppRuntimeInterfaceOffsetPair Il2CppRuntimeInterfaceOffsetPair;

typedef void(*Il2CppMethodPointer)(void);

struct VirtualInvokeData {
    Il2CppMethodPointer methodPtr;
    const struct MethodInfo* method;
};

struct Il2CppType {
    void* data;
    unsigned int bits;
};

struct Il2CppObject {
    struct Il2CppClass *klass;
    void *monitor;
};

union Il2CppRGCTXData {
    void* rgctxDataDummy;
    const struct MethodInfo* method;
    const struct Il2CppType* type;
    struct Il2CppClass* klass;
};

struct Il2CppRuntimeInterfaceOffsetPair {
    struct Il2CppClass* interfaceType;
    int32_t offset;
};
)";
    const std::unordered_set<std::string> Keywords = {
            "klass",     "monitor",   "register",  "_cs",
            "auto",      "friend",    "template",  "flat",
            "default",   "_ds",       "interrupt",  "unsigned",
            "signed",    "asm",       "if",        "case",
            "break",     "continue",  "do",        "new",
            "_",         "short",     "union",     "class",
            "namespace", "inline",    "near",      "far"};

    static inline std::stringstream headerStruct;
    static inline std::stringstream sb;

    static inline std::unordered_set<Il2CppClass*> alreadyParsed;
    static inline std::unordered_set<std::string> alreadyForwardDeclared;

    static inline char GetTypeChar(const Il2CppType* type) {
        if (!type) return 'i';
        if (type->byref) return 'i';
        switch (type->type) {
            case IL2CPP_TYPE_VOID: return 'v';
            case IL2CPP_TYPE_I8:
            case IL2CPP_TYPE_U8:   return 'j';
            case IL2CPP_TYPE_R4:   return 'f';
            case IL2CPP_TYPE_R8:   return 'd';
            default:               return 'i';
        }
    }

    static inline std::string GetMethodTypeSignature(const MethodInfo* method) {
        std::string sig = "";
        sig += GetTypeChar(method->return_type);
        if (!(method->flags & 0x0010)) { // METHOD_ATTRIBUTE_STATIC
            sig += 'i';
        }
        for (int i = 0; i < method->parameters_count; i++) {
            sig += GetTypeChar(method->parameters[i]);
        }
        sig += 'i';
        return sig;
    }

    static inline std::string fixName(const char* name) {
        if (!name || name[0] == '\0') return "unnamed";

        static const std::unordered_set<std::string> Keywords = {
                "klass", "monitor", "register", "_cs", "auto", "friend", "template",
                "flat", "default", "_ds", "interrupt", "unsigned", "signed", "asm",
                "if", "case", "break", "continue", "do", "new", "_", "short", "union",
                "class", "namespace", "inline", "near", "far"
        };
        static const std::string allowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_";
        static const std::string numberChars = "1234567890";

        std::string str = name;
        if (Keywords.find(str) != Keywords.end()) str = "_" + str;
        if (numberChars.find(str[0]) != std::string::npos) str = "_" + str;

        std::string newString = "";
        for (char c : str) {
            if (allowedChars.find(c) != std::string::npos) newString += c;
            else newString += "_";
        }
        return newString;
    }

    static inline std::string fixName(std::string name) {
        return fixName(name.c_str());
    }


    static inline std::string getRefName(Il2CppClass* klass) {
        std::string ret = fixName(std::string(klass->namespaze) + "." + std::string(klass->name));
        if (!il2cpp_class_is_valuetype(klass)) {
            ret = "struct " + ret + "*";
        }

        return ret;
    }

    static inline std::string getClassFixFullName(Il2CppClass* klass) {
        if (!klass) return "void";
        const char* ns = (klass->namespaze) ? klass->namespaze : "";
        const char* nm = (klass->name) ? klass->name : "unnamed";

        char buf[32];
        sprintf(buf, "_%p", (void*)klass);

        std::string full = std::string(ns) + "_" + std::string(nm) + buf;
        return fixName(full.c_str());
    }

    static inline std::string ParseType(const Il2CppType* type) {
        if (!type) return "void*";

        if (type->type == IL2CPP_TYPE_PTR) {
            return ParseType((Il2CppType*)type->data.type) + "*";
        }

        switch (type->type) {
            case IL2CPP_TYPE_VOID:    return "void";
            case IL2CPP_TYPE_BOOLEAN: return "_BOOL";
            case IL2CPP_TYPE_CHAR:    return "uint16_t";
            case IL2CPP_TYPE_I1:      return "int8_t";
            case IL2CPP_TYPE_U1:      return "uint8_t";
            case IL2CPP_TYPE_I2:      return "int16_t";
            case IL2CPP_TYPE_U2:      return "uint16_t";
            case IL2CPP_TYPE_I4:      return "int32_t";
            case IL2CPP_TYPE_U4:      return "uint32_t";
            case IL2CPP_TYPE_I8:      return "int64_t";
            case IL2CPP_TYPE_U8:      return "uint64_t";
            case IL2CPP_TYPE_R4:      return "float";
            case IL2CPP_TYPE_R8:      return "double";
            case IL2CPP_TYPE_I:       return "intptr_t";
            case IL2CPP_TYPE_U:       return "uintptr_t";
            case IL2CPP_TYPE_STRING:  return "struct System_String_o*";
            case IL2CPP_TYPE_OBJECT:  return "struct Il2CppObject*";
            case IL2CPP_TYPE_TYPEDBYREF: return "struct Il2CppObject*";

            case IL2CPP_TYPE_ARRAY:
            case IL2CPP_TYPE_SZARRAY:
                return "struct Il2CppArray*";

            case IL2CPP_TYPE_CLASS:
            case IL2CPP_TYPE_VALUETYPE:
            case IL2CPP_TYPE_GENERICINST: {
                Il2CppClass* klass = il2cpp_class_from_type(type);
                if (!klass) return "void*";

                if (il2cpp_class_is_enum(klass)) {
                    void* iter = nullptr;
                    FieldInfo* f = il2cpp_class_get_fields(klass, &iter);
                    if (f) return ParseType(f->type);
                }

                std::string name = getClassFixFullName(klass);

                if (il2cpp_class_is_valuetype(klass)) {
                    return "struct " + name + "_Fields";
                } else {
                    return "struct " + name + "_o*";
                }
            }

            case IL2CPP_TYPE_VAR:
            case IL2CPP_TYPE_MVAR:
                return "struct Il2CppObject*";

            default:
                return "void*";
        }
    }

    static inline std::string RecursionStructInfo(const Il2CppClass* klass) {
        if (!klass) return "";
        auto cklass = const_cast<Il2CppClass*>(klass);
        std::string baseName = getClassFixFullName(cklass);

        if (alreadyForwardDeclared.find(baseName) == alreadyForwardDeclared.end()) {
            sb << "struct " << baseName << "_o;\n";
            sb << "struct " << baseName << "_c;\n";
            sb << "struct " << baseName << "_Fields;\n";
            alreadyForwardDeclared.insert(baseName);
        }

        if (alreadyParsed.find(cklass) != alreadyParsed.end()) return "";
        alreadyParsed.insert(cklass);

        std::stringstream pre;
        std::stringstream ss;

        if (cklass->parent) {
            pre << RecursionStructInfo(cklass->parent);
        }

        ss << "struct " << baseName << "_Fields {\n";
        if (cklass->parent) {
            ss << "\tstruct " << getClassFixFullName(cklass->parent) << "_Fields __parent;\n";
        }

        void* fieldIter = nullptr;
        int fIdx = 0;
        std::vector<FieldInfo*> staticFields;

        while (auto field = il2cpp_class_get_fields(cklass, &fieldIter)) {
            if (il2cpp_field_get_flags(field) & 0x0010) {
                staticFields.emplace_back(field);
                continue;
            }

            Il2CppClass* fieldKlass = il2cpp_class_from_type(field->type);
            if (fieldKlass && il2cpp_class_is_valuetype(fieldKlass) && fieldKlass != cklass) {
                pre << RecursionStructInfo(fieldKlass);
            }
            ss << "\t" << ParseType(field->type) << " " << fixName(field->name) << "_" << fIdx++ << ";\n";
        }
        ss << "};\n";

        if (!staticFields.empty()) {
            for (auto* sf : staticFields) {
                Il2CppClass* sfKlass = il2cpp_class_from_type(sf->type);
                if (sfKlass && il2cpp_class_is_valuetype(sfKlass) && sfKlass != cklass) {
                    pre << RecursionStructInfo(sfKlass);
                }
            }
            ss << "struct " << baseName << "_StaticFields {\n";
            int sfIdx = 0;
            for (auto* sf : staticFields) {
                ss << "\t" << ParseType(sf->type) << " " << fixName(sf->name) << "_" << sfIdx++ << ";\n";
            }
            ss << "};\n";
        }

        if (cklass->vtable_count > 0) {
            ss << "struct " << baseName << "_VTable {\n";
            for (int k = 0; k < cklass->vtable_count; ++k) {
                auto method = cklass->vtable[k].method;
                std::string mName = (method && method->name) ? fixName(method->name) : "unknown";
                ss << "\tstruct VirtualInvokeData _" << k << "_" << mName << ";\n";
            }
            ss << "};\n";
        }

        ss << "struct " << baseName << "_c {\n";
        ss << "\tstruct Il2CppClass_1 _1;\n";
        ss << "\t" << (staticFields.empty() ? "void*" : "struct " + baseName + "_StaticFields*") << " static_fields;\n";
        ss << "\tunion Il2CppRGCTXData* rgctx_data;\n";
        ss << "\tstruct Il2CppClass_2 _2;\n";
        ss << "\tstruct " << (cklass->vtable_count > 0 ? baseName + "_VTable" : "VirtualInvokeData") << " vtable[" << (cklass->vtable_count > 0 ? std::to_string(cklass->vtable_count) : "32") << "];\n";
        ss << "};\n";

        ss << "struct " << baseName << "_o {\n";
        if (!il2cpp_class_is_valuetype(cklass)) {
            ss << "\tstruct " << baseName << "_c *klass;\n";
            ss << "\tvoid *monitor;\n";
        }
        ss << "\tstruct " << baseName << "_Fields fields;\n";
        ss << "};\n";

        return pre.str() + ss.str();
    }

    static inline void doClass(const Il2CppClass* klass) {
        if (!klass) return;

        headerStruct << RecursionStructInfo(klass);

        void* iter = nullptr;
        while (auto nestedClass = il2cpp_class_get_nested_types(const_cast<Il2CppClass*>(klass), &iter)) {
            doClass(nestedClass);
        }
    }

public:

    static inline void GenerateScriptJson() {
        LOGI("Generating script.json");
        uintptr_t il2cppBase = proc_map.startAddress;
        auto domain = il2cpp_domain_get();
        size_t assemblyCount;
        auto assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);

        nlohmann::json root;
        root["ScriptMethod"] = nlohmann::json::array();
        root["ScriptMetadata"] = nlohmann::json::array();
        root["ScriptMetadataMethod"] = nlohmann::json::array();
        root["ScriptString"] = nlohmann::json::array();

        std::vector<uintptr_t> all_function_pointers;

        for (size_t i = 0; i < assemblyCount; ++i) {
            auto image = il2cpp_assembly_get_image(assemblies[i]);
            if (!image) continue;

            for (int j = 0; j < il2cpp_image_get_class_count(image); ++j) {
                auto klass = il2cpp_image_get_class(image, j);
                if (!klass) continue;

                void* methodIter = nullptr;
                while (auto method = il2cpp_class_get_methods(const_cast<Il2CppClass*>(klass), &methodIter)) {
                    uintptr_t methodPtr = (uintptr_t)method->methodPointer;
                    if (methodPtr == 0 || methodPtr < il2cppBase) {
                        continue;
                    }
                    uintptr_t rva = methodPtr - il2cppBase;
                    all_function_pointers.emplace_back(rva);

                    std::string ns = klass->namespaze ? klass->namespaze : "";
                    std::string cl = klass->name ? klass->name : "unnamed";
                    std::string mt = method->name ? method->name : "unnamed";
                    std::string fullName = (ns.empty() ? "" : ns + ".") + cl + "$$" + mt;

                    std::stringstream signature;
                    signature << ParseType(method->return_type) << " " << fixName(fullName) << " (";

                    std::vector<std::string> params;
                    if (!(method->flags & 0x0010)) {
                        params.emplace_back(ParseType(il2cpp_class_get_type(const_cast<Il2CppClass*>(klass))) + " __this");
                    }
                    for (int k = 0; k < method->parameters_count; k++) {
                        const char* pname = il2cpp_method_get_param_name(method, k);
                        std::string safeName;
                        if (pname && pname[0] != '\0') {
                            safeName = fixName(pname);
                        } else {
                            safeName = "p" + std::to_string(k);
                        }
                        params.emplace_back(ParseType(method->parameters[k]) + " " + safeName);
                    }
                    params.emplace_back("const MethodInfo* method");

                    for (size_t p = 0; p < params.size(); ++p) {
                        signature << params[p] << (p == params.size() - 1 ? "" : ", ");
                    }
                    signature << ");";

                    nlohmann::json mJson;
                    mJson["Address"] = rva;
                    mJson["Name"] = fullName;
                    mJson["Signature"] = signature.str();
                    mJson["TypeSignature"] = GetMethodTypeSignature(method);
                    root["ScriptMethod"].emplace_back(mJson);
                }
            }
        }

        std::sort(all_function_pointers.begin(), all_function_pointers.end());
        all_function_pointers.erase(std::unique(all_function_pointers.begin(), all_function_pointers.end()), all_function_pointers.end());
        all_function_pointers.erase(std::remove(all_function_pointers.begin(), all_function_pointers.end(), 0), all_function_pointers.end());

        root["Addresses"] = all_function_pointers;

        std::string outputDir = "/storage/emulated/0/Android/data/" + GetPackageName() + "/dump";
        std::filesystem::create_directories(outputDir);
        std::string outputPath = outputDir + "/script.json";

        std::ofstream out(outputPath);
        if (out.is_open()) {
            out << root.dump(4);
            out.close();
            LOGI("Output script.json at %s", outputPath.c_str());
        } else {
            LOGI("Failed to write script.json at %s", outputPath.c_str());
        }
    }

    static inline void GenerateIl2CppHeader() {
        LOGI("Generating il2cpp.h");
        sb.str(""); sb.clear();
        headerStruct.str(""); headerStruct.clear();
        alreadyParsed.clear();
        alreadyForwardDeclared.clear();

        void* handle = dlopen("libil2cpp.so", RTLD_LAZY);
        il2cpp_thread_attach(il2cpp_domain_get());

        auto domain = il2cpp_domain_get();
        size_t assemblyCount;
        auto assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);

        for (size_t i = 0; i < assemblyCount; ++i) {
            auto image = il2cpp_assembly_get_image(assemblies[i]);
            if (!image) continue;

            const char* imageNameChar = il2cpp_image_get_name(image);
            std::string imageName = imageNameChar ? imageNameChar : "";

            for (int j = 0; j < il2cpp_image_get_class_count(image); ++j) {
                auto klass = il2cpp_image_get_class(image, j);
                if (klass) doClass(klass);
            }
        }

        std::stringstream finalFile;
        finalFile << genericHeader << "\n";
        finalFile << sb.str() << "\n";
        finalFile << header << "\n";
        finalFile << headerStruct.str();

        std::string result = finalFile.str();
        std::string outputDir = "/storage/emulated/0/Android/data/" + GetPackageName() + "/dump";
        std::filesystem::create_directories(outputDir);
        std::string outputPath = outputDir + "/il2cpp.h";

        std::ofstream out(outputPath);
        if (out.is_open()) {
            out << result;
            out.close();
            LOGI("Header saved to: %s", outputPath.c_str());
        } else {
            LOGI("Failed to write il2cpp.h at %s", outputPath.c_str());
        }
    }

    static inline void Gen() {
        GenerateIl2CppHeader();
        GenerateScriptJson();
    }
};
