/**
 * Example tests of the Obj() class
 */


// Test framework
#include "gtest/gtest.h"
#include "gmock/gmock.h"

//Stub
#include "stub.h"

// Fixtures
#include "fixtures/strings.hpp"

// Mocks
#include "mocks/mock_obj.hpp"

// Functions to test
#include "obj.hpp"
#include "obj.cpp"
#include "obj_operations.hpp"
#include "obj_operations.cpp"

// Add useful testing functions to namespace
using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::Exactly;


//stub function
int get_number_stub(void* obj)
{
    return 5;
}




/**
 * Basic assertion of Obj::get_number()
 *
 * More info can be found here:
 *     https://github.com/google/googletest/blob/master/googletest/docs/Primer.md#simple-tests
 */
TEST(get_number_test, returns_correct_value) {
    Stub stub;
    stub.set(ADDR(Obj, get_number), get_number_stub);
    Obj obj;
    ASSERT_EQ(obj.get_number(), 5);
}


/**
 * Simple parameterised test which checks the Obj::is_valid(std::string)
 * function with the set of string parameters provided in
 * `fixtures/strings.hpp`
 *
 * More info can be found here:
 *     https://github.com/google/googletest/blob/master/googletest/docs/Primer.md#basic-concepts
 */
TEST_P(StringsFixture, test_string_valid) {
    // Retrieve the test parameter from the fixture
    auto test_str = this->GetParam();

    Obj obj;
    ASSERT_EQ(obj.is_valid(test_str), test_str.find("Kano") == 0);
}


/**
 * Example of testing a function with a mock object.
 *
 * Tests the `check_obj(Obj&)` function of `obj_operations.cpp` by
 * mocking the Obj() object (provided in `mocks/mock_obj.hpp`) and
 * overrides the mocked functions to return the desired data upon
 * each call.
 *
 * More info can be found here:
 *     https://github.com/google/googletest/blob/master/googlemock/docs/ForDummies.md#using-mocks-in-tests
 */
TEST(ObjTest, test_obj_operations_with_mocks) {
    MockObj obj;

    EXPECT_CALL(obj, is_valid(_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(obj, get_number())
        .Times(AtLeast(2))
        .WillOnce(Return(8))
        .WillRepeatedly(Return(7));

    EXPECT_TRUE(check_obj(obj));
}
