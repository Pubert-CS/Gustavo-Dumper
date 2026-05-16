#pragma once
#include "il2cpp-api-functions.h"
#include "mono/MonoString.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace Reflection {
    template<typename T>
    struct Array : Il2CppObject
    {
        Il2CppArrayBounds* bounds;
        il2cpp_array_size_t capacity;
        T m_Items[0];

        inline il2cpp_array_size_t GetSize() const
        {
            return capacity;
        }

        inline T* At(il2cpp_array_size_t index)
        {
            if (!this || index >= capacity) return nullptr;
            return &m_Items[index];
        }

        inline std::vector<T> ToVector() const
        {
            if (!this) return {};
            std::vector<T> items_v;
            for (int i = 0; i < capacity; i++) items_v.push_back(m_Items[i]);
            return items_v;
        }
    };

    inline std::unordered_map<std::string, std::unordered_map<std::string, const Il2CppClass*>> classMap;

    void Init() {
        auto dm = il2cpp_domain_get();
        size_t a_c = 0;
        auto assemblies = il2cpp_domain_get_assemblies(dm, &a_c);
        for (int i = 0; i < a_c; i++) {
            auto assembly = assemblies[i];
            auto image = il2cpp_assembly_get_image(assembly);
            size_t cc = il2cpp_image_get_class_count(image);
            for (int j = 0; j < cc; j++) {
                auto klass = il2cpp_image_get_class(image, j);
                classMap[std::string(klass->namespaze)][std::string(klass->name)] = klass;
            }
        }
    }

    struct MethodInfo;
    struct FieldInfo;
    struct Type;
    struct Assembly;
    struct AppDomain;
    struct MonoType;

    struct MonoParameterInfo {
        MonoString* get_Name() {
            auto klass = classMap["System.Reflection"]["ParameterInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_Name", 0);
            return reinterpret_cast<MonoString*(*)(MonoParameterInfo*)>((void*)meth->methodPointer)(this);
        }

        MonoType* get_ParameterType() {
            auto klass = classMap["System.Reflection"]["ParameterInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_ParameterType", 0);
            return reinterpret_cast<MonoType*(*)(MonoParameterInfo*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsOut() {
            auto klass = classMap["System.Reflection"]["ParameterInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsOut", 0);
            return reinterpret_cast<bool(*)(MonoParameterInfo*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsIn() {
            auto klass = classMap["System.Reflection"]["ParameterInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsIn", 0);
            return reinterpret_cast<bool(*)(MonoParameterInfo*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsOptional() {
            auto klass = classMap["System.Reflection"]["ParameterInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsOptional", 0);
            return reinterpret_cast<bool(*)(MonoParameterInfo*)>((void*)meth->methodPointer)(this);
        }
    };

    struct MonoCustomAttribute {
        MonoType* get_AttributeType() {
            auto klass = classMap["System.Reflection"]["CustomAttributeData"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_AttributeType", 0);
            return reinterpret_cast<MonoType*(*)(MonoCustomAttribute*)>((void*)meth->methodPointer)(this);
        }
    };

    struct MonoMemberInfo {
        Array<MonoCustomAttribute*>* GetCustomAttributesData() {
            auto klass = classMap["System.Reflection"]["MemberInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "GetCustomAttributesData", 0);
            return reinterpret_cast<Array<MonoCustomAttribute*>*(*)(MonoMemberInfo*)>((void*)meth->methodPointer)(this);
        }
    };

    struct MonoMethodInfo : MonoMemberInfo {
        MonoString* get_Name() {
            auto klass = classMap["System.Reflection"]["MethodInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_Name", 0);
            return reinterpret_cast<MonoString*(*)(MonoMethodInfo*)>((void*)meth->methodPointer)(this);
        }

        MonoType* get_ReturnType() {
            auto klass = classMap["System.Reflection"]["MethodInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_ReturnType", 0);
            return reinterpret_cast<MonoType*(*)(MonoMethodInfo*)>((void*)meth->methodPointer)(this);
        }

        Array<MonoParameterInfo*>* GetParameters() {
            auto klass = classMap["System.Reflection"]["MethodInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "GetParameters", 0);
            return reinterpret_cast<Array<MonoParameterInfo*>*(*)(MonoMethodInfo*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsPublic() {
            auto klass = classMap["System.Reflection"]["MethodBase"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsPublic", 0);
            return reinterpret_cast<bool(*)(MonoMethodInfo*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsPrivate() {
            auto klass = classMap["System.Reflection"]["MethodBase"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsPrivate", 0);
            return reinterpret_cast<bool(*)(MonoMethodInfo*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsStatic() {
            auto klass = classMap["System.Reflection"]["MethodBase"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsStatic", 0);
            return reinterpret_cast<bool(*)(MonoMethodInfo*)>((void*)meth->methodPointer)(this);
        }
    };

    struct MonoFieldInfo : MonoMemberInfo {
        MonoString* get_Name() {
            auto klass = classMap["System.Reflection"]["FieldInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_Name", 0);
            return reinterpret_cast<MonoString*(*)(MonoFieldInfo*)>((void*)meth->methodPointer)(this);
        }

        MonoType* get_FieldType() {
            auto klass = classMap["System.Reflection"]["FieldInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_FieldType", 0);
            return reinterpret_cast<MonoType*(*)(MonoFieldInfo*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsPublic() {
            auto klass = classMap["System.Reflection"]["FieldInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsPublic", 0);
            return reinterpret_cast<bool(*)(MonoFieldInfo*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsPrivate() {
            auto klass = classMap["System.Reflection"]["FieldInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsPrivate", 0);
            return reinterpret_cast<bool(*)(MonoFieldInfo*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsStatic() {
            auto klass = classMap["System.Reflection"]["FieldInfo"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsStatic", 0);
            return reinterpret_cast<bool(*)(MonoFieldInfo*)>((void*)meth->methodPointer)(this);
        }
    };

    struct MonoType : MonoMemberInfo {
        MonoString* get_Name() {
            auto klass = classMap["System"]["Type"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_Name", 0);
            return reinterpret_cast<MonoString*(*)(MonoType*)>((void*)meth->methodPointer)(this);
        }

        MonoString* get_Namespace() {
            auto klass = classMap["System"]["Type"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_Namespace", 0);
            return reinterpret_cast<MonoString*(*)(MonoType*)>((void*)meth->methodPointer)(this);
        }

        bool get_IsByRef() {
            auto klass = classMap["System"]["Type"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_IsByRef", 0);
            return reinterpret_cast<bool(*)(MonoType*)>((void*)meth->methodPointer)(this);
        }

        Array<MonoMethodInfo*>* GetMethods() {
            auto klass = classMap["System"]["Type"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "GetMethods", 0);
            return reinterpret_cast<Array<MonoMethodInfo*>*(*)(MonoType*)>((void*)meth->methodPointer)(this);
        }

        Array<MonoFieldInfo*>* GetFields() {
            auto klass = classMap["System"]["Type"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "GetFields", 0);
            return reinterpret_cast<Array<MonoFieldInfo*>*(*)(MonoType*)>((void*)meth->methodPointer)(this);
        }
    };

    struct Assembly {
        MonoString* get_FullName() {
            auto klass = classMap["System.Reflection"]["Assembly"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_FullName", 0);
            return reinterpret_cast<MonoString*(*)(Assembly*)>((void*)meth->methodPointer)(this);
        }

        Array<MonoType*>* GetTypes() {
            auto klass = classMap["System.Reflection"]["Assembly"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "GetTypes", 0);
            return reinterpret_cast<Array<MonoType*>*(*)(Assembly*)>((void*)meth->methodPointer)(this);
        }
    };

    struct AppDomain {
        static AppDomain* get_CurrentDomain() {
            auto klass = classMap["System"]["AppDomain"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "get_CurrentDomain", 0);
            return reinterpret_cast<AppDomain*(*)()>((void*)meth->methodPointer)();
        }

        Array<Assembly*>* GetAssemblies() {
            auto klass = classMap["System"]["AppDomain"];
            auto meth = il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(klass), "GetAssemblies", 0);
            return reinterpret_cast<Array<Assembly*>*(*)(AppDomain*)>((void*)meth->methodPointer)(this);
        }
    };
}