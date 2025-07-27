#include <iostream>
extern void* malloc_at(size_t size, size_t offset);
int main ()
{
	void* addr = malloc_at(4, 512);

	std::cout << addr << std::endl;
}
