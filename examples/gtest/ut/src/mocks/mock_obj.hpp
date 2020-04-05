/**
 * Example mocking of the Obj() object
 *
 * More details can be found here:
 *    https://github.com/google/googletest/blob/master/googlemock/docs/ForDummies.md#how-to-define-it
 */

#ifndef __MOCKS_MOCK_OBJ_HPP__
#define __MOCKS_MOCK_OBJ_HPP__


#include "gmock/gmock.h"

#include "obj.hpp"


class MockObj : public Obj {
    public:
        MOCK_METHOD1(is_valid, bool(std::string));
        MOCK_METHOD0(get_number, int());
};


#endif  // __MOCKS_MOCK_OBJ_HPP__
