#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <vector>

// memory block alignment (byte)
constexpr size_t ALIGNMENT = 8;

// heap memory start address
static uintptr_t heap_base = 600000;

// free list node (embed in free block)
struct FreeNode
{
	FreeNode* prev;
	FreeNode* next;
	size_t size; // include head end info total block size
};

// memory sign bit
constexpr size_t BLOCK_IN_USE = 0x1UL;
constexpr size_t BLOCK_FREE = 0x0UL;
constexpr size_t BLOCK_MASK = ~(0x7UL); // size alignment mask (8 bytes)

// separate the free linked list (according to size)
constexpr size_t NUM_SIZE_CLASSES = 10;
static FreeNode* free_lists[NUM_SIZE_CLASSES] = { nullptr };

// thread safe mutex
static std::mutex alloc_mutex;

// assist function : align to up
static inline size_t align_up(size_t n)
{
	return (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

// get memory block head info (address)
static inline size_t* get_block_header(void* ptr)
{
	return reinterpret_cast<size_t*> (
		reinterpret_cast<uintptr_t>(ptr) - sizeof(size_t));
}

// get memory block size info
static inline size_t get_block_size(void* ptr)
{
	return (*get_block_header(ptr)) & BLOCK_MASK;
}

// get block stat (in use or free)
static inline bool is_block_free(void* ptr)
{
	return !(*(get_block_header(ptr)) & BLOCK_IN_USE);
}

// set block header info
static inline void set_block_header(void* ptr, size_t size, bool in_use)
{
	size_t* header = get_block_header(ptr);
	*header = size | (in_use ? BLOCK_IN_USE : BLOCK_FREE);
}

// get size class index
static size_t get_size_class_index(size_t size)
{
	static const size_t class_limits[] = {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, (size_t) - 1};
	for (size_t i = 0; i < NUM_SIZE_CLASSES; ++i)
	{
		if (size <= class_limits[i])
		{
			return i;
		}
	}
	return NUM_SIZE_CLASSES - 1;
}

