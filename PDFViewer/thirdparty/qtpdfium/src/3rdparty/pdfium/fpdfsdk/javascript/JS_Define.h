// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_JS_DEFINE_H_
#define FPDFSDK_JAVASCRIPT_JS_DEFINE_H_

#include <vector>

#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/resource.h"
#include "fxjs/fxjs_v8.h"

struct JSConstSpec {
  const wchar_t* pName;
  double number;
  const wchar_t* str;
  uint8_t t;              // 0:double 1:str
};

struct JSPropertySpec {
  const wchar_t* pName;
  v8::AccessorGetterCallback pPropGet;
  v8::AccessorSetterCallback pPropPut;
};

struct JSMethodSpec {
  const wchar_t* pName;
  v8::FunctionCallback pMethodCall;
};

#define JS_WIDESTRING(widestring) L## #widestring
#define BEGIN_JS_STATIC_CONST(js_class_name) \
  JSConstSpec js_class_name::JS_Class_Consts[] = {
#define JS_STATIC_CONST_ENTRY_NUMBER(const_name, pValue) \
  { const_name, pValue, L"", 0 }                         \
  ,

#define JS_STATIC_CONST_ENTRY_STRING(const_name, pValue) \
  { const_name, 0, pValue, 1 }                           \
  ,

#define END_JS_STATIC_CONST() \
  { 0, 0, 0, 0 }              \
  }                           \
  ;  // NOLINT

#define BEGIN_JS_STATIC_PROP(js_class_name) \
  JSPropertySpec js_class_name::JS_Class_Properties[] = {
#define JS_STATIC_PROP_ENTRY(prop_name)                \
  {JS_WIDESTRING(prop_name), get_##prop_name##_static, \
   set_##prop_name##_static},  // NOLINT

#define END_JS_STATIC_PROP() \
  { 0, 0, 0 }                \
  }                          \
  ;  // NOLINT

#define BEGIN_JS_STATIC_METHOD(js_class_name) \
  JSMethodSpec js_class_name::JS_Class_Methods[] = {
#define JS_STATIC_METHOD_ENTRY(method_name)            \
  { JS_WIDESTRING(method_name), method_name##_static } \
  ,

#define END_JS_STATIC_METHOD() \
  { 0, 0 }                     \
  }                            \
  ;  // NOLINT

template <class C, bool (C::*M)(IJS_Context*, CJS_PropValue&, CFX_WideString&)>
void JSPropGetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  const v8::PropertyCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;
  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CFX_WideString sError;
  CJS_PropValue value(pRuntime);
  value.StartGetting();
  if (!(pObj->*M)(pRuntime->GetCurrentContext(), value, sError)) {
    pRuntime->Error(
        JSFormatErrorString(class_name_string, prop_name_string, sError));
    return;
  }
  info.GetReturnValue().Set(value.GetJSValue()->ToV8Value(pRuntime));
}

template <class C, bool (C::*M)(IJS_Context*, CJS_PropValue&, CFX_WideString&)>
void JSPropSetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  v8::Local<v8::Value> value,
                  const v8::PropertyCallbackInfo<void>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;
  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CFX_WideString sError;
  CJS_PropValue propValue(pRuntime, CJS_Value(pRuntime, value));
  propValue.StartSetting();
  if (!(pObj->*M)(pRuntime->GetCurrentContext(), propValue, sError)) {
    pRuntime->Error(
        JSFormatErrorString(class_name_string, prop_name_string, sError));
  }
}

#define JS_STATIC_PROP(prop_name, class_name)                                 \
  static void get_##prop_name##_static(                                       \
      v8::Local<v8::String> property,                                         \
      const v8::PropertyCallbackInfo<v8::Value>& info) {                      \
    JSPropGetter<class_name, &class_name::prop_name>(#prop_name, #class_name, \
                                                     property, info);         \
  }                                                                           \
  static void set_##prop_name##_static(                                       \
      v8::Local<v8::String> property, v8::Local<v8::Value> value,             \
      const v8::PropertyCallbackInfo<void>& info) {                           \
    JSPropSetter<class_name, &class_name::prop_name>(#prop_name, #class_name, \
                                                     property, value, info);  \
  }

