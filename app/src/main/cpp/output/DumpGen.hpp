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
#include <codecvt>
#include <dlfcn.h>

#include <string-utils.h>
#include <nlohmann/json.hpp>
#include <KittyInclude.hpp>
#include <il2cpp/il2cpp-headers.h>
#include "il2cpp/il2cpp-tabledefs.hpp"
#include <il2cpp/il2cpp-api-functions.h>
#include <il2cpp/il2cpp-exports.hpp>
#include <il2cpp/ext/modifier-util.hpp>

class DumpGen {
private:
    static inline std::stringstream writer;
    static inline std::string getIndent(const int depth) {
        return std::string(depth, '\t');
    }

    static inline std::string getNestedTypeName(Il2CppClass* klass) {
        if (!klass) return "unknown";

        std::vector<std::string> segments;
        auto current = klass;
        while (current) {
            if (current->name && current->name[0] != '\0') {
                segments.emplace_back(current->name);
            } else {
                segments.emplace_back("unnamed_" + std::to_string(reinterpret_cast<uintptr_t>(current)));
            }
            current = current->declaringType;
        }

        std::reverse(segments.begin(), segments.end());

        std::stringstream ss;
        for (size_t i = 0; i < segments.size(); ++i) {
            if (i > 0) ss << ".";
            ss << segments[i];
        }
        return ss.str();
    }

    static inline std::string getIl2CppActualNameString(Il2CppClass* klass) {
        if (!klass) return "void";

        auto type = il2cpp_class_get_type(klass);
        if (!type) return "void";

        switch (type->type) {
            case IL2CPP_TYPE_VOID:    return "void";
            case IL2CPP_TYPE_BOOLEAN: return "bool";
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
            case IL2CPP_TYPE_STRING:  return "string";
            case IL2CPP_TYPE_OBJECT:  return "object";
            case IL2CPP_TYPE_ARRAY:
            case IL2CPP_TYPE_SZARRAY: {
                auto elClass = il2cpp_class_get_element_class(klass);
                return (elClass ? getIl2CppActualNameString(elClass) : "object") + "[]";
            }
            default: {
                std::stringstream ss;

                const char* ns = klass->namespaze;
                auto nsOwner = klass->declaringType;
                while ((!ns || ns[0] == '\0') && nsOwner) {
                    ns = nsOwner->namespaze;
                    nsOwner = nsOwner->declaringType;
                }

                if (ns && ns[0] != '\0') {
                    ss << ns << ".";
                }

                auto nestedName = getNestedTypeName(klass);
                if (type->type == IL2CPP_TYPE_GENERICINST && klass->generic_class && klass->generic_class->context.class_inst) {
                    auto evilString = "`" + std::to_string(klass->generic_class->context.class_inst->type_argc);
                    if (nestedName.find(evilString) != std::string::npos) {
                        nestedName = ReplaceAll(nestedName, evilString, "");
                    }
                }
                ss << nestedName;

                if (type->type == IL2CPP_TYPE_GENERICINST && klass->generic_class) {
                    auto classInst = klass->generic_class->context.class_inst;
                    if (classInst && classInst->type_argc > 0) {
                        ss << "<";
                        for (int i = 0; i < classInst->type_argc; i++) {
                            auto genericType = classInst->type_argv[i];
                            if (genericType) {
                                auto genericClass = il2cpp_class_from_type(genericType);
                                ss << getIl2CppActualNameString(genericClass);
                            } else {
                                ss << "unknown";
                            }

                            if (i < classInst->type_argc - 1) {
                                ss << ", ";
                            }
                        }
                        ss << ">";
                    }
                }
                return ss.str();
            }
        }
    }
    static inline std::string getIl2CppActualNameString(Il2CppType* type) {
        return getIl2CppActualNameString(il2cpp_class_from_type(type));
    }
    static inline std::string il2cpp_to_std(Il2CppString* il2cpp_str) {
        if (!il2cpp_str || !il2cpp_str->chars) {
            return "";
        }

        std::u16string u16((char16_t*)il2cpp_str->chars, il2cpp_str->length);
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;

        try {
            return convert.to_bytes(u16);
        } catch (const std::range_error& e) {
            return "";
        }
    }

