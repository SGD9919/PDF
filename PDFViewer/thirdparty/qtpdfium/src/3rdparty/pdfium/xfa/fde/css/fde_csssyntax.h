// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSSYNTAX_H_
#define XFA_FDE_CSS_FDE_CSSSYNTAX_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "xfa/fde/css/fde_css.h"
#include "xfa/fgas/crt/fgas_stream.h"

class CFDE_CSSTextBuf {
 public:
  CFDE_CSSTextBuf();
  ~CFDE_CSSTextBuf();

  bool AttachBuffer(const FX_WCHAR* pBuffer, int32_t iBufLen);
  bool EstimateSize(int32_t iAllocSize);
  int32_t LoadFromStream(const CFX_RetainPtr<IFGAS_Stream>& pTxtStream,
                         int32_t iStreamOffset,
                         int32_t iMaxChars,
                         bool& bEOS);
  bool AppendChar(FX_WCHAR wch) {
    if (m_iDatLen >= m_iBufLen && !ExpandBuf(m_iBufLen * 2))
      return false;
    m_pBuffer[m_iDatLen++] = wch;
    return true;
  }
  void Clear() { m_iDatPos = m_iDatLen = 0; }
  void Reset();
  int32_t TrimEnd() {
    while (m_iDatLen > 0 && m_pBuffer[m_iDatLen - 1] <= ' ') {
      --m_iDatLen;
    }
    AppendChar(0);
    return --m_iDatLen;
  }
  void Subtract(int32_t iStart, int32_t iLength);
  bool IsEOF() const { return m_iDatPos >= m_iDatLen; }
  FX_WCHAR GetAt(int32_t index) const { return m_pBuffer[index]; }
  FX_WCHAR GetChar() const { return m_pBuffer[m_iDatPos]; }
  FX_WCHAR GetNextChar() const {
    return (m_iDatPos + 1 >= m_iDatLen) ? 0 : m_pBuffer[m_iDatPos + 1];
  }
  void MoveNext() { m_iDatPos++; }
  int32_t GetLength() const { return m_iDatLen; }
  const FX_WCHAR* GetBuffer() const { return m_pBuffer; }

 protected:
  bool ExpandBuf(int32_t iDesiredSize);
  bool m_bExtBuf;
  FX_WCHAR* m_pBuffer;
  int32_t m_iBufLen;
  int32_t m_iDatLen;
  int32_t m_iDatPos;
};

#define FDE_CSSSYNTAXCHECK_AllowCharset 1
#define FDE_CSSSYNTAXCHECK_AllowImport 2

enum FDE_CSSSYNTAXMODE {
  FDE_CSSSYNTAXMODE_RuleSet,
  FDE_CSSSYNTAXMODE_Comment,
  FDE_CSSSYNTAXMODE_AtRule,
  FDE_CSSSYNTAXMODE_UnknownRule,
  FDE_CSSSYNTAXMODE_Charset,
  FDE_CSSSYNTAXMODE_Import,
  FDE_CSSSYNTAXMODE_MediaRule,
  FDE_CSSSYNTAXMODE_URI,
  FDE_CSSSYNTAXMODE_MediaType,
  FDE_CSSSYNTAXMODE_Selector,
  FDE_CSSSYNTAXMODE_PropertyName,
  FDE_CSSSYNTAXMODE_PropertyValue,
};

class CFDE_CSSSyntaxParser {
 public:
  CFDE_CSSSyntaxParser();
  ~CFDE_CSSSyntaxParser();

  bool Init(const CFX_RetainPtr<IFGAS_Stream>& pStream,
            int32_t iCSSPlaneSize,
            int32_t iTextDataSize = 32,
            bool bOnlyDeclaration = false);
  bool Init(const FX_WCHAR* pBuffer,
            int32_t iBufferSize,
            int32_t iTextDatSize = 32,
            bool bOnlyDeclaration = false);
  FDE_CSSSyntaxStatus DoSyntaxParse();
  const FX_WCHAR* GetCurrentString(int32_t& iLength) const;

 protected:
  void Reset(bool bOnlyDeclaration);
  void SwitchMode(FDE_CSSSYNTAXMODE eMode);
  int32_t SwitchToComment();

  bool RestoreMode();
  bool AppendChar(FX_WCHAR wch);
  int32_t SaveTextData();
  bool IsCharsetEnabled() const {
    return (m_dwCheck & FDE_CSSSYNTAXCHECK_AllowCharset) != 0;
  }
  void DisableCharset() { m_dwCheck = FDE_CSSSYNTAXCHECK_AllowImport; }
  bool IsImportEnabled() const;
  void DisableImport() { m_dwCheck = 0; }

  CFX_RetainPtr<IFGAS_Stream> m_pStream;
  int32_t m_iStreamPos;
  int32_t m_iPlaneSize;
  CFDE_CSSTextBuf m_TextData;
  CFDE_CSSTextBuf m_TextPlane;
  int32_t m_iTextDatLen;
  uint32_t m_dwCheck;
  FDE_CSSSYNTAXMODE m_eMode;
  FDE_CSSSyntaxStatus m_eStatus;
  CFX_StackTemplate<FDE_CSSSYNTAXMODE> m_ModeStack;
};

#endif  // XFA_FDE_CSS_FDE_CSSSYNTAX_H_
