#include <gtest/gtest.h>

#include "../server/include/server/Config.hpp"

TEST(ConfigTest, DefaultValues) {
    const char *argv[] = {"server"};
    server::Config config =
        server::Config::Parse(1, const_cast<char **>(argv));

    EXPECT_EQ(config.GetTcpPort(), 50000);
    EXPECT_EQ(config.GetUdpPort(), 50000);
    EXPECT_EQ(config.GetMaxPlayers(), 4);
}

TEST(ConfigTest, PositionalPorts) {
    const char *argv[] = {"server", "6000", "7000"};
    server::Config config =
        server::Config::Parse(3, const_cast<char **>(argv));

    EXPECT_EQ(config.GetTcpPort(), 6000);
    EXPECT_EQ(config.GetUdpPort(), 7000);
    EXPECT_EQ(config.GetMaxPlayers(), 4);
}

TEST(ConfigTest, SinglePortDefaultsUdp) {
    const char *argv[] = {"server", "6000"};
    server::Config config =
        server::Config::Parse(2, const_cast<char **>(argv));

    EXPECT_EQ(config.GetTcpPort(), 6000);
    EXPECT_EQ(config.GetUdpPort(), 6000);
}

TEST(ConfigTest, MaxPlayersFlag) {
    const char *argv[] = {"server", "-p", "8"};
    server::Config config =
        server::Config::Parse(3, const_cast<char **>(argv));

    EXPECT_EQ(config.GetMaxPlayers(), 8);
    EXPECT_EQ(config.GetTcpPort(), 50000);
}

TEST(ConfigTest, MaxPlayersAndPorts) {
    const char *argv[] = {"server", "6000", "7000", "-p", "10"};
    server::Config config =
        server::Config::Parse(5, const_cast<char **>(argv));

    EXPECT_EQ(config.GetTcpPort(), 6000);
    EXPECT_EQ(config.GetUdpPort(), 7000);
    EXPECT_EQ(config.GetMaxPlayers(), 10);
}

TEST(ConfigTest, MaxPlayersAndPortsMixed) {
    // Note: The current implementation expects positional args first or flags
    // anywhere? Looking at the loop: it iterates 1 to argc.
    // If it sees -p, it consumes next arg.
    // Else it treats as positional.
    // So "server -p 10 6000 7000" should work.
    const char *argv[] = {"server", "-p", "10", "6000", "7000"};
    server::Config config =
        server::Config::Parse(5, const_cast<char **>(argv));

    EXPECT_EQ(config.GetTcpPort(), 6000);
    EXPECT_EQ(config.GetUdpPort(), 7000);
    EXPECT_EQ(config.GetMaxPlayers(), 10);
}

TEST(ConfigTest, InvalidMaxPlayersTooLow) {
    const char *argv[] = {"server", "-p", "0"};
    EXPECT_THROW(
        { server::Config::Parse(3, const_cast<char **>(argv)); },
        std::out_of_range);
}

TEST(ConfigTest, InvalidMaxPlayersTooHigh) {
    const char *argv[] = {"server", "-p", "256"};
    EXPECT_THROW(
        { server::Config::Parse(3, const_cast<char **>(argv)); },
        std::out_of_range);
}

TEST(ConfigTest, InvalidMaxPlayersNotNumber) {
    const char *argv[] = {"server", "-p", "abc"};
    // stoi throws invalid_argument
    EXPECT_THROW(
        { server::Config::Parse(3, const_cast<char **>(argv)); },
        std::invalid_argument);
}

TEST(ConfigTest, MissingMaxPlayersValue) {
    const char *argv[] = {"server", "-p"};
    EXPECT_THROW(
        { server::Config::Parse(2, const_cast<char **>(argv)); },
        std::invalid_argument);
}
