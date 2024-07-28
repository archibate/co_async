#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

#include "hmac.h" // IWYU: export
#include "keccak.h" // IWYU: export
#include "md5.h" // IWYU: export
#include "sha1.h" // IWYU: export
#include "sha256.h" // IWYU: export
#include "sha3.h" // IWYU: export
#include "base64.hpp" // IWYU: export

#pragma GCC diagnostic pop
