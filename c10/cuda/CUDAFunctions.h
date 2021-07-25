#pragma once

// This header provides C++ wrappers around commonly used CUDA API functions.
// The benefit of using C++ here is that we can raise an exception in the
// event of an error, rather than explicitly pass around error codes.  This
// leads to more natural APIs.
//
// The naming convention used here matches the naming convention of torch.cuda

#include <c10/core/Device.h>
#include <c10/cuda/CUDAException.h>
#include <c10/cuda/CUDAMacros.h>
#ifdef __HIP_PLATFORM_HCC__
#include <hip/hip_version.h>
#endif
#include <cuda_runtime_api.h>

namespace c10 {
namespace cuda {

// NB: In the past, we were inconsistent about whether or not this reported
// an error if there were driver problems are not.  Based on experience
// interacting with users, it seems that people basically ~never want this
// function to fail; it should just return zero if things are not working.
// Oblige them.
// It still might log a warning for user first time it's invoked
C10_CUDA_API DeviceIndex device_count() noexcept;

// Version of device_count that throws is no devices are detected
C10_CUDA_API DeviceIndex device_count_ensure_non_zero();

C10_CUDA_API DeviceIndex current_device();

C10_CUDA_API void set_device(DeviceIndex device);

C10_CUDA_API void device_synchronize();

C10_CUDA_API void warn_or_error_on_sync();

enum class SyncWarningLevel {
  L_DISABLED = 0,
  L_WARN,
  L_ERROR
};

class WarningState {
  public:

    C10_CUDA_API void set_sync_warning_level(SyncWarningLevel l) {
      sync_warning_level = l;
    }

    SyncWarningLevel get_sync_warning_level(){
      return sync_warning_level;
    }

  private:
    SyncWarningLevel sync_warning_level=SyncWarningLevel::L_DISABLED;

};

//Make it CUDAWarningState class static member function to inline?
C10_CUDA_API WarningState& warning_state();

// the subsequent functions are defined in the header because for performance
// reasons we want them to be inline
C10_CUDA_API void __inline__ memcpy_and_sync(
    void* dst,
    void* src,
    int64_t nbytes,
    cudaMemcpyKind kind,
    cudaStream_t stream) {
  //here's uninlined call to warning_state that will be made even in the fast case
  if (C10_UNLIKELY(warning_state().get_sync_warning_level() != SyncWarningLevel::L_DISABLED)){
     warn_or_error_on_sync();
  }
#if defined(HIP_VERSION) && (HIP_VERSION >= 301)
  C10_CUDA_CHECK(hipMemcpyWithStream(dst, src, nbytes, kind, stream));
#else
  C10_CUDA_CHECK(cudaMemcpyAsync(dst, src, nbytes, kind, stream));
  C10_CUDA_CHECK(cudaStreamSynchronize(stream));
#endif
}

C10_CUDA_API void __inline__ stream_synchronize(cudaStream_t stream) {
  if (C10_UNLIKELY(warning_state().get_sync_warning_level() != SyncWarningLevel::L_DISABLED)){
     warn_or_error_on_sync();
  }
  C10_CUDA_CHECK(cudaStreamSynchronize(stream));
}

} // namespace cuda
} // namespace c10
