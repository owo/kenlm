#ifndef UTIL_STREAM_CONFIG__
#define UTIL_STREAM_CONFIG__

#include <cstddef>
#include <string>

namespace util { namespace stream {

struct ChainConfig {
  std::size_t entry_size;
  std::size_t block_count;
  // Chain's constructor will make this a multiple of entry_size. 
  std::size_t total_memory;
};

struct SortConfig {
  std::string temp_prefix;

  // Size of each input/output buffer.
  std::size_t buffer_size;

  // Total memory to use when running alone.
  std::size_t total_memory;
};

}} // namespaces
#endif // UTIL_STREAM_CONFIG__
