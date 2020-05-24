#define FAST_ATOMIC_FLOAT

/**
Defines a 4-byte AtomicFloatType to be used for atomic float storage.
Defines getAtomicFloat(mem) to read an atomic float value from an AtomicFloatType.
Defines atomicAddFloat(mem, data) to atomically add a float data to an AtomicFloatType.

If drivers support the NV_shader_atomic_float extension, atomicAddFloat will 
use atomicAdd(float*, float), which will provide good performance and precision.

Otherwise, if FAST_ATOMIC_FLOAT is defined, atomicAddFloat will perform a 
fixed-point conversion and use atomicAdd(int*, int) internally. This is fast
but runs the risk of overflow and loss of precision.

If FAST_ATOMIC_FLOAT is not defined, atomicAddFloat will use atomicCompSwap()
internally, which allows full precision but performs much worse.
*/
#extension GL_NV_shader_atomic_float : enable
#ifdef GL_NV_shader_atomic_float
    #define AtomicFloatType float
    // fast and always correct
    #define atomicAddFloat(mem, data) atomicAdd(mem, data)
    #define getAtomicFloat(mem) mem
#else
    #ifdef FAST_ATOMIC_FLOAT
        #define AtomicFloatType int
        #define atomicAddFloat(mem, data) atomicAdd(mem, float2fix(data))
        #define getAtomicFloat(mem) fix2float(mem)
    #else
        #define AtomicFloatType int
        // slow but always correct
        #define atomicAddFloat(mem, data) \
            { \
                int expected, result; \
                do { \
                    expected = mem; \
                    result = floatBitsToInt(intBitsToFloat(expected) + data); \
                } while (atomicCompSwap(mem, expected, result) != expected); \
            }
        #define getAtomicFloat(mem) intBitsToFloat(mem)
    #endif
#endif

const int MAX_INT = 2147483647;
const int FIXED_SCALE = MAX_INT / 10000;

int float2fix(float f) {
    return int(round(f * FIXED_SCALE));
}

float fix2float(int fix) {
    return fix / float(FIXED_SCALE);
}
