#pragma once

// Shared symbol facts and the LowIR metadata vocabulary carried by the LowIR
// and machine-IR model scaffolds.  Every named metadata value the LowIR text
// contract admits appears here once, so a model that names one of them says
// the same thing the compiler does.
//
// This is intentionally smaller than the semantic-layer symbol representation:
// by the time a program crosses the LowIR boundary, backend code should only
// need concrete internal/object spellings and linkage choices.

#include <string>

namespace ir_model {

// Pointer catch binding is a source semantic fact. Reference permits only an
// exact type match; ConstReference can bind a converted pointer temporary.
enum class CatchBinding : unsigned char { Value, Reference, ConstReference };

enum SymbolLinkage
{
  SL_INTERNAL,
  SL_EXTERNAL,
  SL_WEAK
};

enum SymbolRole
{
  SR_NONE,
  SR_ENTRY,
  SR_INIT,
  SR_FINI,
  SR_EH_ALLOCATE_EXCEPTION,
  SR_EH_BEGIN_CATCH,
  SR_EH_END_CATCH,
  SR_EH_RETHROW,
  SR_EH_THROW,
  SR_EH_PERSONALITY,
  SR_EH_RESUME,
  SR_ALLOCATE_MEMORY,
  SR_FREE_MEMORY,
  SR_TERMINATE,
  SR_PURE_VIRTUAL,
  SR_DYNAMIC_CAST,
  SR_BAD_CAST,
  SR_BAD_TYPEID,
  SR_EH_FREE_EXCEPTION,
  SR_MALLOC,
  SR_RTTI_CLASS,
  SR_RTTI_SI,
  SR_RTTI_VMI,
  SR_RTTI_DATA
};

enum LanguageLinkageMode
{
  LLM_DEFAULT,
  LLM_C
};

enum SymbolBindingMode
{
  SBM_DEFAULT,
  SBM_INTERNAL,
  SBM_STRONG,
  SBM_WEAK
};

enum ParamPassingMode
{
  PPM_DIRECT,
  PPM_INDIRECT_RESULT,
  PPM_BY_ADDRESS
};

enum ParamAliasMode
{
  PALM_DEFAULT,
  PALM_NOALIAS
};

enum CallArityMode
{
  CAM_FIXED,
  CAM_VARIADIC
};

enum CallEffectsMode
{
  CFXM_DEFAULT,
  CFXM_READNONE,
  CFXM_READONLY
};

enum CallUnwindMode
{
  CUM_DEFAULT,
  CUM_NO
};

enum CallReturnMode
{
  CRM_DEFAULT,
  CRM_NORETURN
};

enum CallQueryMode
{
  CQM_DEFAULT,
  CQM_STABLE_PREFIX
};

enum GlobalStorageMode
{
  GSM_DEFAULT,
  GSM_READONLY,
  GSM_THREAD_LOCAL
};

enum IndexProjectionKind
{
  IPK_NONE,
  IPK_ARRAY_ELEMENT,
  IPK_FIELD
};

struct FunctionBoundaryMetadata
{
  CallArityMode arity = CAM_FIXED;
  CallEffectsMode effects = CFXM_DEFAULT;
  CallUnwindMode unwind = CUM_DEFAULT;
  CallReturnMode returns = CRM_DEFAULT;
  CallQueryMode query = CQM_DEFAULT;
};

struct ExportedSymbol
{
  std::string internal_symbol;
  std::string object_symbol;
  std::string thread_local_wrapper_object_symbol;
  bool keep_internal_alias = false;
  bool prefer_local_object_binding = false;
  SymbolLinkage linkage = SL_EXTERNAL;
};

inline bool has_object_symbol(const ExportedSymbol & symbol)
{
  return !symbol.object_symbol.empty();
}

inline bool has_exported_object_symbol(const ExportedSymbol & symbol)
{
  return !symbol.object_symbol.empty() && symbol.linkage != SL_INTERNAL;
}

inline std::string exported_object_symbol(const ExportedSymbol & symbol)
{
  return symbol.object_symbol;
}

inline bool has_weak_linkage(const ExportedSymbol & symbol)
{
  return symbol.linkage == SL_WEAK;
}

}  // namespace ir_model
