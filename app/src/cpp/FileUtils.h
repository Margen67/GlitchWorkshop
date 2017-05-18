#pragma once
namespace cs {
namespace FileUtils {
    bool copyFile(const char* src, const char* dst);
    bool copyFile(cloture::util::string::String_t src, cloture::util::string::String_t dst);
}}