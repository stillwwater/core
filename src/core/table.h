#ifndef CORE_TABLE_H_
#define CORE_TABLE_H_

#include <string.h>

#include "core/basic.h"
#include "core/memory.h"

template <typename K, typename V>
struct Table_Pair {
    K key;
    V value;
};

// Each entry contains an extra 8 bytes of metadata. The highest two bits
// indicate whether the entry is empty or removed. A signature also contains
// the 62 lower bits of the original hash. This hash can be used to disabiguate
// between entries without comparing the keys when resolving hash collisions.
// This value also avoids rehashing keys during a table resize.
struct Table_Signature {
    enum : u64 { EMPTY = 0ull, TOMBSTONE = ~0ull };
    u64 value;
};

template <typename K, typename V>
struct Table_Entry : Table_Pair<K, V> {
    Table_Signature sign;
};

// Hash table
//   * Entries are stored in a flat array using open addressing.
//   * Collisions are resolved with quadratic probing.
//   * Entries are not ordered, iterating returns entries in arbitrary order.
//   * Maintains a load factor <70%.
//   * The table capacity is always a power of 2.
//   * An empty table starts with 8 empty entries.
//   * If the key requires allocation it is handled by the caller.
//   * Destructors are not called.
//   * Does not guarantee pointer stability (unlike std::unordered_map).
template <typename K, typename V>
struct Table {
    enum { LOAD_FACTOR = 70, MIN_CAPACITY = 8 };

    struct iterator {
        Table_Entry<K, V> *ptr;
        Table_Entry<K, V> *end;

        const Table_Pair<K, V> &
        operator*()
        {
            return *static_cast<Table_Pair<K, V> *>(ptr);
        }

        iterator &
        operator++()
        {
            ++ptr;
            for (; ptr != end && table_is_sentinel_entry(ptr); ++ptr);
            return *this;
        }

        iterator
        operator++(int)
        {
            iterator it = *this;
            ++(*this);
            return it;
        }

        bool operator==(const iterator &it) const { return it.ptr == ptr; }
        bool operator!=(const iterator &it) const { return it.ptr != ptr; }
    };

    usize count;
    Table_Entry<K, V> *entries;
    usize capacity;
    Allocator *allocator;
    u64 (*hash_func)(K *key);

    V &operator[](const K &key);
};

// FNV-1a hash function.
inline u64
hash_fnv1a(const u8 *key, usize len)
{
    u64 h = 0xCBF29CE484222325ull;
    for (; len; --len) {
        h ^= (u64)(*key++);
        h *= 0x00000100000001B3ull;
    }
    return h;
}

template <typename K>
u64
table_auto_hash(K *key)
{
    if constexpr (is_slice<K>()) {
        return hash_fnv1a((const u8 *)key->data, sizeof(*key->data) * key->count);
    } else {
        return hash_fnv1a((const u8 *)key, sizeof(*key));
    }
}

// Creates a signature from a hash value for an occupied table entry.
inline Table_Signature
table_create_signature(u64 hash)
{
    Table_Signature sign;
    sign.value = (hash >> 2) | 0x8000000000000000;
    return sign;
}

// Returns true if an entry is an empty entry or tombstone.
template <typename K, typename V>
bool
table_is_sentinel_entry(Table_Entry<K, V> *entry)
{
    return entry->sign.value == 0 || entry->sign.value == ~0ull;
}

template <typename K, typename V>
Table<K, V>
make_table(u64 (*hash_func)(K *key) = table_auto_hash<K>, ALLOCATOR_PARAM(allocator))
{
    Table<K, V> table;
    table.count = 0;
    table.entries = NULL;
    table.capacity = 0;
    table.allocator = allocator;
    table.hash_func = hash_func;
    return table;
}

template <typename K, typename V>
void
deinit(Table<K, V> table)
{
    mem::free(table.entries);
}

// Returns an entry matching a key. This overload takes a signature argument to
// avoid rehashing the key.
template <typename K, typename V>
Table_Entry<K, V> *
table_find_entry(Table<K, V> *table, K key, Table_Signature sign)
{
    if (table->capacity == table->count) return NULL;
    assert(table->entries);

    u64 hash = sign.value & 0x3FFFFFFFFFFFFFFF;
    usize i = hash & (table->capacity - 1);
    usize probe = 1;
    Table_Entry<K, V> *first_removed_slot = NULL;
    Table_Entry<K, V> *entry = &table->entries[i];

    for (; entry->sign.value != 0; entry = &table->entries[i]) {
        if (entry->sign.value == sign.value && key == entry->key) {
            return entry;
        }
        if (entry->sign.value == ~0ull && !first_removed_slot) {
            first_removed_slot = entry;
        }
        i += probe;
        probe += 1;
        for (; i >= table->capacity;) i -= table->capacity;
    }

    if (first_removed_slot) return first_removed_slot;
    return entry;
}

