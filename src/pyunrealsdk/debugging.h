#ifndef PYUNREALSDK_DEBUGGING_H
#define PYUNREALSDK_DEBUGGING_H

namespace pyunrealsdk {

/**
 * @brief Makes the python being called in this thread debuggable.
 * @note Should be called before running any python callback which may be triggered by a hook on a
 *       different thread.
 */
void debug_this_thread(void);

}  // namespace pyunrealsdk

#endif /* PYUNREALSDK_DEBUGGING_H */
