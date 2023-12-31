// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********
//     See cpp_generator.py for modifications
// ************************************************************

/*
** Copyright (c) 2017-2021 The Khronos Group Inc.
** Copyright (c) 2019-2021 Collabora, Ltd.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** ---- Exceptions to the Apache 2.0 License: ----
**
** As an exception, if you use this Software to generate code and portions of
** this Software are embedded into the generated code as a result, you may
** redistribute such product without providing attribution as would otherwise
** be required by Sections 4(a), 4(b) and 4(d) of the License.
**
** In addition, if you combine or link code generated by this Software with
** software that is licensed under the GPLv2 or the LGPL v2.0 or 2.1
** ("`Combined Software`") and if a court of competent jurisdiction determines
** that the patent provision (Section 3), the indemnity provision (Section 9)
** or other Section of the License conflicts with the conditions of the
** applicable GPL or LGPL license, you may retroactively and prospectively
** choose to deem waived or otherwise exclude such Section(s) of the License,
** but only in their entirety and only with respect to the Combined Software.
**
*/

/*
** This header is generated from the Khronos OpenXR XML API Registry.
**
*/
#ifndef OPENXR_HANDLES_FORWARD_HPP_
#define OPENXR_HANDLES_FORWARD_HPP_
/**
 * @file
 * @brief Forward declarations of OpenXR handle wrapper types.
 *
 * @see openxr_handles.hpp
 * @ingroup handles
 */

#include "openxr_enums.hpp"

#if !defined(OPENXR_HPP_NAMESPACE)
#define OPENXR_HPP_NAMESPACE xr
#endif  // !OPENXR_HPP_NAMESPACE

namespace OPENXR_HPP_NAMESPACE {

#ifndef OPENXR_HPP_NO_SMART_HANDLE

namespace traits {

  template <typename Type, typename Dispatch>
  class UniqueHandleTraits;

}  // namespace traits

namespace impl {

  // Used when returning unique handles.
  template <typename T>
  using RemoveRefConst = typename std::remove_const<typename std::remove_reference<T>::type>::type;
}  // namespace impl

/*!
 * @brief Template class for holding a handle with unique ownership, much like unique_ptr.
 *
 * Note that this does not keep track of children or parents, though OpenXR specifies that
 * destruction of a handle also destroys its children automatically. Thus, it is important to order
 * destruction of these correctly, usually by ordering declarations.
 *
 * Inherits from the deleter to use empty-base-class optimization when possible.
 */
template <typename Type, typename Dispatch>
class UniqueHandle : public traits::UniqueHandleTraits<Type, Dispatch>::deleter {
private:
  using Deleter = typename traits::UniqueHandleTraits<Type, Dispatch>::deleter;

public:
  //! Explicit constructor with deleter.
  explicit UniqueHandle(Type const &value = Type(), Deleter const &deleter = Deleter())
      : Deleter(deleter), m_value(value) {}

  // Cannot copy
  UniqueHandle(UniqueHandle const &) = delete;

  //! Move constructor
  UniqueHandle(UniqueHandle &&other)
      : Deleter(std::move(static_cast<Deleter &>(other))), m_value(other.release()) {}

  //! Destructor: destroys owned handle if valid.
  ~UniqueHandle() {
    if (m_value) this->destroy(m_value);
  }

  // cannot copy-assign
  UniqueHandle &operator=(UniqueHandle const &) = delete;

  //! Move-assignment operator.
  UniqueHandle &operator=(UniqueHandle &&other) {
    reset(other.release());
    *static_cast<Deleter *>(this) = std::move(static_cast<Deleter &>(other));
    return *this;
  }

  //! Explicit bool conversion: for testing if the handle is valid.
  explicit operator bool() const { return m_value.operator bool(); }

  // Smart pointer operator
  Type const *operator->() const { return &m_value; }

  // Smart pointer operator
  Type *operator->() { return &m_value; }

  // Smart pointer operator
  Type const &operator*() const { return m_value; }

