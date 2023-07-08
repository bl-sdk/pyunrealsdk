#include "pyunrealsdk/pch.h"
#include "unrealsdk/logging.h"
#include "unrealsdk/format.h"

using unrealsdk::logging::Level;

namespace pyunrealsdk::logging {

namespace {

/**
 * @brief Get the current Python frame's location to use when logging.
 *
 * @param frame A pointer to the current frame. If nullptr, will grab it automatically.
 * @return A pair of the location and line number.
 */
std::pair<std::string, int> get_python_location(PyFrameObject* frame = nullptr) {
    py::gil_scoped_acquire gil{};

    if (frame == nullptr) {
        frame = PyEval_GetFrame();
    }

    std::string filename{};
    int line_num = -1;

    if (frame != nullptr) {
        auto code = PyFrame_GetCode(frame);
        if (code != nullptr) {
            filename = py::str(code->co_filename);
        }

        line_num = PyFrame_GetLineNumber(frame);
    }

    return {filename, line_num};
}

/**
 * @brief A write only file object which redirects to the unrealsdk log system.
 */
class Logger {
   private:
    std::stringstream stream{};

   public:
    /// The log level to log at.
    Level level = Level::INFO;

    /**
     * @brief Constructs a new Logger
     *
     * @param level The initial log level to use.
     */
    Logger(void) = default;
    Logger(Level level) : level(level) {}

    /**
     * @brief Writes a string to the stream.
     *
     * @param text The text to write.
     * @return The number of chars which were written (which is always equal to the length of the
     *         string).
     */
    size_t write(const std::string& text) {
        auto num_newlines = std::count(text.begin(), text.end(), '\n');
        this->stream << text;
        this->flush(num_newlines);
        return text.size();
    }

    /**
     * @brief Flushes the buffer.
     *
     * @param lines_to_flush The number of lines to flush. Defaults to the rest of the buffer.
     */
    void flush(size_t lines_to_flush = std::numeric_limits<size_t>::max()) {
        auto clamped_level = std::clamp(this->level, Level::MIN, Level::MAX);
        auto [location, line_num] = get_python_location();

        std::string str;
        for (size_t i = 0; i < lines_to_flush && std::getline(this->stream, str); i++) {
            unrealsdk::logging::log(clamped_level, str, location.c_str(), line_num);
        }

        // We need to clear the stream occasionally
        // Since most of the time, we're writing one line at a time, rather than doing something
        // complex to deal with possible left over data, only clear when we're at the end already.
        if (this->stream.peek() == decltype(this->stream)::traits_type::eof()) {
            this->stream.clear();
            this->stream.str({});
            this->stream.seekg(0);
        }
    }
};

/**
 * @brief Registers a function which prints at a specific log level.
 *
 * @param logging The logging module to register within.
 * @param level The log level this printer is for.
 * @param func_name The name of the printing function.
 * @param docstring_name The name of the log level to include in the docstring.
 */
void register_per_log_level_printer(py::module_& logging,
                                    Level level,
                                    const char* func_name,
                                    const char* docstring_name) {
    auto docstring = unrealsdk::fmt::format(
        "Wrapper around print(), which temporarily changes the log level of stdout to\n"
        "{}.\n"
        "\n"
        "Args:\n"
        "    *args: Forwarded to print().\n"
        "    **kwargs: Forwarded to print().",
        docstring_name);
    logging.def(
        func_name,
        [level](const py::args& args, const py::kwargs& kwargs) {
            auto py_stdout = py::module_::import("sys").attr("stdout");

            auto old_level = py::cast<Level>(py_stdout.attr("level"));
            py_stdout.attr("level") = level;

            py::print(*args, **kwargs);

            py_stdout.attr("level") = old_level;
        },
        docstring.c_str());
}

}  // namespace

void register_module(py::module_& mod) {
    auto logging = mod.def_submodule("logging");

    py::enum_<Level>(logging, "Level", "Enum of valid logging levels.")
        .value("ERROR", Level::ERROR, "Used to display error messages.")
        .value("WARNING", Level::WARNING, "Used to display warnings.")
        .value("INFO", Level::INFO,
               "Default logging level, used for anything that should be shown in console.")
        .value("DEV_WARNING", Level::DEV_WARNING,
               "Used for warnings which don't concern users, so shouldn't be shown in console.")
        .value("MISC", Level::MISC, "Used for miscellaneous debug messages.")
        .export_values();

    py::class_<Logger>(logging, "Logger",
                       "A write only file object which redirects to the unrealsdk log system.")
        .def(py::init<Level>(),
             "Creates a new logger.\n"
             "\n"
             "Args:\n"
             "    level: The default log level to initialize to.",
             "level"_a = Level::INFO)
        .def_readwrite("level", &Logger::level,
                       "The current log level which messages are written at.")
        .def("write", &Logger::write,
             "Writes a string to the stream.\n"
             "\n"
             "Args:\n"
             "    text: The text to write.\n"
             "Returns:\n"
             "    The number of chars which were written (which is always equal to the length\n"
             "    of the string).",
             "text"_a)
        .def(
            "flush", [](Logger& self) { self.flush(); }, "Flushes the stream.");

    logging.def("set_console_level", &unrealsdk::logging::set_console_level,
                "Sets the log level of the unreal console.\n"
                "\n"
                "Does not affect the log file or external console, if enabled.\n"
                "\n"
                "Args:\n"
                "    level: The new log level.\n"
                "Returns:\n"
                "    True if console level changed, false if an invalid value was passed in.",
                "level"_a);

    register_per_log_level_printer(logging, Level::MISC, "misc", "misc");
    register_per_log_level_printer(logging, Level::DEV_WARNING, "dev_warning", "dev warning");
    register_per_log_level_printer(logging, Level::INFO, "info", "info");
    register_per_log_level_printer(logging, Level::WARNING, "warning", "warning");
    register_per_log_level_printer(logging, Level::ERROR, "error", "error");
}

void py_init(void) {
    py::gil_scoped_acquire gil{};

    auto sys = py::module_::import("sys");
    sys.attr("stdout") = Logger{Level::INFO};
    sys.attr("stderr") = Logger{Level::ERROR};
}

void log_python_exception(const std::exception& exc) {
    PyFrameObject* frame = nullptr;

    try {
        // See if this is a python error
        auto error_already_set = dynamic_cast<const py::error_already_set&>(exc);

        // If so, read the location from the traceback's frame instead
        py::gil_scoped_acquire gil{};
        frame = reinterpret_cast<PyTracebackObject*>(error_already_set.trace().ptr())->tb_frame;
    } catch (const std::bad_cast&) {}

    auto [location, line_num] = get_python_location(frame);

    std::istringstream stream{exc.what()};
    std::string msg_line;
    while (std::getline(stream, msg_line)) {
        unrealsdk::logging::log(unrealsdk::logging::Level::ERROR, msg_line, location.c_str(),
                                line_num);
    }
}

}  // namespace pyunrealsdk::logging
