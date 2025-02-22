#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/commands.h"
#include "pyunrealsdk/debugging.h"
#include "pyunrealsdk/logging.h"
#include "unrealsdk/commands.h"
#include "unrealsdk/config.h"
#include "unrealsdk/utils.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk::commands {

namespace {

void pyexec_cmd_handler(const wchar_t* line, size_t size, size_t cmd_len) {
    static const std::filesystem::path root =
        unrealsdk::config::get_str("pyunrealsdk.pyexec_root").value_or("");

    auto file_start = std::find_if_not(line + cmd_len, line + size, &std::iswspace);
    const size_t file_len = (line + size) - file_start;

    const auto path = (root / std::wstring_view{file_start, file_len}).wstring();

    try {
        const py::gil_scoped_acquire gil{};
        const py::str py_path{
            PyUnicode_FromWideChar(path.c_str(), static_cast<py::ssize_t>(path.size()))};

        // Use a custom globals so it's not contaminated from the init script/`py` commands
        // This also ensures `__file__` gets redefined properly
        py::eval_file(py_path, py::dict{});
    } catch (const std::exception& ex) {
        logging::log_python_exception(ex);
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

    const std::wstring py_str{py_start, py_len};

    // If we reach eof, clear it, and don't add to the stream
    if (heredoc_eof == py_str) {
        heredoc_eof.clear();
    } else {
        stream << py_str;
    }

    // If we're waiting on more lines, add a newline, register a next line command, and exit
    if (!heredoc_eof.empty()) {
        stream << L'\n';
        unrealsdk::commands::add_command(unrealsdk::commands::NEXT_LINE, py_cmd_handler);
        return;
    }

    // Get the full string, and clear the stream
    auto str = stream.str();
    stream.clear();
    stream.str({});

    try {
        const py::gil_scoped_acquire gil{};

        const py::str code_block{
            PyUnicode_FromWideChar(str.c_str(), static_cast<py::ssize_t>(str.size()))};

        py::exec(code_block);
    } catch (const std::exception& ex) {
        logging::log_python_exception(ex);
    }
}

}  // namespace

void register_module(py::module_& mod) {
    auto commands = mod.def_submodule("commands");

    commands.def(
        "add_command",
        [](const std::wstring& cmd, const py::object& callback) {
            unrealsdk::commands::add_command(
                cmd, [callback](const wchar_t* line, size_t size, size_t cmd_len) {
                    try {
                        const py::gil_scoped_acquire gil{};
                        debug_this_thread();

                        const py::str py_line{
                            PyUnicode_FromWideChar(line, static_cast<py::ssize_t>(size))};

                        callback(py_line, cmd_len);
                    } catch (const std::exception& ex) {
                        logging::log_python_exception(ex);
                    }
                });
        },
        "Adds a custom console command.\n"
        "\n"
        "Console commands are matched by comparing the first block of non-whitespace\n"
        "characters in a line submitted to console against all registered commands.\n"
        "\n"
        "As a special case, if you register the special NEXT_LINE command, it will always\n"
        "match the very next line, in place of anything else which might have been\n"
        "matched otherwise. It will then immediately be removed (though before the\n"
        "callback is run, so you can re-register it if needed), to allow normal command\n"
        "processing to continue afterwards.\n"
        "\n"
        "Console command callbacks take two args:\n"
        "    line: The full line which triggered the callback - including any\n"
        "          whitespace.\n"
        "    cmd_len: The length of the matched command, including leading whitespace -\n"
        "             i.e. line[cmd_len] points to the first whitespace char after the\n"
        "             command (or off the end of the string if there was none). 0 in the\n"
        "             case of a `NEXT_LINE` match.\n"
        "The return value is ignored.\n"
        "\n"
        "Args:\n"
        "    cmd: The command to match.\n"
        "    callback: The callback for when the command is run.\n"
        "Returns:\n"
        "    True if successfully added, false if an identical command already exists.",
        "cmd"_a, "callback"_a);

    commands.def("has_command", &unrealsdk::commands::has_command,
                 "Check if a custom console command has been registered.\n"
                 "\n"
                 "Args:\n"
                 "    cmd: The command to match.\n"
                 "Returns:\n"
                 "    True if the command has been registered.",
                 "cmd"_a);

    commands.def("remove_command", &unrealsdk::commands::remove_command,
                 "Removes a custom console command.\n"
                 "\n"
                 "Args:\n"
                 "    cmd: The command to remove.\n"
                 "Returns:\n"
                 "    True if successfully removed, false if no such command exists.",
                 "cmd"_a);

    commands.attr("NEXT_LINE") = unrealsdk::commands::NEXT_LINE;
}

void register_commands(void) {
    // Make sure unrealsdk is already in globals, for convenience
    // The init script and `pyexec` commands both use a local dict, so this won't affect them
    auto globals = py::globals();
    if (!globals.contains("unrealsdk")) {
        globals["unrealsdk"] = py::module_::import("unrealsdk");
    }

    unrealsdk::commands::add_command(L"pyexec", &pyexec_cmd_handler);
    unrealsdk::commands::add_command(L"py", &py_cmd_handler);
}

}  // namespace pyunrealsdk::commands

#endif
