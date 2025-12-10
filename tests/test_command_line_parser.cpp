#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "game/CommandLineParser.hpp"

namespace RC = Rtype::Client;

// Helper to convert vector of strings to argc/argv format
class ArgvHelper {
 public:
    explicit ArgvHelper(const std::vector<std::string> &args) : args_(args) {
        argv_.reserve(args_.size());
        for (auto &arg : args_) {
            argv_.push_back(arg.data());
        }
    }

    int Argc() const {
        return static_cast<int>(argv_.size());
    }

    char **Argv() {
        return argv_.data();
    }

 private:
    std::vector<std::string> args_;  // Owns the strings
    std::vector<char *> argv_;       // Points into args_
};

TEST(CommandLineParser, ParsesMinimalValidArguments) {
    ArgvHelper args({"r-type_client", "192.168.1.1", "8080", "TestUser"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.server_ip, "192.168.1.1");
    EXPECT_EQ(config.tcp_port, 8080);
    EXPECT_EQ(config.udp_port, 8080);  // Defaults to TCP port
    EXPECT_EQ(config.username, "TestUser");
}

TEST(CommandLineParser, ParsesWithShortUdpFlag) {
    ArgvHelper args(
        {"r-type_client", "127.0.0.1", "50000", "Player1", "-up", "50001"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.server_ip, "127.0.0.1");
    EXPECT_EQ(config.tcp_port, 50000);
    EXPECT_EQ(config.udp_port, 50001);
    EXPECT_EQ(config.username, "Player1");
}

TEST(CommandLineParser, ParsesWithLongUdpFlag) {
    ArgvHelper args(
        {"r-type_client", "10.0.0.1", "12345", "User", "--udp-port", "54321"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.server_ip, "10.0.0.1");
    EXPECT_EQ(config.tcp_port, 12345);
    EXPECT_EQ(config.udp_port, 54321);
    EXPECT_EQ(config.username, "User");
}

TEST(CommandLineParser, ThrowsOnMissingArguments) {
    ArgvHelper args({"r-type_client", "127.0.0.1"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Missing required arguments");
}

TEST(CommandLineParser, ThrowsOnInvalidTcpPort) {
    ArgvHelper args({"r-type_client", "127.0.0.1", "70000", "User"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid TCP-PORT");
}

TEST(CommandLineParser, ThrowsOnNonNumericTcpPort) {
    ArgvHelper args({"r-type_client", "127.0.0.1", "abc", "User"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid TCP-PORT");
}

TEST(CommandLineParser, ThrowsOnInvalidUdpPort) {
    ArgvHelper args(
        {"r-type_client", "127.0.0.1", "50000", "User", "-up", "0"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid UDP-PORT");
}

TEST(CommandLineParser, ThrowsOnEmptyUsername) {
    ArgvHelper args({"r-type_client", "127.0.0.1", "50000", ""});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "USERNAME cannot be empty");
}

TEST(CommandLineParser, ThrowsOnUsernameTooLong) {
    std::string long_username(33, 'A');  // 33 characters (max is 32)
    ArgvHelper args({"r-type_client", "127.0.0.1", "50000", long_username});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE),
        "USERNAME too long \\(max 32 characters\\)");
}

TEST(CommandLineParser, AcceptsMaxLengthUsername) {
    std::string max_username(32, 'A');  // Exactly 32 characters
    ArgvHelper args({"r-type_client", "127.0.0.1", "50000", max_username});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.username, max_username);
    EXPECT_EQ(config.username.length(), 32u);
}

TEST(CommandLineParser, ThrowsOnMissingUdpPortValue) {
    ArgvHelper args({"r-type_client", "127.0.0.1", "50000", "User", "-up"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE),
        "Missing value for -up/--udp-port flag");
}

TEST(CommandLineParser, ThrowsOnUnknownFlag) {
    ArgvHelper args(
        {"r-type_client", "127.0.0.1", "50000", "User", "--verbose"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Unknown argument");
}

TEST(CommandLineParser, ExitsWithSuccessOnHelpFlag) {
    ArgvHelper args({"r-type_client", "--help"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_SUCCESS), "Usage:");
}

TEST(CommandLineParser, ValidatesPortBoundaries) {
    // Test minimum valid port
    ArgvHelper args_min({"r-type_client", "127.0.0.1", "1", "User"});
    RC::ClientConfig config_min =
        RC::CommandLineParser::Parse(args_min.Argc(), args_min.Argv());
    EXPECT_EQ(config_min.tcp_port, 1);

    // Test maximum valid port
    ArgvHelper args_max({"r-type_client", "127.0.0.1", "65535", "User"});
    RC::ClientConfig config_max =
        RC::CommandLineParser::Parse(args_max.Argc(), args_max.Argv());
    EXPECT_EQ(config_max.tcp_port, 65535);
}

TEST(CommandLineParser, HandlesIPv4Addresses) {
    ArgvHelper args({"r-type_client", "192.168.1.100", "50000", "User"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.server_ip, "192.168.1.100");
}

TEST(CommandLineParser, HandlesHostnames) {
    ArgvHelper args({"r-type_client", "localhost", "50000", "User"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.server_ip, "localhost");
}

TEST(CommandLineParser, HandlesSpecialCharactersInUsername) {
    ArgvHelper args({"r-type_client", "127.0.0.1", "50000", "User_123-XYZ"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.username, "User_123-XYZ");
}
