/**
 * Helper code for logging.
 *
 * Author: Dr. Sat Garcia (sat@sandiego.edu)
 */
#include "logging.h"
#include "aixlog.hpp"

void initialize_logging(bool verbose) {
	// Log error and higher severity messages to cerr

	if (!verbose) {
		// Log only error level and above to stderr
		auto cerr_sink = std::make_shared<AixLog::SinkCerr>(AixLog::Severity::error, "%Y-%m-%d %H:%M:%S.#ms [#severity] (#tag_func)");
		AixLog::Log::init({cerr_sink});
	}
	else {
		// verbose mode: log everything to stderr
		auto cerr_sink = std::make_shared<AixLog::SinkCerr>(AixLog::Severity::trace, "%Y-%m-%d %H:%M:%S.#ms [#severity] (#tag_func)");
		AixLog::Log::init({cerr_sink});
	}
}
