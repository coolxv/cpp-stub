/**
 * Fixture to repeat a test many times with different supplied strings
 */


#ifndef __FIXTURES_STRINGS_HPP__
#define __FIXTURES_STRINGS_HPP__


#include "gtest/gtest.h"

#include <string>


class StringsFixture : public ::testing::TestWithParam<std::string>
{
    public:
        virtual void SetUp()
        {
            // Do some setup
        }
        virtual void TearDown()
        {
            // Do some tear down
        }
};


INSTANTIATE_TEST_SUITE_P(
    StringsFixtureName,  // Instantiation name
    StringsFixture,  // Fixture controller
    ::testing::Values(  // Parameters
        "Kano",
        "KanoTest",
        "Kano123",
        "123Kano",
        "^$K@",
        "Kano%£("
    )
);


#endif  // __FIXTURES_STRINGS_HPP__