template <class C,
          bool (C::*M)(IJS_Context*,
                       const std::vector<CJS_Value>&,
                       CJS_Value&,
                       CFX_WideString&)>
void JSMethod(const char* method_name_string,
              const char* class_name_string,
              const v8::FunctionCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;
  std::vector<CJS_Value> parameters;
  for (unsigned int i = 0; i < (unsigned int)info.Length(); i++) {
    parameters.push_back(CJS_Value(pRuntime, info[i]));
  }
  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CFX_WideString sError;
  CJS_Value valueRes(pRuntime);
  if (!(pObj->*M)(pRuntime->GetCurrentContext(), parameters, valueRes,
                  sError)) {
    pRuntime->Error(
        JSFormatErrorString(class_name_string, method_name_string, sError));
    return;
  }
  info.GetReturnValue().Set(valueRes.ToV8Value(pRuntime));
}

#define JS_STATIC_METHOD(method_name, class_name)                             \
  static void method_name##_static(                                           \
      const v8::FunctionCallbackInfo<v8::Value>& info) {                      \
    JSMethod<class_name, &class_name::method_name>(#method_name, #class_name, \
                                                   info);                     \
  }

#define JS_SPECIAL_STATIC_METHOD(method_name, class_alternate, class_name) \
  static void method_name##_static(                                        \
      const v8::FunctionCallbackInfo<v8::Value>& info) {                   \
    JSMethod<class_alternate, &class_alternate::method_name>(              \
        #method_name, #class_name, info);                                  \
  }

// All JS classes have a name, an object defintion ID, and the ability to
// register themselves with FXJS_V8. We never make a BASE class on its own
// because it can't really do anything.
#define DECLARE_JS_CLASS_BASE_PART()  \
  static const wchar_t* g_pClassName; \
  static int g_nObjDefnID;            \
  static void DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType);

#define IMPLEMENT_JS_CLASS_BASE_PART(js_class_name, class_name)           \
  const wchar_t* js_class_name::g_pClassName = JS_WIDESTRING(class_name); \
  int js_class_name::g_nObjDefnID = -1;

// CONST classes provide constants, but not constructors, methods, or props.
#define DECLARE_JS_CLASS_CONST() \
  DECLARE_JS_CLASS_BASE_PART()   \
  DECLARE_JS_CLASS_CONST_PART()

#define IMPLEMENT_JS_CLASS_CONST(js_class_name, class_name)                  \
  IMPLEMENT_JS_CLASS_BASE_PART(js_class_name, class_name)                    \
  IMPLEMENT_JS_CLASS_CONST_PART(js_class_name, class_name)                   \
  void js_class_name::DefineJSObjects(CFXJS_Engine* pEngine,                 \
                                      FXJSOBJTYPE eObjType) {                \
    g_nObjDefnID = pEngine->DefineObj(js_class_name::g_pClassName, eObjType, \
                                      nullptr, nullptr);                     \
    DefineConsts(pEngine);                                                   \
  }

#define DECLARE_JS_CLASS_CONST_PART()   \
  static JSConstSpec JS_Class_Consts[]; \
  static void DefineConsts(CFXJS_Engine* pEngine);

#define IMPLEMENT_JS_CLASS_CONST_PART(js_class_name, class_name)     \
  void js_class_name::DefineConsts(CFXJS_Engine* pEngine) {          \
    for (size_t i = 0; i < FX_ArraySize(JS_Class_Consts) - 1; ++i) { \
      pEngine->DefineObjConst(                                       \
          g_nObjDefnID, JS_Class_Consts[i].pName,                    \
          JS_Class_Consts[i].t == 0                                  \
              ? pEngine->NewNumber(JS_Class_Consts[i].number)        \
              : pEngine->NewString(JS_Class_Consts[i].str));         \
    }                                                                \
  }

// Convenience macros for declaring classes without an alternate.
#define DECLARE_JS_CLASS() DECLARE_JS_CLASS_RICH()
#define IMPLEMENT_JS_CLASS(js_class_name, class_name) \
  IMPLEMENT_JS_CLASS_RICH(js_class_name, class_name, class_name)

