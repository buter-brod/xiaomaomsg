#include "utils.h"
#include <fstream>
#include <streambuf>
#include <random>


std::string Utils::loadFile(const std::string& filename) {
	std::ifstream t(filename);

	const bool openOk = t.is_open();

	if (!openOk) {
		return "";
	}

	std::string str;

	t.seekg(0, std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	return str;
}

inline unsigned long long Utils::rnd(const unsigned long long max) {

	static std::default_random_engine rng(std::random_device{}());
	static const unsigned long long limit = std::numeric_limits<unsigned long long>::max() - 1;
	static std::uniform_int_distribution<unsigned long long> dist(0, max > 0 ? max : limit);
	return (unsigned long long)dist(rng);
}

Utils::IdType Utils::newID() {
	const auto id = rnd();
	return id;
}