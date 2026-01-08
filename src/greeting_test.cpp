#include <greeting.hpp>

#include <userver/utest/utest.hpp>

using coordinator::UserType;

UTEST(SayHelloTo, Basic) {
    EXPECT_EQ(coordinator::SayHelloTo("Developer", UserType::kFirstTime), "Hello, Developer!\n");
    EXPECT_EQ(coordinator::SayHelloTo({}, UserType::kFirstTime), "Hello, unknown user!\n");

    EXPECT_EQ(coordinator::SayHelloTo("Developer", UserType::kKnown), "Hi again, Developer!\n");
    EXPECT_EQ(coordinator::SayHelloTo({}, UserType::kKnown), "Hi again, unknown user!\n");
}