// Rich JS classes provide constants, methods, properties, and the ability
// to construct native object state.
#define DECLARE_JS_CLASS_RICH() \
  DECLARE_JS_CLASS_BASE_PART()  \
  DECLARE_JS_CLASS_CONST_PART() \
  DECLARE_JS_CLASS_RICH_PART()

#define IMPLEMENT_JS_CLASS_RICH(js_class_name, class_alternate, class_name)  \
  IMPLEMENT_JS_CLASS_BASE_PART(js_class_name, class_name)                    \
  IMPLEMENT_JS_CLASS_CONST_PART(js_class_name, class_name)                   \
  IMPLEMENT_JS_CLASS_RICH_PART(js_class_name, class_alternate, class_name)   \
  void js_class_name::DefineJSObjects(CFXJS_Engine* pEngine,                 \
                                      FXJSOBJTYPE eObjType) {                \
    g_nObjDefnID = pEngine->DefineObj(js_class_name::g_pClassName, eObjType, \
                                      JSConstructor, JSDestructor);          \
    DefineConsts(pEngine);                                                   \
    DefineProps(pEngine);                                                    \
    DefineMethods(pEngine);                                                  \
  }

#define DECLARE_JS_CLASS_RICH_PART()                                           \
  static void JSConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj); \
  static void JSDestructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);  \
  static void DefineProps(CFXJS_Engine* pEngine);                              \
  static void DefineMethods(CFXJS_Engine* pEngine);                            \
  static JSPropertySpec JS_Class_Properties[];                                 \
  static JSMethodSpec JS_Class_Methods[];

#define IMPLEMENT_JS_CLASS_RICH_PART(js_class_name, class_alternate,         \
                                     class_name)                             \
  void js_class_name::JSConstructor(CFXJS_Engine* pEngine,                   \
                                    v8::Local<v8::Object> obj) {             \
    CJS_Object* pObj = new js_class_name(obj);                               \
    pObj->SetEmbedObject(new class_alternate(pObj));                         \
    pEngine->SetObjectPrivate(obj, (void*)pObj);                             \
    pObj->InitInstance(static_cast<CJS_Runtime*>(pEngine));                  \
  }                                                                          \
  void js_class_name::JSDestructor(CFXJS_Engine* pEngine,                    \
                                   v8::Local<v8::Object> obj) {              \
    js_class_name* pObj =                                                    \
        static_cast<js_class_name*>(pEngine->GetObjectPrivate(obj));         \
    delete pObj;                                                             \
  }                                                                          \
  void js_class_name::DefineProps(CFXJS_Engine* pEngine) {                   \
    for (size_t i = 0; i < FX_ArraySize(JS_Class_Properties) - 1; ++i) {     \
      pEngine->DefineObjProperty(g_nObjDefnID, JS_Class_Properties[i].pName, \
                                 JS_Class_Properties[i].pPropGet,            \
                                 JS_Class_Properties[i].pPropPut);           \
    }                                                                        \
  }                                                                          \
  void js_class_name::DefineMethods(CFXJS_Engine* pEngine) {                 \
    for (size_t i = 0; i < FX_ArraySize(JS_Class_Methods) - 1; ++i) {        \
      pEngine->DefineObjMethod(g_nObjDefnID, JS_Class_Methods[i].pName,      \
                               JS_Class_Methods[i].pMethodCall);             \
    }                                                                        \
  }

// Special JS classes implement methods, props, and queries, but not consts.
#define DECLARE_SPECIAL_JS_CLASS() \
  DECLARE_JS_CLASS_BASE_PART()     \
  DECLARE_JS_CLASS_CONST_PART()    \
  DECLARE_JS_CLASS_RICH_PART()     \
  DECLARE_SPECIAL_JS_CLASS_PART()

