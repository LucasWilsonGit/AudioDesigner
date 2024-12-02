#define MINIAUDIO_IMPLEMENTATION

#include "AudioEngine/core.hpp"
#include "AudioEngine/miniaudio_utils.hpp"

static bool uninitialized = false;

template <class T>
struct test_allocator {
    int& allocated = 0;

    T* allocate(size_t count) {
        allocated += count;
        return new T[count];
    }

    void deallocate(T* inst, size_t count) {
        delete[] inst;
        allocated -= count;
    }
};

ma_result uninit_wrapper(ma_context *ctx) {
    uninitialized = true;
    return ma_context_uninit(ctx);
}

int main() {
    
    int allocation_count = 0;
    auto ctx_allocator = test_allocator<ma_context>{.allocated=allocation_count};
    {
        ma_context *ctx = ctx_allocator.allocate(1);
        AudioEngine::ma_call(ma_context_init(NULL, 0, NULL, ctx));
        AudioEngine::ma_wrapper<ma_context, uninit_wrapper, decltype(ctx_allocator)> context(ctx, std::move(ctx_allocator));

        if (allocation_count != 1)
            return 1;
    }

    if (!uninitialized)
        return 2;
    
    if (allocation_count != 0)
        return 3;

    

    return 0;
}