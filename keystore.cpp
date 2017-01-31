#if 0
(
set -euo pipefail
declare -r out="$(mktemp)"
trap 'rm -f -- "$out"' EXIT ERR
g++ -Wall -Wextra -DTEST_keystore_cpp -std=c++14 -O0 -o "$out" "$0"
exec "$out"
)
exit 0
#endif
#include <string.h>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "keystore.hpp"

namespace mark {

keystore::keystore(const char *buf, size_t size, char assignment, char delimiter) :
	keystore(assignment, delimiter)
{
	const char *end = buf + size;
	while (buf < end) {
		const auto pend = (const char *) memchr(buf, delimiter, size);
		if (pend == NULL) {
			goto invalid;
		}
		const auto pdelim = (const char *) memchr(buf, assignment, pend - buf);
		if (pdelim == NULL) {
			goto invalid;
		}
		emplace(std::string(buf, pdelim), std::string(pdelim + 1, pend));
		const auto skip = 1 + pend - buf;
		size -= skip;
		buf += skip;
	}
	return;
invalid:
	throw std::runtime_error("Multistr buffer is not terminated correctly");
}

std::string keystore::join() const
{
	std::vector<std::string> lines;
	size_t length = 0;
	for (const auto& p : *this) {
		const std::string line = p.first + assignment + p.second + delimiter;
		length += line.size();
		lines.emplace_back(line);
	}
	std::string out;
	out.resize(length);
	char *buf = &out[0];
	for (const auto& line : lines) {
		memcpy(buf, line.data(), line.size());
		buf += line.size();
	}
	return out;
}

}

#if defined TEST_keystore_cpp
#include <iostream>
int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	std::string data = "key=value;other=another value;this=that;";
	auto ms = mark::keystore(data, '=', ';');
	const auto str = ms.join();
	if (str != data) {
		std::cout << data << std::endl;
		std::cout << str << std::endl;
		std::cerr << "Failed" << std::endl;
		return 1;
	}
	ms.assignment = ':';
	ms.delimiter = '\n';
	std::cerr << ms.join() << std::endl;
	return 0;
}
#endif
