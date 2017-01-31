#pragma once

#define _kv_delim '='
#define _line_delim '\0'

/* Definitions for keystore key/value data format */
#if defined __cplusplus

namespace mark {

constexpr auto kv_delim = _kv_delim;
constexpr auto line_delim = _line_delim;

}

#undef _kv_delim
#undef _line_delim

#else

#define kv_delim _kv_delim
#define line_delim _line_delim

#endif