    static inline void emitClass(Il2CppClass* klass,
                                 const std::unordered_map<Il2CppClass*, std::vector<Il2CppClass*>>& nestedTypesByParent,
                                 std::unordered_set<Il2CppClass*>& emittedTypes,
                                 const int indentDepth) {
        if (!klass || emittedTypes.find(klass) != emittedTypes.end()) {
            return;
        }
        emittedTypes.insert(klass);

        const auto indent = getIndent(indentDepth);
        const auto memberIndent = indent + "\t";

        std::vector<std::string> extends;

        writer << indent << "// Namespace: " << klass->namespaze << "\n";

        writer << indent;
        if (il2cpp_class_is_valuetype(klass)) {
            writer << "struct";
        } else {
            writer << get_class_modifier(klass->flags) << get_class_type_keyword(const_cast<Il2CppClass*>(klass));
        }
        writer << " " << klass->name;

        if (klass->parent) {
            auto fullParentName = getIl2CppActualNameString(klass->parent);
            if (!il2cpp_class_is_valuetype(klass->parent) && !il2cpp_class_is_enum(klass->parent) && il2cpp_class_get_type(klass->parent)->type != IL2CPP_TYPE_OBJECT) {
                extends.emplace_back(fullParentName);
            }
        }
        if (!il2cpp_class_is_valuetype(klass)) {
            if (klass->interfaces_count > 0) {
                void* interIter = nullptr;
                while (auto interface = il2cpp_class_get_interfaces(const_cast<Il2CppClass*>(klass), &interIter)) {
                    auto fullParentName = getIl2CppActualNameString(interface);
                    extends.emplace_back(fullParentName);
                }
            }
        }
        if (!extends.empty()) {
            writer << " : " << extends[0];
            for (size_t k = 1; k < extends.size(); ++k) {
                writer << ", " << extends[k];
            }
        }
        writer << "\n" << indent << "{\n";

        writer << memberIndent << "// Fields\n";
        void* fieldIter = nullptr;
        while (auto field = il2cpp_class_get_fields(const_cast<Il2CppClass*>(klass), &fieldIter)) {
            writer << memberIndent;
            auto fflags = il2cpp_field_get_flags(field);
            writer << get_field_modifier(fflags);
            writer << getIl2CppActualNameString(const_cast<Il2CppType*>(field->type)) << " " << field->name;

            if (fflags & FIELD_ATTRIBUTE_LITERAL) {
                auto typeEnum = field->type->type;

                writer << " = ";

                switch (typeEnum) {
                    case IL2CPP_TYPE_BOOLEAN: {
                        bool val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << (val ? "true" : "false");
                        break;
                    }
                    case IL2CPP_TYPE_I1: {
                        int8_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << (int)val;
                        break;
                    }
                    case IL2CPP_TYPE_U1: {
                        uint8_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << (int)val;
                        break;
                    }
                    case IL2CPP_TYPE_I2: {
                        int16_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << val;
                        break;
                    }
                    case IL2CPP_TYPE_U2: {
                        uint16_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << val;
                        break;
                    }
                    case IL2CPP_TYPE_I4: {
                        int32_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << val;
                        break;
                    }
                    case IL2CPP_TYPE_U4: {
                        uint32_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << val;
                        break;
                    }
                    case IL2CPP_TYPE_I8: {
                        int64_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << val;
                        break;
                    }
                    case IL2CPP_TYPE_U8: {
                        uint64_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << val;
                        break;
                    }
                    case IL2CPP_TYPE_R4: {
                        float val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << val << "f";
                        break;
                    }
                    case IL2CPP_TYPE_R8: {
                        double val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << val;
                        break;
                    }
                    case IL2CPP_TYPE_CHAR: {
                        uint16_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << "'" << (char)val << "'";
                        break;
                    }
                    case IL2CPP_TYPE_STRING: {
                        Il2CppString* val;
                        il2cpp_field_static_get_value(field, &val);
                        if (val && val->chars) {
                            writer << "\"" << il2cpp_to_std(val) << "\"";
                        } else {
                            writer << "null";
                        }
                        break;
                    }
                    case IL2CPP_TYPE_VALUETYPE: {
                        int32_t val;
                        il2cpp_field_static_get_value(field, &val);
                        writer << val;
                        break;
                    }
                    default:
                        writer << "/* unknown const */";
                        break;
                }
            }

            writer << ";";
            writer << " // 0x" << std::hex << std::uppercase << field->offset;
            writer << "\n";
        }
        writer << "\n";

        writer << memberIndent << "// Properties\n";
        void* propertyIter = nullptr;
        while (auto property = il2cpp_class_get_properties(const_cast<Il2CppClass*>(klass), &propertyIter)) {
            auto prop_get_method = il2cpp_property_get_get_method(const_cast<PropertyInfo*>(property));
            auto prop_set_method = il2cpp_property_get_set_method(const_cast<PropertyInfo*>(property));

            writer << memberIndent;

            if (prop_get_method) {
                writer << getIl2CppActualNameString(const_cast<Il2CppType*>(prop_get_method->return_type));
            } else if (prop_set_method) {
                writer << getIl2CppActualNameString(const_cast<Il2CppType*>(prop_set_method->parameters[0]));
            } else {
                writer << "UNKNOWN_TYPE";
            }

            writer << " ";
            writer << property->name << " ";
            writer << "{";

            if (prop_get_method) {
                writer << get_method_access_for_prop(prop_get_method->flags) << "get;";
            }
            if (prop_set_method) {
                if (prop_get_method) writer << " ";
                writer << get_method_access_for_prop(prop_set_method->flags) << "set;";
            }

            writer << " }\n";
        }

        writer << "\n";
        writer << memberIndent << "// Methods\n\n";
        void* methodIter = nullptr;
        while (auto method = il2cpp_class_get_methods(const_cast<Il2CppClass*>(klass), &methodIter)) {
            auto mflags = method->flags;
            if (!(mflags & METHOD_ATTRIBUTE_ABSTRACT) && (uintptr_t)method->methodPointer > 0) {
                writer << memberIndent << "// RVA: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << (uintptr_t)method->methodPointer;
                writer << " Offset: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << (uintptr_t)((uintptr_t)method->methodPointer - proc_map.startAddress);
                writer << " VA: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << (uintptr_t)method->methodPointer;
            } else {
                writer << memberIndent << " RVA: -1 Offset: -1";
            }
            if (method->slot != 65535) {
                writer << " Slot: " << method->slot;
            }
            writer << "\n";

            writer << memberIndent;
            writer << get_method_modifier(mflags);
            writer << getIl2CppActualNameString(const_cast<Il2CppType*>(method->return_type)) << " ";
            writer << method->name;
            writer << "(";

            for (int k = 0; k < method->parameters_count; k++) {
                auto param = method->parameters[k];
                writer << getIl2CppActualNameString(const_cast<Il2CppType*>(param)) << " " << il2cpp_method_get_param_name(method, k);
                if (k < method->parameters_count - 1) {
                    writer << ", ";
                }
            }

            writer << ") { }\n\n";
        }

        auto nestedIt = nestedTypesByParent.find(klass);
        if (nestedIt != nestedTypesByParent.end() && !nestedIt->second.empty()) {
            writer << memberIndent << "// Nested types\n\n";
            for (auto* nestedClass : nestedIt->second) {
                emitClass(nestedClass, nestedTypesByParent, emittedTypes, indentDepth + 1);
                writer << "\n";
            }
        }

        writer << indent << "}\n";
    }
public:
    static inline void GenerateDumpCs() {
        LOGI("Generating dump.cs");
        writer.str("");
        writer.clear();

        auto domain = il2cpp_domain_get();

        size_t assemblyCount;
        auto assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);

