#ifdef __CUDACC__

#define SCHNEK_WITH_CUDA__
#define SCHNEK_INLINE __host__ __device__ inline

#else

#define SCHNEK_INLINE inline

#endif