// Returns an entry matching a key. If the entry does not exist, the next empty
// entry is retuned. The signature of the returned entry can be used to
// determine if it is an empty entry. If the entry returned is empty or
// removed, key.value may be uninitialized memory.
template <typename K, typename V>
Table_Entry<K, V> *
table_find_entry(Table<K, V> *table, K key)
{
    u64 hash = table->hash_func(&key);
    auto sign = table_create_signature(hash);
    return table_find_entry(table, key, sign);
}

// Returns a pointer to the value associated with a key, or NULL if the key
// does not exist in the table.
template <typename K, typename V>
V *
table_find(Table<K, V> *table, K key)
{
    if (table->capacity == 0) return NULL;
    auto entry = table_find_entry(table, key);
    if (!table_is_sentinel_entry(entry)) {
        return &entry->value;
    }
    return NULL;
}

// Resizes the table to have a certain capacity. The given capacity must be
// larger than the current table capacity. The given capacity will be rounded
// to the nearest power of two value. This is an expensive operation which
// reinserts every entry in the table, though it does not need to rehash the
// keys. The capacity must be larger than the current table capacity.
template <typename K, typename V>
void
table_expand(Table<K, V> *table, usize capacity)
{
    assert(capacity != 0);
    assert(capacity >= table->capacity);
    capacity = nextpow2(capacity);

    Table<K, V> copy;
    copy.count = table->count;
    copy.capacity = capacity;
    copy.allocator = table->allocator;
    copy.hash_func = table->hash_func;

    copy.entries = (Table_Entry<K, V> *)mem::alloc(
            capacity * sizeof(Table_Entry<K, V>),
            alignof(Table_Entry<K, V>),
            table->allocator);
    memset(copy.entries, 0, capacity * sizeof(Table_Entry<K, V>));

    auto end = table->entries + table->capacity;
    for (auto ptr = table->entries; ptr != end; ++ptr) {
        if (!table_is_sentinel_entry(ptr)) {
            auto entry = table_find_entry(&copy, ptr->key, ptr->sign);
            entry->key = ptr->key;
            entry->value = ptr->value;
            entry->sign = ptr->sign;
        }
    }
    mem::free(table->entries, table->allocator);
    *table = copy;
}

// Creates a new non-empty entry for a key. This function will reuse an
// existing entry if it already exists. `entry.value` may be unitialized
// memory.
template <typename K, typename V>
Table_Entry<K, V> *
table_create_entry(Table<K, V> *table, K key)
{
    usize load_factor = (table->count + 1) * 100;
    if (load_factor >= table->capacity * Table<K, V>::LOAD_FACTOR) {
        if (table->capacity >= Table<K, V>::MIN_CAPACITY) {
            table_expand(table, table->capacity << 1);
        } else {
            table_expand(table, Table<K, V>::MIN_CAPACITY);
        }
    }

    usize hash = table->hash_func(&key);
    auto sign = table_create_signature(hash);
    auto entry = table_find_entry(table, key, sign);
    if (table_is_sentinel_entry(entry)) {
        entry->key = key;
        entry->sign = sign;
        table->count += 1;
    }
    return entry;
}

// Creates a new entry in the table with the given key or updates an existing
// entry with a new value.
template <typename K, typename V>
V *
table_update(Table<K, V> *table, K key, V value)
{
    auto entry = table_create_entry(table, key);
    entry->value = value;
    return &entry->value;
}

// Removes an entry from the table. Returns true if an entry with the given key
// existed in the table.
template <typename K, typename V>
bool
table_remove(Table<K, V> *table, const K &key)
{
    if (table->count > 0) {
        auto entry = table_find_entry(table, key);
        if (!table_is_sentinel_entry(entry)) {
            entry->sign.value = ~0ull;
            --table->count;
            return true;
        }
    }
    return false;
}

// Resets the table by setting all entries to the empty sentinel and resetting
// .count to zero. Does not free existing memory so .capacity is unchanced
template <typename K, typename V>
void
table_clear(Table<K, V> *table)
{
    memset(table->entries, 0, table->capacity);
    table->count = 0;
}

template <typename K, typename V>
typename Table<K, V>::iterator
begin(Table<K, V> table)
{
    auto begin_ = table.entries;
    auto end_ = table.entries + table.capacity;
    for (; begin_ != end_ && table_is_sentinel_entry(begin_); ++begin_);
    return typename Table<K, V>::iterator{begin_, end_};
}

template <typename K, typename V>
typename Table<K, V>::iterator
end(Table<K, V> table)
{
    auto end_ = table.entries + table.capacity;
    return typename Table<K, V>::iterator{end_, end_};
}

template <typename K, typename V>
V &
Table<K, V>::operator[](const K &key)
{
    auto entry = table_create_entry(this, key);
    return entry->value;
}

#endif // CORE_TABLE_H_
