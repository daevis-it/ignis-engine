#pragma once
#include <iostream>
#include <format>
#include <string_view>

namespace Ignis
{
    class Logger {
    public:
        // Usiamo std::format per permettere di passare argomenti dinamici in modo pulito
        template<typename... Args>
        static void Info(std::string_view rt_fmt_str, Args&&... args) {
            std::cout << "[IGNIS INFO]: " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << '\n';
        }

        template<typename... Args>
        static void Error(std::string_view rt_fmt_str, Args&&... args) {
            std::cerr << "[IGNIS ERRORE]: " << std::vformat(rt_fmt_str, std::make_format_args(args...)) << '\n';
        }
    };
}