  // Smart pointer operator
  Type &operator*() { return m_value; }

  //! Get the underlying (wrapped) handle type.
  const Type &get() const { return m_value; }

  //! Get the underlying (wrapped) handle type.
  Type &get() { return m_value; }

  //! Get the raw OpenXR handle or XR_NULL_HANDLE
  typename Type::RawHandleType getRawHandle() { return m_value ? m_value.get() : XR_NULL_HANDLE; }

  //! Clear or re-assign the underlying handle
  void reset(Type const &value = Type()) {
    if (m_value != value) {
      if (m_value) this->destroy(m_value);
      m_value = value;
    }
  }

  //! Clear this handle and return a pointer to the storage, for assignment/creation purposes.
  typename Type::RawHandleType *put() {
    reset();
    return m_value.put();
  }

  //! Relinquish ownership of the contained handle and return it without destroying it.
  Type release() {
    Type value = m_value;
    m_value = nullptr;
    return value;
  }

  //! Swap with another handle of this type.
  void swap(UniqueHandle<Type, Dispatch> &rhs) {
    std::swap(m_value, rhs.m_value);
    std::swap(static_cast<Deleter &>(*this), static_cast<Deleter &>(rhs));
  }

private:
  Type m_value;
};

template <typename Type, typename Dispatch>
class UniqueHandle<Type, Dispatch &> : public UniqueHandle<Type, Dispatch> {};

template <typename Type, typename Dispatch>
class UniqueHandle<Type, Dispatch const> : public UniqueHandle<Type, Dispatch> {};

//! @relates UniqueHandle
template <typename Type, typename Dispatch>
OPENXR_HPP_INLINE void swap(UniqueHandle<Type, Dispatch> &lhs, UniqueHandle<Type, Dispatch> &rhs) {
  lhs.swap(rhs);
}

//! @relates UniqueHandle
template <typename Type, typename Dispatch>
OPENXR_HPP_INLINE const Type &get(const UniqueHandle<Type, Dispatch> &h) {
  return h.get();
}

//! @brief Equality comparison between two UniqueHandles, potentially of different dispatch.
//! @relates UniqueHandle
template <typename Type, typename D1, typename D2>
OPENXR_HPP_CONSTEXPR OPENXR_HPP_INLINE bool operator==(UniqueHandle<Type, D1> const &lhs,
                                                       UniqueHandle<Type, D2> const &rhs) {
  return lhs.get() == rhs.get();
}
//! @brief Inequality comparison between two UniqueHandles, potentially of different dispatch.
//! @relates UniqueHandle
template <typename Type, typename D1, typename D2>
OPENXR_HPP_CONSTEXPR OPENXR_HPP_INLINE bool operator!=(UniqueHandle<Type, D1> const &lhs,
                                                       UniqueHandle<Type, D2> const &rhs) {
  return lhs.get() != rhs.get();
}
//! @brief Equality comparison between UniqueHandle and nullptr: true if the handle is
//! null.
//! @relates UniqueHandle
template <typename Type, typename Dispatch>
OPENXR_HPP_CONSTEXPR OPENXR_HPP_INLINE bool operator==(UniqueHandle<Type, Dispatch> const &lhs,
                                                       std::nullptr_t /* unused */) {
  return lhs.get() == XR_NULL_HANDLE;
}
//! @brief Equality comparison between nullptr and UniqueHandle: true if the handle is
//! null.
//! @relates UniqueHandle
template <typename Type, typename Dispatch>
OPENXR_HPP_CONSTEXPR OPENXR_HPP_INLINE bool operator==(std::nullptr_t /* unused */,
                                                       UniqueHandle<Type, Dispatch> const &rhs) {
  return rhs.get() == XR_NULL_HANDLE;
}
//! @brief Inequality comparison between UniqueHandle and nullptr: true if the handle
//! is not null.
//! @relates UniqueHandle
template <typename Type, typename Dispatch>
OPENXR_HPP_CONSTEXPR OPENXR_HPP_INLINE bool operator!=(UniqueHandle<Type, Dispatch> const &lhs,
                                                       std::nullptr_t /* unused */) {
  return lhs.get() != XR_NULL_HANDLE;
}
//! @brief Inequality comparison between nullptr and UniqueHandle: true if the handle
//! is not null.
//! @relates UniqueHandle
template <typename Type, typename Dispatch>
OPENXR_HPP_CONSTEXPR OPENXR_HPP_INLINE bool operator!=(std::nullptr_t /* unused */,
                                                       UniqueHandle<Type, Dispatch> const &rhs) {
  return rhs.get() != XR_NULL_HANDLE;
}
#endif

template <typename Dispatch>
class ObjectDestroy {
public:
  ObjectDestroy(Dispatch const &dispatch = Dispatch()) : m_dispatch(&dispatch) {}

protected:
  template <typename T>
  void destroy(T t) {
    t.destroy(*m_dispatch);
  }

private:
  Dispatch const *m_dispatch;
};
}  // namespace OPENXR_HPP_NAMESPACE
namespace OPENXR_HPP_NAMESPACE {

namespace traits {
  //! Type trait associating an ObjectType enum value with its C++ type.
  template <ObjectType o>
  struct cpp_type_from_object_type_enum;
}  // namespace traits

// forward declarations

class DispatchLoaderStatic;
class DispatchLoaderDynamic;

class Instance;

class Session;

class Space;

class Action;

class Swapchain;

class ActionSet;

class DebugUtilsMessengerEXT;

class SpatialAnchorMSFT;

class HandTrackerEXT;

class SceneObserverMSFT;

class SceneMSFT;

class FacialTrackerHTC;

class FoveationProfileFB;

class TriangleMeshFB;

class PassthroughFB;

class PassthroughLayerFB;

class GeometryInstanceFB;

class SpatialAnchorStoreConnectionMSFT;
/*!
 * @defgroup handles Handle types
 * @brief Wrappers for OpenXR handle types, with associated functions mapped as methods.
 * @ingroup wrappers
 */
/*!
 * @defgroup unique_handle_aliases Aliases for UniqueHandle types
 * @brief Convenience names for specializations of UniqueHandle<>
 * @ingroup handles
 */

#ifndef OPENXR_HPP_NO_SMART_HANDLE
#ifndef OPENXR_HPP_DOXYGEN
namespace traits {
  template <typename Dispatch>
  class UniqueHandleTraits<Instance, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<Session, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<Space, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<Action, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<Swapchain, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<ActionSet, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<DebugUtilsMessengerEXT, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<SpatialAnchorMSFT, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<HandTrackerEXT, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<SceneObserverMSFT, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<SceneMSFT, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<FacialTrackerHTC, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<FoveationProfileFB, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<TriangleMeshFB, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<PassthroughFB, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<PassthroughLayerFB, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<GeometryInstanceFB, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
  template <typename Dispatch>
  class UniqueHandleTraits<SpatialAnchorStoreConnectionMSFT, Dispatch> {
  public:
    using deleter = ObjectDestroy<Dispatch>;
  };
}  // namespace traits
#endif  // !OPENXR_HPP_DOXYGEN

//! @addtogroup unique_handle_aliases
//! @{

//! Shorthand name for unique handles of type Instance, using a static dispatch.
using UniqueInstance = UniqueHandle<Instance, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type Instance, using a dynamic dispatch.
using UniqueDynamicInstance = UniqueHandle<Instance, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type Session, using a static dispatch.
using UniqueSession = UniqueHandle<Session, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type Session, using a dynamic dispatch.
using UniqueDynamicSession = UniqueHandle<Session, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type Space, using a static dispatch.
using UniqueSpace = UniqueHandle<Space, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type Space, using a dynamic dispatch.
using UniqueDynamicSpace = UniqueHandle<Space, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type Action, using a static dispatch.
using UniqueAction = UniqueHandle<Action, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type Action, using a dynamic dispatch.
using UniqueDynamicAction = UniqueHandle<Action, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type Swapchain, using a static dispatch.
using UniqueSwapchain = UniqueHandle<Swapchain, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type Swapchain, using a dynamic dispatch.
using UniqueDynamicSwapchain = UniqueHandle<Swapchain, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type ActionSet, using a static dispatch.
using UniqueActionSet = UniqueHandle<ActionSet, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type ActionSet, using a dynamic dispatch.
using UniqueDynamicActionSet = UniqueHandle<ActionSet, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type DebugUtilsMessengerEXT, using a static dispatch.
using UniqueDebugUtilsMessengerEXT = UniqueHandle<DebugUtilsMessengerEXT, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type DebugUtilsMessengerEXT, using a dynamic dispatch.
using UniqueDynamicDebugUtilsMessengerEXT =
    UniqueHandle<DebugUtilsMessengerEXT, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type SpatialAnchorMSFT, using a static dispatch.
using UniqueSpatialAnchorMSFT = UniqueHandle<SpatialAnchorMSFT, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type SpatialAnchorMSFT, using a dynamic dispatch.
using UniqueDynamicSpatialAnchorMSFT = UniqueHandle<SpatialAnchorMSFT, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type HandTrackerEXT, using a static dispatch.
using UniqueHandTrackerEXT = UniqueHandle<HandTrackerEXT, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type HandTrackerEXT, using a dynamic dispatch.
using UniqueDynamicHandTrackerEXT = UniqueHandle<HandTrackerEXT, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type SceneObserverMSFT, using a static dispatch.
using UniqueSceneObserverMSFT = UniqueHandle<SceneObserverMSFT, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type SceneObserverMSFT, using a dynamic dispatch.
using UniqueDynamicSceneObserverMSFT = UniqueHandle<SceneObserverMSFT, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type SceneMSFT, using a static dispatch.
using UniqueSceneMSFT = UniqueHandle<SceneMSFT, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type SceneMSFT, using a dynamic dispatch.
using UniqueDynamicSceneMSFT = UniqueHandle<SceneMSFT, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type FacialTrackerHTC, using a static dispatch.
using UniqueFacialTrackerHTC = UniqueHandle<FacialTrackerHTC, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type FacialTrackerHTC, using a dynamic dispatch.
using UniqueDynamicFacialTrackerHTC = UniqueHandle<FacialTrackerHTC, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type FoveationProfileFB, using a static dispatch.
using UniqueFoveationProfileFB = UniqueHandle<FoveationProfileFB, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type FoveationProfileFB, using a dynamic dispatch.
using UniqueDynamicFoveationProfileFB = UniqueHandle<FoveationProfileFB, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type TriangleMeshFB, using a static dispatch.
using UniqueTriangleMeshFB = UniqueHandle<TriangleMeshFB, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type TriangleMeshFB, using a dynamic dispatch.
using UniqueDynamicTriangleMeshFB = UniqueHandle<TriangleMeshFB, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type PassthroughFB, using a static dispatch.
using UniquePassthroughFB = UniqueHandle<PassthroughFB, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type PassthroughFB, using a dynamic dispatch.
using UniqueDynamicPassthroughFB = UniqueHandle<PassthroughFB, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type PassthroughLayerFB, using a static dispatch.
using UniquePassthroughLayerFB = UniqueHandle<PassthroughLayerFB, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type PassthroughLayerFB, using a dynamic dispatch.
using UniqueDynamicPassthroughLayerFB = UniqueHandle<PassthroughLayerFB, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type GeometryInstanceFB, using a static dispatch.
using UniqueGeometryInstanceFB = UniqueHandle<GeometryInstanceFB, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type GeometryInstanceFB, using a dynamic dispatch.
using UniqueDynamicGeometryInstanceFB = UniqueHandle<GeometryInstanceFB, DispatchLoaderDynamic>;
//! Shorthand name for unique handles of type SpatialAnchorStoreConnectionMSFT, using a static
//! dispatch.
using UniqueSpatialAnchorStoreConnectionMSFT =
    UniqueHandle<SpatialAnchorStoreConnectionMSFT, DispatchLoaderStatic>;
//! Shorthand name for unique handles of type SpatialAnchorStoreConnectionMSFT, using a dynamic
//! dispatch.
using UniqueDynamicSpatialAnchorStoreConnectionMSFT =
    UniqueHandle<SpatialAnchorStoreConnectionMSFT, DispatchLoaderDynamic>;
//! @}
#endif  // !OPENXR_HPP_NO_SMART_HANDLE

#ifndef OPENXR_HPP_DOXYGEN
namespace traits {
  // Explicit specializations of cpp_type_from_object_type_enum

