#include "pyunrealsdk/pch.h"
#include "unrealsdk/logging.h"

using unrealsdk::logging::Level;

namespace pyunrealsdk::logging {

namespace {

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

        std::string filename{};
        int line = -1;

        auto frame = PyEval_GetFrame();
        if (frame != nullptr) {
            auto code = PyFrame_GetCode(frame);
            if (code != nullptr) {
                filename = py::str(code->co_filename);
            }

            line = PyFrame_GetLineNumber(frame);
        }

        std::string str;
        for (size_t i = 0; i < lines_to_flush && std::getline(this->stream, str); i++) {
            unrealsdk::logging::log(std::chrono::system_clock::now(), clamped_level, str, nullptr,
                                    filename.c_str(), line);
        }

        // We need to clear the stream occasionally
        // Since most of the time, we're writing one line at a time, rather than doing something
        // complex to deal with possible left over data, only clear when we're at the end already.
        if (this->stream.eof()) {
            this->stream.clear();
            this->stream.str({});
            this->stream.seekg(0);
        }
    }
};

Logger stdout_logger{Level::INFO};
Logger stderr_logger{Level::ERROR};

}  // namespace

void register_module(py::module_& mod) {
    py::enum_<Level>(mod, "LogLevel", "Enum of valid logging levels.")
        .value("ERROR", Level::ERROR, "Used to display error messages.")
        .value("WARNING", Level::WARNING, "Used to display warnings.")
        .value("INFO", Level::INFO,
               "Default logging level, used for anything that should be shown in console.")
        .value("DEV_WARNING", Level::DEV_WARNING,
               "Used for warnings which don't concern users, so shouldn't be shown in console.")
        .value("MISC", Level::MISC, "Used for miscellaneous debug messages.")
        .export_values();

    py::class_<Logger>(mod, "Logger",
                       "A write only file object which redirects to the unrealsdk log system.")
        .def(py::init<Level>(),
             "Creates a new logger.\n"
             "\n"
             "Args:\n"
             "    level: The default log level to initialize to.\n",
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
             "    of the string).\n",
             "text"_a)
        .def(
            "flush", [](Logger& self) { self.flush(); }, "Flushes the stream.");

    mod.def(
        "log",
        [](const py::args& args, Level level, const py::kwargs& kwargs) {
            auto old_level = stdout_logger.level;
            stdout_logger.level = level;

            py::print(*args, **kwargs);

            stdout_logger.level = old_level;
        },
        "Wrapper around print(), which automatically changes the log level of stdout.\n"
        "\n"
        "Args:\n"
        "    *args: Forwarded to print().\n"
        "    level: The log level to print at.\n"
        "    **kwargs: Forwarded to print().\n",
        "level"_a = Level::INFO);

    mod.def("set_console_log_level", &unrealsdk::logging::set_console_level,
            "Sets the log level of the unreal console.\n"
            "\n"
            "Does not affect the log file or external console, if enabled.\n"
            "\n"
            "Args:\n"
            "    level: The new log level.\n"
            "Returns:\n"
            "    True if console level changed, false if an invalid value was passed in.\n",
            "level"_a);
}

void py_init(void) {
    auto sys = py::module_::import("sys");
    sys.attr("stdout") = py::cast(stdout_logger, py::return_value_policy::reference);
    sys.attr("stderr") = py::cast(stderr_logger, py::return_value_policy::reference);
}

}  // namespace pyunrealsdk::logging
