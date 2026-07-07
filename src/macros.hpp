#if defined(__CUDACC__) || defined(__HIPCC__)

#define SCHNEK_WITH_CUDA__
#define SCHNEK_INLINE __host__ __device__ inline

#define SCHNEK_FUNCTION __host__ __device__
#define SCHNEK_INLINE_FUNCTION __host__ __device__ inline

#else

#define SCHNEK_INLINE inline

#define SCHNEK_FUNCTION
#define SCHNEK_INLINE_FUNCTION inline

#endif
