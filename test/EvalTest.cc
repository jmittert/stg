#include "UnitTest++/UnitTest++.h"
#include "eval.hh"
#include "parser_driver.hh"

SUITE(Eval) {
  TEST(simple) {
    parser_driver driver;
    driver.parse("../test/simple.stg");
    Prog* p = driver.p;
    int res = eval(*p);
    CHECK_EQUAL(0, res);
  }

  TEST(simple2) {
    parser_driver driver;
    driver.parse("../test/simple2.stg");
    Prog* p = driver.p;
    int res = eval(*p);
    CHECK_EQUAL(1, res);
  }
  TEST(op) {
    parser_driver driver;
    driver.parse("../test/op.stg");
    Prog* p = driver.p;
    int res = eval(*p);
    CHECK_EQUAL(3, res);
  }

  TEST(func) {
    parser_driver driver;
    driver.parse("../test/func.stg");
    Prog* p = driver.p;
    int res = eval(*p);
    CHECK_EQUAL(6, res);
  }
}
