// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSSTYLESHEET_H_
#define XFA_FDE_CSS_FDE_CSSSTYLESHEET_H_

#include <memory>
#include <unordered_map>

#include "core/fxcrt/fx_ext.h"
#include "xfa/fde/css/fde_cssdeclaration.h"

class CFDE_CSSSyntaxParser;

class CFDE_CSSSelector {
 public:
  CFDE_CSSSelector(FDE_CSSSelectorType eType,
                   const FX_WCHAR* psz,
                   int32_t iLen,
                   bool bIgnoreCase);

  virtual FDE_CSSSelectorType GetType() const;
  virtual uint32_t GetNameHash() const;
  virtual CFDE_CSSSelector* GetNextSelector() const;

  static CFDE_CSSSelector* FromString(const FX_WCHAR* psz, int32_t iLen);

  void SetNext(CFDE_CSSSelector* pNext) { m_pNext = pNext; }

 protected:
  void SetType(FDE_CSSSelectorType eType) { m_eType = eType; }

  FDE_CSSSelectorType m_eType;
  uint32_t m_dwHash;
  CFDE_CSSSelector* m_pNext;
};

class CFDE_CSSStyleRule : public IFDE_CSSStyleRule {
 public:
  CFDE_CSSStyleRule();

  // IFDE_CSSStyleRule
  int32_t CountSelectorLists() const override;
  CFDE_CSSSelector* GetSelectorList(int32_t index) const override;
  CFDE_CSSDeclaration* GetDeclaration() override;

  CFDE_CSSDeclaration& GetDeclImp() { return m_Declaration; }
  void SetSelector(const CFX_ArrayTemplate<CFDE_CSSSelector*>& list);

 protected:
  CFDE_CSSDeclaration m_Declaration;
  CFDE_CSSSelector** m_ppSelector;
  int32_t m_iSelectors;
};

class CFDE_CSSMediaRule : public IFDE_CSSMediaRule {
 public:
  explicit CFDE_CSSMediaRule(uint32_t dwMediaList);
  ~CFDE_CSSMediaRule() override;

  // IFDE_CSSMediaRule
  uint32_t GetMediaList() const override;
  int32_t CountRules() const override;
  IFDE_CSSRule* GetRule(int32_t index) override;

  CFX_MassArrayTemplate<IFDE_CSSRule*>& GetArray() { return m_RuleArray; }

 protected:
  uint32_t m_dwMediaList;
  CFX_MassArrayTemplate<IFDE_CSSRule*> m_RuleArray;
};

class CFDE_CSSFontFaceRule : public IFDE_CSSFontFaceRule {
 public:
  // IFDE_CSSFontFaceRule
  CFDE_CSSDeclaration* GetDeclaration() override;

  CFDE_CSSDeclaration& GetDeclImp() { return m_Declaration; }

 protected:
  CFDE_CSSDeclaration m_Declaration;
};

class CFDE_CSSStyleSheet : public IFDE_CSSStyleSheet {
 public:
  explicit CFDE_CSSStyleSheet(uint32_t dwMediaList);
  ~CFDE_CSSStyleSheet() override;

  // IFX_Retainable:
  uint32_t Retain() override;
  uint32_t Release() override;

  // IFDE_CSSStyleSheet:
  bool GetUrl(CFX_WideString& szUrl) override;
  uint32_t GetMediaList() const override;
  uint16_t GetCodePage() const override;
  int32_t CountRules() const override;
  IFDE_CSSRule* GetRule(int32_t index) override;

  bool LoadFromBuffer(const CFX_WideString& szUrl,
                      const FX_WCHAR* pBuffer,
                      int32_t iBufSize,
                      uint16_t wCodePage);

 protected:
  void Reset();
  bool LoadFromSyntax(CFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSyntaxStatus LoadStyleRule(
      CFDE_CSSSyntaxParser* pSyntax,
      CFX_MassArrayTemplate<IFDE_CSSRule*>& ruleArray);
  FDE_CSSSyntaxStatus LoadImportRule(CFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSyntaxStatus LoadPageRule(CFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSyntaxStatus LoadMediaRule(CFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSyntaxStatus LoadFontFaceRule(
      CFDE_CSSSyntaxParser* pSyntax,
      CFX_MassArrayTemplate<IFDE_CSSRule*>& ruleArray);
  FDE_CSSSyntaxStatus SkipRuleSet(CFDE_CSSSyntaxParser* pSyntax);

  uint16_t m_wCodePage;
  uint16_t m_wRefCount;
  uint32_t m_dwMediaList;
  CFX_MassArrayTemplate<IFDE_CSSRule*> m_RuleArray;
  CFX_WideString m_szUrl;
  CFX_ArrayTemplate<CFDE_CSSSelector*> m_Selectors;
  std::unordered_map<uint32_t, FX_WCHAR*> m_StringCache;
};

#endif  // XFA_FDE_CSS_FDE_CSSSTYLESHEET_H_
