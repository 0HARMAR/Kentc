#include <string>

using namespace std;

string trim(const string& str)
{
	size_t start = 0;
	size_t end = str.length();

	while (start < end && isspace(static_cast<unsigned char>(str[start]))) start++;

	while (end > start && isspace(static_cast<unsigned char>(str[end - 1]))) end--;

	return str.substr(start, (end - start));
}