#define IMPLEMENT_SPECIAL_JS_CLASS(js_class_name, class_alternate, class_name) \
  IMPLEMENT_JS_CLASS_BASE_PART(js_class_name, class_name)                      \
  IMPLEMENT_JS_CLASS_CONST_PART(js_class_name, class_name)                     \
  IMPLEMENT_JS_CLASS_RICH_PART(js_class_name, class_alternate, class_name)     \
  IMPLEMENT_SPECIAL_JS_CLASS_PART(js_class_name, class_alternate, class_name)  \
  void js_class_name::DefineJSObjects(CFXJS_Engine* pEngine,                   \
                                      FXJSOBJTYPE eObjType) {                  \
    g_nObjDefnID = pEngine->DefineObj(js_class_name::g_pClassName, eObjType,   \
                                      JSConstructor, JSDestructor);            \
    DefineConsts(pEngine);                                                     \
    DefineProps(pEngine);                                                      \
    DefineMethods(pEngine);                                                    \
    DefineAllProperties(pEngine);                                              \
  }

#define DECLARE_SPECIAL_JS_CLASS_PART()                                        \
  static void queryprop_static(                                                \
      v8::Local<v8::String> property,                                          \
      const v8::PropertyCallbackInfo<v8::Integer>& info);                      \
  static void getprop_static(v8::Local<v8::String> property,                   \
                             const v8::PropertyCallbackInfo<v8::Value>& info); \
  static void putprop_static(v8::Local<v8::String> property,                   \
                             v8::Local<v8::Value> value,                       \
                             const v8::PropertyCallbackInfo<v8::Value>& info); \
  static void delprop_static(                                                  \
      v8::Local<v8::String> property,                                          \
      const v8::PropertyCallbackInfo<v8::Boolean>& info);                      \
  static void DefineAllProperties(CFXJS_Engine* pEngine);

