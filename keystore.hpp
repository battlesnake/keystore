#pragma once
/* C++ codec for key/value list */
#include "keystore_defs.h"
#include <map>
#include <string>
#include <stddef.h>

namespace mark {

struct keystore : std::map<std::string, std::string>
{
	char assignment;
	char delimiter;
	keystore(char assignment = kv_delim, char delimiter = line_delim) :
		std::map<std::string, std::string>(),
		assignment(assignment),
		delimiter(delimiter)
		{ }
	keystore(const char *buf, size_t size, char assignment = kv_delim, char delimiter = line_delim);
	keystore(const std::string& buf, char assignment = kv_delim, char delimiter = line_delim) :
		keystore(buf.data(), buf.size(), assignment, delimiter)
		{ }
	std::string join() const;
};

}
