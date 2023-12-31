// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: http_log.capnp

#pragma once

#include <capnp/generated-header-support.h>
#include <kj/windows-sanity.h>

#ifndef CAPNP_VERSION
#error "CAPNP_VERSION is not defined, is capnp/generated-header-support.h missing?"
#elif CAPNP_VERSION != 1000001
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif


CAPNP_BEGIN_HEADER

namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(c5c66f22213ada4b);

}  // namespace schemas
}  // namespace capnp


struct HttpLogRecord {
  HttpLogRecord() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(c5c66f22213ada4b, 5, 4)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

class HttpLogRecord::Reader {
public:
  typedef HttpLogRecord Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline  ::uint64_t getTimestampEpochMilli() const;

  inline  ::uint64_t getResourceId() const;

  inline  ::uint64_t getBytesSent() const;

  inline  ::uint64_t getRequestTimeMilli() const;

  inline  ::uint16_t getResponseStatus() const;

  inline bool hasCacheStatus() const;
  inline  ::capnp::Text::Reader getCacheStatus() const;

  inline bool hasMethod() const;
  inline  ::capnp::Text::Reader getMethod() const;

  inline bool hasRemoteAddr() const;
  inline  ::capnp::Text::Reader getRemoteAddr() const;

  inline bool hasUrl() const;
  inline  ::capnp::Text::Reader getUrl() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class HttpLogRecord::Builder {
public:
  typedef HttpLogRecord Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::uint64_t getTimestampEpochMilli();
  inline void setTimestampEpochMilli( ::uint64_t value);

  inline  ::uint64_t getResourceId();
  inline void setResourceId( ::uint64_t value);

  inline  ::uint64_t getBytesSent();
  inline void setBytesSent( ::uint64_t value);

  inline  ::uint64_t getRequestTimeMilli();
  inline void setRequestTimeMilli( ::uint64_t value);

  inline  ::uint16_t getResponseStatus();
  inline void setResponseStatus( ::uint16_t value);

  inline bool hasCacheStatus();
  inline  ::capnp::Text::Builder getCacheStatus();
  inline void setCacheStatus( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initCacheStatus(unsigned int size);
  inline void adoptCacheStatus(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownCacheStatus();

  inline bool hasMethod();
  inline  ::capnp::Text::Builder getMethod();
  inline void setMethod( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initMethod(unsigned int size);
  inline void adoptMethod(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownMethod();

  inline bool hasRemoteAddr();
  inline  ::capnp::Text::Builder getRemoteAddr();
  inline void setRemoteAddr( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initRemoteAddr(unsigned int size);
  inline void adoptRemoteAddr(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownRemoteAddr();

  inline bool hasUrl();
  inline  ::capnp::Text::Builder getUrl();
  inline void setUrl( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initUrl(unsigned int size);
  inline void adoptUrl(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownUrl();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class HttpLogRecord::Pipeline {
public:
  typedef HttpLogRecord Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

// =======================================================================================

inline  ::uint64_t HttpLogRecord::Reader::getTimestampEpochMilli() const {
  return _reader.getDataField< ::uint64_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline  ::uint64_t HttpLogRecord::Builder::getTimestampEpochMilli() {
  return _builder.getDataField< ::uint64_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline void HttpLogRecord::Builder::setTimestampEpochMilli( ::uint64_t value) {
  _builder.setDataField< ::uint64_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value);
}

inline  ::uint64_t HttpLogRecord::Reader::getResourceId() const {
  return _reader.getDataField< ::uint64_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS);
}

inline  ::uint64_t HttpLogRecord::Builder::getResourceId() {
  return _builder.getDataField< ::uint64_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS);
}
inline void HttpLogRecord::Builder::setResourceId( ::uint64_t value) {
  _builder.setDataField< ::uint64_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS, value);
}

inline  ::uint64_t HttpLogRecord::Reader::getBytesSent() const {
  return _reader.getDataField< ::uint64_t>(
      ::capnp::bounded<2>() * ::capnp::ELEMENTS);
}

inline  ::uint64_t HttpLogRecord::Builder::getBytesSent() {
  return _builder.getDataField< ::uint64_t>(
      ::capnp::bounded<2>() * ::capnp::ELEMENTS);
}
inline void HttpLogRecord::Builder::setBytesSent( ::uint64_t value) {
  _builder.setDataField< ::uint64_t>(
      ::capnp::bounded<2>() * ::capnp::ELEMENTS, value);
}

inline  ::uint64_t HttpLogRecord::Reader::getRequestTimeMilli() const {
  return _reader.getDataField< ::uint64_t>(
      ::capnp::bounded<3>() * ::capnp::ELEMENTS);
}

inline  ::uint64_t HttpLogRecord::Builder::getRequestTimeMilli() {
  return _builder.getDataField< ::uint64_t>(
      ::capnp::bounded<3>() * ::capnp::ELEMENTS);
}
inline void HttpLogRecord::Builder::setRequestTimeMilli( ::uint64_t value) {
  _builder.setDataField< ::uint64_t>(
      ::capnp::bounded<3>() * ::capnp::ELEMENTS, value);
}

inline  ::uint16_t HttpLogRecord::Reader::getResponseStatus() const {
  return _reader.getDataField< ::uint16_t>(
      ::capnp::bounded<16>() * ::capnp::ELEMENTS);
}

inline  ::uint16_t HttpLogRecord::Builder::getResponseStatus() {
  return _builder.getDataField< ::uint16_t>(
      ::capnp::bounded<16>() * ::capnp::ELEMENTS);
}
inline void HttpLogRecord::Builder::setResponseStatus( ::uint16_t value) {
  _builder.setDataField< ::uint16_t>(
      ::capnp::bounded<16>() * ::capnp::ELEMENTS, value);
}

inline bool HttpLogRecord::Reader::hasCacheStatus() const {
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool HttpLogRecord::Builder::hasCacheStatus() {
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader HttpLogRecord::Reader::getCacheStatus() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder HttpLogRecord::Builder::getCacheStatus() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline void HttpLogRecord::Builder::setCacheStatus( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder HttpLogRecord::Builder::initCacheStatus(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), size);
}
inline void HttpLogRecord::Builder::adoptCacheStatus(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> HttpLogRecord::Builder::disownCacheStatus() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}

inline bool HttpLogRecord::Reader::hasMethod() const {
  return !_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline bool HttpLogRecord::Builder::hasMethod() {
  return !_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader HttpLogRecord::Reader::getMethod() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder HttpLogRecord::Builder::getMethod() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline void HttpLogRecord::Builder::setMethod( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder HttpLogRecord::Builder::initMethod(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), size);
}
inline void HttpLogRecord::Builder::adoptMethod(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> HttpLogRecord::Builder::disownMethod() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}

inline bool HttpLogRecord::Reader::hasRemoteAddr() const {
  return !_reader.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS).isNull();
}
inline bool HttpLogRecord::Builder::hasRemoteAddr() {
  return !_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader HttpLogRecord::Reader::getRemoteAddr() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder HttpLogRecord::Builder::getRemoteAddr() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS));
}
inline void HttpLogRecord::Builder::setRemoteAddr( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder HttpLogRecord::Builder::initRemoteAddr(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS), size);
}
inline void HttpLogRecord::Builder::adoptRemoteAddr(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> HttpLogRecord::Builder::disownRemoteAddr() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS));
}

inline bool HttpLogRecord::Reader::hasUrl() const {
  return !_reader.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS).isNull();
}
inline bool HttpLogRecord::Builder::hasUrl() {
  return !_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader HttpLogRecord::Reader::getUrl() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder HttpLogRecord::Builder::getUrl() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS));
}
inline void HttpLogRecord::Builder::setUrl( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder HttpLogRecord::Builder::initUrl(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS), size);
}
inline void HttpLogRecord::Builder::adoptUrl(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> HttpLogRecord::Builder::disownUrl() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<3>() * ::capnp::POINTERS));
}


CAPNP_END_HEADER

