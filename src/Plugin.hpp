#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <REL/Relocation.h>
#include <boost/regex.hpp>
#include <string_view>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <ClibUtil/utils.hpp>
#include <ankerl/unordered_dense.h>

using namespace std::literals;

namespace logger
{
	using spdlog::info;
	using spdlog::warn;
	using spdlog::error;
	using SKSE::log::log_directory;
}
namespace string = clib_util::string;

using RNG = clib_util::RNG;

namespace util
{
	inline void report_and_fail(std::string_view msg) { REX::FAIL("{}", std::string(msg)); }
}

#define DLLEXPORT __declspec(dllexport)

namespace Plugin
{
	inline constexpr REL::Version VERSION
	{
		// clang-format off
		1u,
		7u,
		0u,
		// clang-format on
	};

	inline constexpr auto NAME = "FormListManipulator"sv;
	inline constexpr auto AUTHOR = "MaskedRPGFan"sv;
}
