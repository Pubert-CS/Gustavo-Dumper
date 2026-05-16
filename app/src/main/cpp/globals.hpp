#pragma once
#include <android/log.h>
#include <jni.h>
#include <string>
#include <filesystem>
#include <KittyInclude.hpp>

#define _Tag "Scaffolder"
#define LOGI(...) \
  __android_log_print(ANDROID_LOG_INFO, _Tag, __VA_ARGS__)

static inline ProcMap proc_map;

std::string GetPackageName() {
    char application_id[256] = {0};
    FILE *fp = fopen("/proc/self/cmdline", "r");
    if (fp) {
        fread(application_id, 1, sizeof(application_id) - 1, fp);
        fclose(fp);
    }
    return std::string(application_id);
}