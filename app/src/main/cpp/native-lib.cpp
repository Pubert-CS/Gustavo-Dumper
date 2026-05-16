#include <android/log.h>
#include <jni.h>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <dlfcn.h>
#include <globals.hpp>
#include "il2cpp/il2cpp-system-reflection.h"
#include <output/StructGen.hpp>
#include <output/DumpGen.hpp>
#include <output/ScaffoldGen.hpp>

#include <KittyInclude.hpp>

bool didMd = false;
bool didLib = false;

__attribute__((constructor)) void onLoad() {
    LOGI("Loaded");
    std::thread([]() {
        while (!proc_map.isValid()) {
            proc_map = KittyMemory::getElfBaseMap("libil2cpp.so");
            if (!proc_map.isValid()) std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        auto sc = ElfScanner::createWithMap(proc_map);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        LOGI("Loaded");

        LOGI("[GenFlow] Starting ScaffoldGen::Gen");
        ScaffoldGen::Gen();
        LOGI("[GenFlow] Finished ScaffoldGen::Gen");

        LOGI("[GenFlow] Starting StructGen::Gen");
        StructGen::Gen();
        LOGI("[GenFlow] Finished StructGen::Gen");

        LOGI("[GenFlow] Starting DumpGen::Gen");
        DumpGen::Gen();
        LOGI("[GenFlow] Finished DumpGen::Gen");

    }).detach();
}
