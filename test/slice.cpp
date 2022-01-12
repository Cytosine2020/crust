#include "gtest/gtest.h"

#include "utility.hpp"
#include "slice.hpp"


using namespace crust;


GTEST_TEST(slice, slice) {
  constexpr usize BUFFER_SIZE = 10;

  u8 buffer[BUFFER_SIZE]{};

  auto slice = Slice<u8>::from_raw_parts(buffer, BUFFER_SIZE);

  EXPECT_EQ(slice[1], 0);
}
