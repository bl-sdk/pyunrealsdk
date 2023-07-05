#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/commands.h"
#include "unrealsdk/commands.h"
#include "unrealsdk/utils.h"

namespace pyunrealsdk::commands {

namespace {

void pyexec_cmd_handler(const wchar_t* line, size_t size, size_t cmd_len) {
    auto file_start = std::find_if_not(line + cmd_len, line + size, &std::iswspace);
    auto file_len = (line + size) - file_start;

    try {
        py::gil_scoped_acquire gil{};
        py::str file{PyUnicode_FromWideChar(file_start, static_cast<Py_ssize_t>(file_len))};
        py::eval_file(file);
    } catch (const std::exception& ex) {
        LOG_MULTILINE(ERROR, ex.what());
    }
}

void py_cmd_handler(const wchar_t* line, size_t size, size_t cmd_len) {
    static std::wostringstream stream{};
    static std::wstring heredoc_eof;

    // Default to matching the entire line, incase we're mid heredoc
    const wchar_t* py_start = line;
    size_t py_len = size;

    // If this is a new command (and not in a heredoc)
    if (heredoc_eof.empty()) {
        // Start on the first non-whitespace character
        py_start = std::find_if_not(line + cmd_len, line + size, &std::iswspace);
        py_len = (line + size) - py_start;

        // Do nothing if we got an empty line
        if (py_start == line + size) {
            return;
        }

        // If we start with `<<`, we're starting a heredoc
        if (py_start[0] == L'<' && py_start[1] == L'<') {
            // Find the next block of non-whitespace characters
            auto heredoc_start = std::find_if_not(py_start + 2, line + size, &std::iswspace);
            heredoc_eof.assign(heredoc_start, line + size);

            // Make the python match empty - we don't want to process anything
            // This also has the side effect of making us instantly match an empty eof marker
            py_start = line + size;
            py_len = 0;
        }
    }

    std::wstring py_str{py_start, py_len};

    // If we reach eof, clear it, and don't add to the stream
    if (heredoc_eof == py_str) {
        heredoc_eof.clear();
    } else {
        stream.write(py_start, py_len);
    }

    // If we're waiting on more lines, add a newline, register a next line command, and exit
    if (!heredoc_eof.empty()) {
        stream << L'\n';
        unrealsdk::commands::add_command(unrealsdk::commands::NEXT_LINE, &py_cmd_handler);
        return;
    }

    // Get the full string, and clear the stream
    auto str = stream.str();
    stream.clear();
    stream.str({});

    try {
        py::gil_scoped_acquire gil{};

        py::str code_block{
            PyUnicode_FromWideChar(str.c_str(), static_cast<Py_ssize_t>(str.size()))};
        py::exec(code_block);
    } catch (const std::exception& ex) {
        LOG_MULTILINE(ERROR, ex.what());
    }
}

}  // namespace

void register_commands(void) {
    unrealsdk::commands::add_command(L"pyexec", &pyexec_cmd_handler);
    unrealsdk::commands::add_command(L"py", &py_cmd_handler);
}

}  // namespace pyunrealsdk::commands