  template <>
  struct cpp_type_from_object_type_enum<ObjectType::Instance> {
    using type = Instance;
  };

  template <>
  struct cpp_type_from_object_type_enum<ObjectType::Session> {
    using type = Session;
  };

  template <>
  struct cpp_type_from_object_type_enum<ObjectType::Space> {
    using type = Space;
  };

  template <>
  struct cpp_type_from_object_type_enum<ObjectType::Action> {
    using type = Action;
  };

  template <>
  struct cpp_type_from_object_type_enum<ObjectType::Swapchain> {
    using type = Swapchain;
  };

  template <>
  struct cpp_type_from_object_type_enum<ObjectType::ActionSet> {
    using type = ActionSet;
  };

#ifdef XR_EXT_debug_utils
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::DebugUtilsMessengerEXT> {
    using type = DebugUtilsMessengerEXT;
  };
#endif  // XR_EXT_debug_utils
#ifdef XR_MSFT_spatial_anchor
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::SpatialAnchorMSFT> {
    using type = SpatialAnchorMSFT;
  };
#endif  // XR_MSFT_spatial_anchor
#ifdef XR_EXT_hand_tracking
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::HandTrackerEXT> {
    using type = HandTrackerEXT;
  };
#endif  // XR_EXT_hand_tracking
#ifdef XR_MSFT_scene_understanding
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::SceneObserverMSFT> {
    using type = SceneObserverMSFT;
  };
#endif  // XR_MSFT_scene_understanding
#ifdef XR_MSFT_scene_understanding
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::SceneMSFT> {
    using type = SceneMSFT;
  };
#endif  // XR_MSFT_scene_understanding
#ifdef XR_HTC_facial_tracking
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::FacialTrackerHTC> {
    using type = FacialTrackerHTC;
  };
#endif  // XR_HTC_facial_tracking
#ifdef XR_FB_foveation
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::FoveationProfileFB> {
    using type = FoveationProfileFB;
  };
#endif  // XR_FB_foveation
#ifdef XR_FB_triangle_mesh
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::TriangleMeshFB> {
    using type = TriangleMeshFB;
  };
#endif  // XR_FB_triangle_mesh
#ifdef XR_FB_passthrough
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::PassthroughFB> {
    using type = PassthroughFB;
  };
#endif  // XR_FB_passthrough
#ifdef XR_FB_passthrough
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::PassthroughLayerFB> {
    using type = PassthroughLayerFB;
  };
#endif  // XR_FB_passthrough
#ifdef XR_FB_passthrough
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::GeometryInstanceFB> {
    using type = GeometryInstanceFB;
  };
#endif  // XR_FB_passthrough
#ifdef XR_MSFT_spatial_anchor_persistence
  template <>
  struct cpp_type_from_object_type_enum<ObjectType::SpatialAnchorStoreConnectionMSFT> {
    using type = SpatialAnchorStoreConnectionMSFT;
  };
#endif  // XR_MSFT_spatial_anchor_persistence
}  // namespace traits
#endif  // !OPENXR_HPP_DOXYGEN

}  // namespace OPENXR_HPP_NAMESPACE

#endif  // ifndef OPENXR_HANDLES_FORWARD_HPP_
