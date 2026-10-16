#pragma once
#include <cstdint>
#include <cstddef>

// Chained hash map: bucket = hash % bucket_count, nodes {key, value, hash, next}.
template <class K, class V>
struct THashMap
{
	struct node
	{
		K key;
		V value;
		uint32_t hash;
		node *next;
	};

	uint32_t bucket_count;
	uint32_t count;
	uint8_t pad0[0x10];
	node *free_list;
	node **buckets;
	uint8_t shrink;
	uint8_t pad1[7];

	template <class F>
	void each(F f) const
	{
		if (!buckets)
			return;

		for (uint32_t i = 0; i < bucket_count; ++i)
		{
			for (node *n = buckets[i]; n; n = n->next)
			{
				f(n->key, n->value);
			}
		}
	}

	const V *find(uint32_t hash, K key) const
	{
		if (!buckets || !bucket_count)
			return nullptr;

		for (node *n = buckets[hash % bucket_count]; n; n = n->next)
		{
			if (n->hash == hash && n->key == key)
				return &n->value;
		}

		return nullptr;
	}
};

using type_map = THashMap<uint32_t, void *>;
using value_map = THashMap<uint32_t, int32_t>;
static_assert(offsetof(type_map, buckets) == 0x20);
static_assert(sizeof(type_map) == 0x30);
static_assert(offsetof(type_map::node, next) == 0x18);
static_assert(offsetof(value_map::node, next) == 0x10);
