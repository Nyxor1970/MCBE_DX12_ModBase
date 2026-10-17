#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dobby.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "NetherMod", __VA_ARGS__)

struct DimensionHeightRange { short min; short max; };

// ضع هنا عنوان دالة Dimension من خريطة رموز إصدارك (0 = لا هوك)
static const uintptr_t DIM_CTOR_RVA = 0x0;

typedef void (*DimCtor_fn)(void*, void*, int, DimensionHeightRange, void*);
static DimCtor_fn orig_DimCtor = nullptr;

static void hook_DimCtor(void* self, void* level, int type,
                         DimensionHeightRange range, void* sched) {
    if (type == 1) range.max = 256; // النيذر
    orig_DimCtor(self, level, type, range, sched);
}

static uintptr_t getBase(const char* lib) {
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[512]; uintptr_t base = 0;
    while (fgets(line, sizeof line, f))
        if (strstr(line, lib)) { base = strtoull(line, nullptr, 16); break; }
    fclose(f);
    return base;
}

static void* worker(void*) {
    uintptr_t base = 0;
    while (!(base = getBase("libminecraftpe.so"))) usleep(200000);
    LOGI("base = %p", (void*)base);
    if (!DIM_CTOR_RVA) { LOGI("RVA not set, skipping hook"); return nullptr; }
    int r = DobbyHook((void*)(base + DIM_CTOR_RVA),
                      (void*)hook_DimCtor, (void**)&orig_DimCtor);
    LOGI("DobbyHook = %d", r);
    return nullptr;
}

__attribute__((constructor)) static void init() {
    pthread_t t; pthread_create(&t, nullptr, worker, nullptr);
}