#define IMPLEMENT_SPECIAL_JS_CLASS_PART(js_class_name, class_alternate,    \
                                        class_name)                        \
  void js_class_name::queryprop_static(                                    \
      v8::Local<v8::String> property,                                      \
      const v8::PropertyCallbackInfo<v8::Integer>& info) {                 \
    JSSpecialPropQuery<class_alternate>(#class_name, property, info);      \
  }                                                                        \
  void js_class_name::getprop_static(                                      \
      v8::Local<v8::String> property,                                      \
      const v8::PropertyCallbackInfo<v8::Value>& info) {                   \
    JSSpecialPropGet<class_alternate>(#class_name, property, info);        \
  }                                                                        \
  void js_class_name::putprop_static(                                      \
      v8::Local<v8::String> property, v8::Local<v8::Value> value,          \
      const v8::PropertyCallbackInfo<v8::Value>& info) {                   \
    JSSpecialPropPut<class_alternate>(#class_name, property, value, info); \
  }                                                                        \
  void js_class_name::delprop_static(                                      \
      v8::Local<v8::String> property,                                      \
      const v8::PropertyCallbackInfo<v8::Boolean>& info) {                 \
    JSSpecialPropDel<class_alternate>(#class_name, property, info);        \
  }                                                                        \
  void js_class_name::DefineAllProperties(CFXJS_Engine* pEngine) {         \
    pEngine->DefineObjAllProperties(                                       \
        g_nObjDefnID, js_class_name::queryprop_static,                     \
        js_class_name::getprop_static, js_class_name::putprop_static,      \
        js_class_name::delprop_static);                                    \
  }

template <class Alt>
void JSSpecialPropQuery(const char*,
                        v8::Local<v8::String> property,
                        const v8::PropertyCallbackInfo<v8::Integer>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  v8::String::Utf8Value utf8_value(property);
  CFX_WideString propname = CFX_WideString::FromUTF8(
      CFX_ByteStringC(*utf8_value, utf8_value.length()));
  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  bool bRet = pObj->QueryProperty(propname.c_str());
  info.GetReturnValue().Set(bRet ? 4 : 0);
}

template <class Alt>
void JSSpecialPropGet(const char* class_name,
                      v8::Local<v8::String> property,
                      const v8::PropertyCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;
  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  CFX_WideString propname = CFX_WideString::FromUTF8(
      CFX_ByteStringC(*utf8_value, utf8_value.length()));
  CFX_WideString sError;
  CJS_PropValue value(pRuntime);
  value.StartGetting();
  if (!pObj->DoProperty(pRuntime->GetCurrentContext(), propname.c_str(), value,
                        sError)) {
    pRuntime->Error(JSFormatErrorString(class_name, "GetProperty", sError));
    return;
  }
  info.GetReturnValue().Set(value.GetJSValue()->ToV8Value(pRuntime));
}

template <class Alt>
void JSSpecialPropPut(const char* class_name,
                      v8::Local<v8::String> property,
                      v8::Local<v8::Value> value,
                      const v8::PropertyCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;
  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  CFX_WideString propname = CFX_WideString::FromUTF8(
      CFX_ByteStringC(*utf8_value, utf8_value.length()));
  CFX_WideString sError;
  CJS_PropValue PropValue(pRuntime, CJS_Value(pRuntime, value));
  PropValue.StartSetting();
  if (!pObj->DoProperty(pRuntime->GetCurrentContext(), propname.c_str(),
                        PropValue, sError)) {
    pRuntime->Error(JSFormatErrorString(class_name, "PutProperty", sError));
  }
}

template <class Alt>
void JSSpecialPropDel(const char* class_name,
                      v8::Local<v8::String> property,
                      const v8::PropertyCallbackInfo<v8::Boolean>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;
  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  CFX_WideString propname = CFX_WideString::FromUTF8(
      CFX_ByteStringC(*utf8_value, utf8_value.length()));
  CFX_WideString sError;
  if (!pObj->DelProperty(pRuntime->GetCurrentContext(), propname.c_str(),
                         sError)) {
    CFX_ByteString cbName;
    cbName.Format("%s.%s", class_name, "DelProperty");
    // Probably a missing call to JSFX_Error().
  }
}

template <bool (*F)(IJS_Context*,
                    const std::vector<CJS_Value>&,
                    CJS_Value&,
                    CFX_WideString&)>
void JSGlobalFunc(const char* func_name_string,
                  const v8::FunctionCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;
  std::vector<CJS_Value> parameters;
  for (unsigned int i = 0; i < (unsigned int)info.Length(); i++) {
    parameters.push_back(CJS_Value(pRuntime, info[i]));
  }
  CJS_Value valueRes(pRuntime);
  CFX_WideString sError;
  if (!(*F)(pRuntime->GetCurrentContext(), parameters, valueRes, sError)) {
    pRuntime->Error(JSFormatErrorString(func_name_string, nullptr, sError));
    return;
  }
  info.GetReturnValue().Set(valueRes.ToV8Value(pRuntime));
}

#define JS_STATIC_GLOBAL_FUN(fun_name)                   \
  static void fun_name##_static(                         \
      const v8::FunctionCallbackInfo<v8::Value>& info) { \
    JSGlobalFunc<fun_name>(#fun_name, info);             \
  }

#define JS_STATIC_DECLARE_GLOBAL_FUN()  \
  static JSMethodSpec global_methods[]; \
  static void DefineJSObjects(CFXJS_Engine* pEngine)

#define BEGIN_JS_STATIC_GLOBAL_FUN(js_class_name) \
  JSMethodSpec js_class_name::global_methods[] = {
#define JS_STATIC_GLOBAL_FUN_ENTRY(method_name) \
  JS_STATIC_METHOD_ENTRY(method_name)

#define END_JS_STATIC_GLOBAL_FUN() END_JS_STATIC_METHOD()

#define IMPLEMENT_JS_STATIC_GLOBAL_FUN(js_class_name)               \
  void js_class_name::DefineJSObjects(CFXJS_Engine* pEngine) {      \
    for (size_t i = 0; i < FX_ArraySize(global_methods) - 1; ++i) { \
      pEngine->DefineGlobalMethod(                                  \
          js_class_name::global_methods[i].pName,                   \
          js_class_name::global_methods[i].pMethodCall);            \
    }                                                               \
  }

#endif  // FPDFSDK_JAVASCRIPT_JS_DEFINE_H_
