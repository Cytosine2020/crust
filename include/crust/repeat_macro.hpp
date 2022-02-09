#ifndef CRUST_REPEAT_MACRO_HPP
#define CRUST_REPEAT_MACRO_HPP


#define CRUST_MACRO_REPEAT_1(FN) FN(0)
#define CRUST_MACRO_REPEAT_2(FN)                                               \
  CRUST_MACRO_REPEAT_1(FN);                                                    \
  FN(1)
#define CRUST_MACRO_REPEAT_3(FN)                                               \
  CRUST_MACRO_REPEAT_2(FN);                                                    \
  FN(2)
#define CRUST_MACRO_REPEAT_4(FN)                                               \
  CRUST_MACRO_REPEAT_3(FN);                                                    \
  FN(3)
#define CRUST_MACRO_REPEAT_5(FN)                                               \
  CRUST_MACRO_REPEAT_4(FN);                                                    \
  FN(4)
#define CRUST_MACRO_REPEAT_6(FN)                                               \
  CRUST_MACRO_REPEAT_5(FN);                                                    \
  FN(5)
#define CRUST_MACRO_REPEAT_7(FN)                                               \
  CRUST_MACRO_REPEAT_6(FN);                                                    \
  FN(6)
#define CRUST_MACRO_REPEAT_8(FN)                                               \
  CRUST_MACRO_REPEAT_7(FN);                                                    \
  FN(7)
#define CRUST_MACRO_REPEAT_9(FN)                                               \
  CRUST_MACRO_REPEAT_8(FN);                                                    \
  FN(8)
#define CRUST_MACRO_REPEAT_10(FN)                                              \
  CRUST_MACRO_REPEAT_9(FN);                                                    \
  FN(9)
#define CRUST_MACRO_REPEAT_11(FN)                                              \
  CRUST_MACRO_REPEAT_10(FN);                                                   \
  FN(10)
#define CRUST_MACRO_REPEAT_12(FN)                                              \
  CRUST_MACRO_REPEAT_11(FN);                                                   \
  FN(11)
#define CRUST_MACRO_REPEAT_13(FN)                                              \
  CRUST_MACRO_REPEAT_12(FN);                                                   \
  FN(12)
#define CRUST_MACRO_REPEAT_14(FN)                                              \
  CRUST_MACRO_REPEAT_13(FN);                                                   \
  FN(13)
#define CRUST_MACRO_REPEAT_15(FN)                                              \
  CRUST_MACRO_REPEAT_14(FN);                                                   \
  FN(14)
#define CRUST_MACRO_REPEAT_16(FN)                                              \
  CRUST_MACRO_REPEAT_15(FN);                                                   \
  FN(15)
#define CRUST_MACRO_REPEAT(N, FN) CRUST_MACRO_REPEAT_##N(FN)


#endif // CRUST_REPEAT_MACRO_HPP
