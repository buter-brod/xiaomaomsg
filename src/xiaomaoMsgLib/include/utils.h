#ifndef UTILS_H
#define UTILS_H
#include <string>


class Utils {
public:

	typedef unsigned long long IdType;

	static std::string loadFile(const std::string& filename);

	static unsigned long long rnd(const unsigned long long max = 0);

	static IdType newID();
};


#endif