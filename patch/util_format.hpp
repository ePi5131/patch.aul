#pragma once
#include <version>
#ifndef __cpp_lib_format
#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include <fmt/core.h>
#include <fmt/xchar.h>
using fmt::format;
using fmt::vformat;
using fmt::vformat_to;
using fmt::make_format_args;
using fmt::make_wformat_args;
using fmt::format_error;
#else
#include <format>
using std::format;
using std::vformat;
using std::vformat_to;
using std::make_format_args;
using std::make_wformat_args;
using std::format_error;
#endif
