#pragma once
#include <expected>
#include <fstream>
#include <filesystem>

enum class ReadFileError {
    not_a_file,
    failed_to_open,
    failed_to_read,
};;
inline auto read_file(std::filesystem::path const& path) -> std::expected<std::string, ReadFileError> {
    using err = ReadFileError;
    if (not std::filesystem::is_regular_file(path)) {
        return std::unexpected(err::not_a_file);
    }
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file) return std::unexpected(err::failed_to_open);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    auto buf = std::string(size, '\0');
    if (not file.read(&buf.data()[0], size)) {
        return std::unexpected(err::failed_to_read);
    }
    return buf;
}
