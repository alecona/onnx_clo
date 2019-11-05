// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "core/graph/basic_types.h"
#include "core/framework/allocator.h"
#include "core/framework/data_types.h"
#include "core/framework/framework_common.h"
#include "core/framework/iexecutor.h"
#include "core/framework/session_state.h"
#include "core/framework/session_options.h"

namespace ONNX_NAMESPACE {
class TensorShapeProto;
class TensorProto;
std::ostream& operator<<(std::ostream& out, const TensorShapeProto& shape_proto);
std::ostream& operator<<(std::ostream& out, const TensorProto& tensor_proto);
}  // namespace ONNX_NAMESPACE

namespace onnxruntime {
class ExecutionProviders;
struct FeedsFetchesInfo;
class FeedsFetchesManager;
struct MLValueCopyInfo;
class Graph;
class KernelDef;
class KernelRegistryManager;
class IExecutionProvider;
class Node;
class Tensor;

namespace logging {
class Logger;
}

namespace utils {
void* DefaultAlloc(size_t size);
void DefaultFree(void* p);

AllocatorPtr GetAllocator(const SessionState& session_state, const OrtMemoryInfo& memory_info);

common::Status AllocateHelper(const IExecutionProvider& execution_provider, int device_id, const Tensor& fetched_tensor,
                              OrtValue& output_mlvalue);

const std::string& GetNodeInputProviderType(const SessionState::NodeInfo& info);

common::Status CopyOneInputAcrossDevices(const SessionState& session_state, const std::string& input_name,
                                         const OrtValue& orig_mlvalue, OrtValue& new_mlvalue);

// Searches the allocation plan from the session_state to find the OrtMemoryInfo for the value 'name'.
const OrtMemoryInfo& FindMemoryInfoForValue(const SessionState& session_state,
                                            const std::string& name);

// Initialize the feed and fetch copy info using session_state.
// Determines the device that each graph input that will be fed will be consumed on,
// and the device that each graph output that will be fetched will be created on.
common::Status InitializeFeedFetchCopyInfo(const SessionState& session_state,
                                           FeedsFetchesManager& feeds_fetches_manager);

// Finalize the feed and fetch copy info using session_state and the device and location information from the feeds
// and fetches that will be used in graph execution.
void FinalizeFeedFetchCopyInfo(const SessionState& session_state,
                               FeedsFetchesManager& feeds_fetches_manager,
                               const std::vector<OrtDevice>& feed_locations,
                               const std::vector<const OrtMemoryInfo*>& fetch_alloc_info);

// Execute the main graph. The feed_fetches_manager will be finalized based on the provided feeds and fetches.
common::Status ExecuteGraph(const SessionState& session_state, FeedsFetchesManager& feeds_fetches_manager,
                            const std::vector<OrtValue>& feeds, std::vector<OrtValue>& fetches,
                            ExecutionMode execution_mode, const bool& terminate_flag, const logging::Logger& logger);

// Execute a subgraph. The feeds_fetches_manager should have been finalized prior to calling this function.
// See IControlFlowNode::SetupSubgraphExecutionInfo usage in the control flow kernels.
common::Status ExecuteSubgraph(const SessionState& session_state, const FeedsFetchesManager& feeds_fetches_manager,
                               const std::vector<OrtValue>& feeds, std::vector<OrtValue>& fetches,
                               const std::unordered_map<size_t, IExecutor::CustomAllocator>& fetch_allocators,
                               ExecutionMode execution_mode, const bool& terminate_flag, const logging::Logger& logger);

#if defined(DEBUG_NODE_INPUTS_OUTPUTS)
// to create a build with these enabled run the build script with 1 to dump just shapes, or 2 to dump shapes and data
// e.g.
//   --cmake_extra_defines onnxruntime_DEBUG_NODE_INPUTS_OUTPUTS=1
// To unset you'll need to either delete CMakeCache.txt or run with
//   --cmake_extra_defines onnxruntime_DEBUG_NODE_INPUTS_OUTPUTS=0
void DumpNodeInputs(const OpKernelContext& context, const Node& node);
void DumpNodeOutputs(OpKernelContext& context, const Node& node, const SessionState& session_state);
#endif

template <typename T>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType();

template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<float>() {
  return ONNX_NAMESPACE::TensorProto_DataType_FLOAT;
}
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<uint8_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_UINT8;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<int8_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_INT8;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<uint16_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_UINT16;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<int16_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_INT16;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<int32_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_INT32;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<int64_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_INT64;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<std::string>() {
  return ONNX_NAMESPACE::TensorProto_DataType_STRING;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<bool>() {
  return ONNX_NAMESPACE::TensorProto_DataType_BOOL;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<MLFloat16>() {
  return ONNX_NAMESPACE::TensorProto_DataType_FLOAT16;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<double>() {
  return ONNX_NAMESPACE::TensorProto_DataType_DOUBLE;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<uint32_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_UINT32;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<uint64_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_UINT64;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorProtoElementType<BFloat16>() {
  return ONNX_NAMESPACE::TensorProto_DataType_BFLOAT16;
};

template <typename T>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<bool>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<std::string>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<float>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<double>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<MLFloat16>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<BFloat16>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_BFLOAT16;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<int8_t>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<uint8_t>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<int16_t>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<uint16_t>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<int32_t>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<uint32_t>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<int64_t>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64;
}

template <>
constexpr ONNXTensorElementDataType GetONNXTensorElementDataType<uint64_t>() {
  return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64;
}

#define DispatchOnTensorType(tensor_type, function, ...)                \
  switch (tensor_type->AsPrimitiveDataType()->GetDataType()) { \
    case ONNX_NAMESPACE::TensorProto_DataType_FLOAT:                    \
      function<float>(__VA_ARGS__);                                     \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_BOOL:                     \
      function<bool>(__VA_ARGS__);                                      \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_DOUBLE:                   \
      function<double>(__VA_ARGS__);                                    \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_STRING:                   \
      function<std::string>(__VA_ARGS__);                               \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_INT8:                     \
      function<int8_t>(__VA_ARGS__);                                    \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_UINT8:                    \
      function<uint32_t>(__VA_ARGS__);                                  \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_INT16:                    \
      function<int16_t>(__VA_ARGS__);                                   \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_UINT16:                   \
      function<uint16_t>(__VA_ARGS__);                                  \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_INT32:                    \
      function<int32_t>(__VA_ARGS__);                                   \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_UINT32:                   \
      function<uint32_t>(__VA_ARGS__);                                  \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_INT64:                    \
      function<int64_t>(__VA_ARGS__);                                   \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_UINT64:                   \
      function<uint64_t>(__VA_ARGS__);                                  \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_FLOAT16:                  \
      function<MLFloat16>(__VA_ARGS__);                                 \
      break;                                                            \
    case ONNX_NAMESPACE::TensorProto_DataType_BFLOAT16:                 \
      function<BFloat16>(__VA_ARGS__);                                  \
      break;                                                            \
    default:                                                            \
      ORT_ENFORCE(false, "Unknown tensor type of ", tensor_type);       \
  }

#define DispatchOnTensorTypeWithReturn(tensor_type, retval, function, ...) \
  switch (tensor_type->AsPrimitiveDataType()->GetDataType()) {    \
    case ONNX_NAMESPACE::TensorProto_DataType_FLOAT:                       \
      retval = function<float>(__VA_ARGS__);                               \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_BOOL:                        \
      retval = function<bool>(__VA_ARGS__);                                \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_DOUBLE:                      \
      retval = function<double>(__VA_ARGS__);                              \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_STRING:                      \
      retval = function<std::string>(__VA_ARGS__);                         \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_INT8:                        \
      retval = function<int8_t>(__VA_ARGS__);                              \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_UINT8:                       \
      retval = function<uint8_t>(__VA_ARGS__);                             \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_UINT16:                      \
      retval = function<uint16_t>(__VA_ARGS__);                            \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_INT16:                       \
      retval = function<int16_t>(__VA_ARGS__);                             \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_INT32:                       \
      retval = function<int32_t>(__VA_ARGS__);                             \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_UINT32:                      \
      retval = function<uint32_t>(__VA_ARGS__);                            \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_INT64:                       \
      retval = function<int64_t>(__VA_ARGS__);                             \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_UINT64:                      \
      retval = function<uint64_t>(__VA_ARGS__);                            \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_FLOAT16:                     \
      retval = function<MLFloat16>(__VA_ARGS__);                           \
      break;                                                               \
    case ONNX_NAMESPACE::TensorProto_DataType_BFLOAT16:                    \
      retval = function<BFloat16>(__VA_ARGS__);                            \
      break;                                                               \
    default:                                                               \
      ORT_ENFORCE(false, "Unknown tensor type of ", tensor_type);          \
  }

// This is a frequently used check so we make a separate utility function.
inline bool IsDataTypeString(MLDataType dt_type) {
  auto prim_type = dt_type->AsPrimitiveDataType();
  return (prim_type != nullptr && prim_type->GetDataType() == ONNX_NAMESPACE::TensorProto_DataType_STRING);
}

// Test if MLDataType is a concrete type of PrimitiveDataTypeBase
// and it is T
template<class T>
inline bool IsPrimitiveDataType(MLDataType dt_type) {
  auto prim_type = dt_type->AsPrimitiveDataType();
  return (prim_type != nullptr && prim_type->GetDataType() == ToTensorProtoElementType<T>());
}

// Use after AsPrimitiveDataType() is successful
// Check if PrimitiveDataTypeBase is of type T
template<class T>
inline bool IsPrimitiveDataType (const PrimitiveDataTypeBase* prim_type) {
  assert(prim_type != nullptr);
  return prim_type->GetDataType() == ToTensorProtoElementType<T>();
}

}  // namespace utils
}  // namespace onnxruntime
