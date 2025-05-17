#ifdef __CUDACC__
#define SCHNEK_WITH_CUDA__
#define SCHNEK_INLINE __host__ __device__ inline
#define SCHNEK_DEVICE_LAMBDA [=] __device__

#else

#define SCHNEK_INLINE inline
#define SCHNEK_DEVICE_LAMBDA [=]

#endif