        for (int imageCount = 0; imageCount < assemblyCount; imageCount++) {
            auto assembly = assemblies[imageCount];
            if (!assembly) continue;
            auto image = il2cpp_assembly_get_image(assembly);
            if (!image) continue;
            writer << "// Image " << std::to_string(imageCount) << ": " << assembly->aname.name << ".dll - " << image->typeCount << "\n";
        }

        std::vector<Il2CppClass*> allTypes;
        std::unordered_map<Il2CppClass*, std::vector<Il2CppClass*>> nestedTypesByParent;
        std::unordered_set<Il2CppClass*> nestedTypeSet;

        for (int i = 0; i < assemblyCount; i++) {
            auto assembly = assemblies[i];
            if (!assembly) continue;
            auto image = il2cpp_assembly_get_image(assembly);
            if (!image) continue;

            auto type_count = il2cpp_image_get_class_count(image);
            for (int j = 0; j < type_count; j++) {
                auto klass = il2cpp_image_get_class(image, j);
                if (!klass) continue;
                allTypes.emplace_back(const_cast<Il2CppClass*>(klass));

                void* nestedIter = nullptr;
                while (auto nestedClass = il2cpp_class_get_nested_types(const_cast<Il2CppClass*>(klass), &nestedIter)) {
                    nestedTypesByParent[const_cast<Il2CppClass*>(klass)].emplace_back(nestedClass);
                    nestedTypeSet.insert(nestedClass);
                }
            };
        }

        std::unordered_set<Il2CppClass*> emittedTypes;
        for (auto* klass : allTypes) {
            if (nestedTypeSet.find(klass) != nestedTypeSet.end()) {
                continue;
            }
            emitClass(klass, nestedTypesByParent, emittedTypes, 0);
            writer << "\n";
        }

        std::string outputDir = "/storage/emulated/0/Android/data/" + GetPackageName() + "/dump";
        std::filesystem::create_directories(outputDir);
        std::string outputPath = outputDir + "/dump.cs";
        std::ofstream out(outputPath);
        if (out.is_open()) {
            out << writer.str();
            out.close();
            LOGI("Output dump.cs at %s", outputPath.c_str());
        }
    }

    static inline void Gen() {
        GenerateDumpCs();
    }
};
