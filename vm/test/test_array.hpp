#include "vm/test/test.hpp"

#include "builtin/array.hpp"
#include "builtin/fixnum.hpp"
#include "builtin/tuple.hpp"

#include "ffi_util.hpp"

class TestArray : public CxxTest::TestSuite, public VMTest {
  public:

  void setUp() {
    create();
  }

  void tearDown() {
    destroy();
  }

  void test_allocate() {
    Class* sub = state->new_class("ArraySub", G(array), 0);
    Array* ary = Array::allocate(state, sub);

    TS_ASSERT_EQUALS(ary->klass(), sub);
  }

  void test_aref() {
    Array* ary = Array::create(state, 2);
    ary->set(state, 0, Fixnum::from(1));
    ary->set(state, 1, Fixnum::from(4));

    TS_ASSERT_EQUALS(Fixnum::from(4), ary->aref(state, Fixnum::from(1)));
    TS_ASSERT_EQUALS(Qnil, ary->aref(state, Fixnum::from(2)));

    // simulate #shift
    ary->start(state, Fixnum::from(1));
    ary->total(state, Fixnum::from(1));

    TS_ASSERT_EQUALS(Fixnum::from(4), ary->aref(state, Fixnum::from(0)));
    TS_ASSERT_EQUALS(Qnil, ary->aref(state, Fixnum::from(1)));
  }

  void test_aref_negative() {
    Array* ary = Array::create(state, 3);
    ary->set(state, 0, Fixnum::from(1));
    ary->set(state, 1, Fixnum::from(4));
    ary->set(state, 2, Fixnum::from(9));

    TS_ASSERT_EQUALS(Fixnum::from(9),
		     ary->aref(state, Fixnum::from(-1)));
    TS_ASSERT_EQUALS(Fixnum::from(1), ary->aref(state, Fixnum::from(-3)));
    TS_ASSERT_EQUALS(Qnil, ary->aref(state, Fixnum::from(-4)));

    // simulate #shift
    ary->start(state, Fixnum::from(1));
    ary->total(state, Fixnum::from(2));

    TS_ASSERT_EQUALS(Fixnum::from(9), ary->aref(state, Fixnum::from(-1)));
    TS_ASSERT_EQUALS(Fixnum::from(4), ary->aref(state, Fixnum::from(-2)));
    TS_ASSERT_EQUALS(Qnil, ary->aref(state, Fixnum::from(-3)));
  }

  void test_aset() {
    Array* ary = Array::create(state, 2);
    ary->aset(state, Fixnum::from(1), Fixnum::from(4));

    TS_ASSERT_EQUALS(4, as<Fixnum>(ary->get(state, 1))->to_native());
  }

  void test_get() {
    Array* ary = Array::create(state, 3);
    TS_ASSERT_EQUALS(Qnil, ary->get(state, 5));

    ary->set(state, 0, Qtrue);

    TS_ASSERT_EQUALS(Qtrue, ary->get(state, 0));
  }

  void test_set_resizes() {
    Array* ary = Array::create(state, 3);

    ary->set(state, 0, Fixnum::from(33));
    ary->set(state, 10, Qtrue);

    TS_ASSERT(ary->tuple()->num_fields() > 3);
    TS_ASSERT_EQUALS(ary->get(state, 0), Fixnum::from(33));

  }

  void test_unshift() {
    Array* ary = Array::create(state, 1);
    ary->set(state, 0, Qtrue);
    ary->unshift(state, Qfalse);

    TS_ASSERT_EQUALS(ary->size(), 2U);
    TS_ASSERT_EQUALS(ary->get(state, 0), Qfalse);
    TS_ASSERT_EQUALS(ary->get(state, 1), Qtrue);

    TS_ASSERT_EQUALS(ary->shift(state),Qfalse);
    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(1)));
    ary->unshift(state, Qnil);
    TS_ASSERT_EQUALS(ary->get(state, 0), Qnil);
    TS_ASSERT_EQUALS(ary->get(state, 1), Qtrue);
  }

  void test_pop() {
    Array* ary = Array::create(state, 3);
    ary->set(state, 0, Fixnum::from(1));
    ary->set(state, 1, Fixnum::from(4));
    ary->set(state, 2, Fixnum::from(9));

    TS_ASSERT_EQUALS(ary->pop(state), Fixnum::from(9));
    TS_ASSERT_EQUALS(ary->pop(state), Fixnum::from(4));
    TS_ASSERT_EQUALS(ary->pop(state), Fixnum::from(1));
    TS_ASSERT_EQUALS(ary->pop(state), Qnil);
  }

  void test_includes_p() {
    Array* ary = Array::create(state, 1);
    ary->set(state, 0, Qtrue);

    TS_ASSERT(ary->includes_p(state, Qtrue));
    TS_ASSERT(!ary->includes_p(state, Qfalse));
  }

  void test_shift() {
    Array* ary = Array::create(state, 2);
    ary->set(state, 0, Fixnum::from(1));
    ary->set(state, 1, Fixnum::from(4));
    ary->set(state, 2, Fixnum::from(9));

    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(0)));
    TS_ASSERT_EQUALS(ary->total(), as<Integer>(Fixnum::from(3)));

    TS_ASSERT_EQUALS(ary->shift(state), Fixnum::from(1));
    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(1)));
    TS_ASSERT_EQUALS(ary->total(), as<Integer>(Fixnum::from(2)));

    TS_ASSERT_EQUALS(ary->shift(state), Fixnum::from(4));
    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(2)));
    TS_ASSERT_EQUALS(ary->total(), as<Integer>(Fixnum::from(1)));

    TS_ASSERT_EQUALS(ary->shift(state), Fixnum::from(9));
    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(3)));
    TS_ASSERT_EQUALS(ary->total(), as<Integer>(Fixnum::from(0)));

    TS_ASSERT_EQUALS(ary->shift(state), Qnil);
    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(3)));
    TS_ASSERT_EQUALS(ary->total(), as<Integer>(Fixnum::from(0)));

    ary->append(state, Qtrue);
    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(3)));
    TS_ASSERT_EQUALS(ary->total(), as<Integer>(Fixnum::from(1)));

    TS_ASSERT_EQUALS(ary->shift(state), Qtrue);
    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(4)));
    TS_ASSERT_EQUALS(ary->total(), as<Integer>(Fixnum::from(0)));

    ary->unshift(state, Qfalse);
    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(3)));
    TS_ASSERT_EQUALS(ary->total(), as<Integer>(Fixnum::from(1)));

    TS_ASSERT_EQUALS(ary->shift(state), Qfalse);
    TS_ASSERT_EQUALS(ary->start(), as<Integer>(Fixnum::from(4)));
    TS_ASSERT_EQUALS(ary->total(), as<Integer>(Fixnum::from(0)));
  }
};
