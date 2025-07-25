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


// malloc entry
void* custom_malloc_at(size_t size, size_t offset);
extern "C" void* malloc_at(size_t size, size_t offset)
{
	custom_malloc_at(size, offset);
}

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

// alloc from free linklist
static void* allocate_from_free_list(size_t size)
{
	size_t class_idx = get_size_class_index(size);
	for (size_t i = class_idx; i < NUM_SIZE_CLASSES; ++i)
	{
		FreeNode* curr = free_lists[i];
		while (curr)
		{
			size_t block_size = curr -> size;
			size_t payload_size = block_size - 2 * sizeof(size_t);

			// check whether you should allocate
			if (payload_size >= size)
			{
				// remove from linklist
				if (curr -> prev) curr -> prev -> next = curr -> next;
				else free_lists[i] = curr -> next;

				if (curr -> next) curr -> next -> prev = curr -> prev;

				// calculate rest space after spilt (at least remain min block)
				const size_t min_remain = 4 * sizeof(size_t);
				if (block_size - size - 2 * sizeof(size_t) >= min_remain)
				{
					// calculate split point
					void* split_at = reinterpret_cast<void*> (
						reinterpret_cast<uintptr_t>(curr) + sizeof(size_t));

					// set rest part to independent block
					size_t remain_size = block_size - size - sizeof(size_t);
					set_block_header(split_at, remain_size, false);

					// add rest block to free list
					FreeNode* new_node = reinterpret_cast<FreeNode*>(
						reinterpret_cast<uintptr_t>(split_at) + sizeof(size_t));

					size_t new_class = get_size_class_index(remain_size - sizeof(size_t));
					new_node -> size = remain_size;
					new_node -> prev = nullptr;
					new_node -> next = free_lists[new_class];
					if (free_lists[new_class]) free_lists[new_class] -> prev = new_node;
					free_lists[new_class] = new_node;

					// update allocate block size
					block_size = size + sizeof(size_t);
				}

				// mark to used
				set_block_header(curr, block_size, true);
				return reinterpret_cast<void*> (
					reinterpret_cast<uintptr_t>(curr) + sizeof(size_t));
			}
			curr = curr -> next;
		}
	}
	return nullptr;
}

// init heap
void init_heap(void* base_addr, size_t heap_size)
{
	std::lock_guard<std::mutex> lock(alloc_mutex);
	heap_base = reinterpret_cast<uintptr_t>(base_addr);

	// init total heap block
	FreeNode* free_node = reinterpret_cast<FreeNode*>(base_addr + sizeof(size_t));
	free_node -> prev = nullptr;
	free_node -> next = nullptr;
	free_node -> size = heap_size;

	// write block header
	set_block_header(free_node, heap_size, false);

	// add to free list
	size_t class_idx = get_size_class_index(heap_size - 2 * sizeof(size_t));
	free_lists[class_idx] = free_node;
}

// custom malloc
void* custom_malloc(size_t size)
{
	if (size == 0) return nullptr;

	std::lock_guard<std::mutex> lock(alloc_mutex);
	size_t aligned_size = align_up(size);

	// try to allocate from free list
	void* block = allocate_from_free_list(aligned_size);
	if (!block)
	{
		// heap memory exhausted
		return nullptr;
	}
	return block;
}

// custom malloc at address
void* custom_malloc_at(size_t size, size_t offset)
{
	if (size == 0 || heap_base == 0) return nullptr;

	std::lock_guard<std::mutex> lock(alloc_mutex);
	size_t aligned_size = align_up(size);
	const uintptr_t target_addr = heap_base + offset;

	// target block start addr
	void* target_block = reinterpret_cast<void*> (target_addr - sizeof(size_t));

	// check addr whether take effect
	const size_t min_addr = heap_base + sizeof(size_t);
	const size_t max_addr = heap_base + get_block_size(
		reinterpret_cast<void*>(heap_base + sizeof(size_t)));
	if (target_addr < min_addr || target_addr + aligned_size >= max_addr)
	{
		return nullptr; // out of heap
	}

	// check target space whether free
	for (size_t i = 0; i < NUM_SIZE_CLASSES; ++i)
	{
		FreeNode* curr = free_lists[i];
		while (curr)
		{
			uintptr_t start_addr = reinterpret_cast<uintptr_t>(curr) - sizeof(size_t);
			uintptr_t end_addr = start_addr + curr -> size;

			// check whether target in current block
			if (reinterpret_cast<uintptr_t>(target_block) >= start_addr &&
				reinterpret_cast<uintptr_t>(target_block) + aligned_size + sizeof(size_t) <= end_addr)
			{
				// remove from linklist
				if (curr -> prev) curr -> prev -> next = curr -> next;
				else free_lists[i] = curr -> next;

				if (curr -> next) curr -> next -> prev = curr -> prev;

				// create new block in target pos
				set_block_header(target_block, aligned_size + sizeof(size_t), true);

				// process front rest space
				size_t before_size = reinterpret_cast<uintptr_t>(target_block) - start_addr;
				if (before_size >= 3 * sizeof(size_t))
				{
					set_block_header(reinterpret_cast<void*>(start_addr), before_size, false);
					FreeNode* before = reinterpret_cast<FreeNode*>(start_addr + sizeof(size_t));
					before -> size = before_size;
					before -> prev = nullptr;
					before -> next = free_lists[get_size_class_index(before_size - 2 * sizeof(size_t))];
    				if (before->next) before->next->prev = before;
    				free_lists[get_size_class_index(before_size - 2 * sizeof(size_t))] = before;
				}

				// process after rest space
				size_t after_addr = reinterpret_cast<uintptr_t>(target_block) + start_addr;
				size_t after_size = end_addr - after_addr;
				if (after_size >= 3 * sizeof(size_t)) { // min block
					set_block_header(reinterpret_cast<void*>(after_addr), after_size, false);
					FreeNode* after = reinterpret_cast<FreeNode*>(after_addr + sizeof(size_t));
					after -> size = after_size;
					after -> prev = nullptr;
					after -> next = free_lists[get_size_class_index(after_size - 2 * sizeof(size_t))];
					if (after->next) after -> next -> prev = after;
					free_lists[get_size_class_index(after_size - 2 * sizeof(size_t))] = after;
				}

				return reinterpret_cast<void*>(target_addr);
			}
			curr = curr -> next;
		}
	}
	return nullptr;
}

// free memory
void custom_free(void* ptr)
{
	if (!ptr || heap_base == 0) return;

	std::lock_guard<std::mutex> lock(alloc_mutex);
	size_t* header = get_block_header(ptr);
	size_t block_size = *header & BLOCK_MASK;
	bool is_free = (*header & BLOCK_MASK) == 0;

	if (is_free)
	{
		return;
	}

	// mark free
	set_block_header(ptr, block_size, false);

	// add to free list
	FreeNode* node = reinterpret_cast<FreeNode*>(reinterpret_cast<uintptr_t>(ptr) - sizeof(size_t) + sizeof(size_t));
	size_t payload_size = block_size - 2 * sizeof(size_t);
	size_t class_idx = get_size_class_index(payload_size);

	node -> size = block_size;
	node -> prev = nullptr;
	node -> next = free_lists[class_idx];
	if (free_lists[class_idx]) free_lists[class_idx] -> prev = node;
	free_lists[class_idx] = node;
}

