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

// =============================================================================
// Solo Mode Tests (new functionality)
// =============================================================================

TEST(CommandLineParser, ParsesUsernameOnlySoloMode) {
    ArgvHelper args({"r-type_client", "TestPlayer"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_TRUE(config.solo_mode);
    EXPECT_EQ(config.username, "TestPlayer");
    EXPECT_EQ(config.server_ip, "127.0.0.1");
    EXPECT_EQ(config.tcp_port, 50000);
    EXPECT_EQ(config.udp_port, 50000);
}

TEST(CommandLineParser, ParsesUsernameAndIPWithDefaultPort) {
    ArgvHelper args({"r-type_client", "Player1", "192.168.1.100"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_FALSE(config.solo_mode);
    EXPECT_EQ(config.username, "Player1");
    EXPECT_EQ(config.server_ip, "192.168.1.100");
    EXPECT_EQ(config.tcp_port, 50000);  // Default port
    EXPECT_EQ(config.udp_port, 50000);
}

TEST(CommandLineParser, ParsesFullArgumentsExplicitMode) {
    ArgvHelper args({"r-type_client", "TestUser", "192.168.1.1", "50000"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_FALSE(config.solo_mode);
    EXPECT_EQ(config.username, "TestUser");
    EXPECT_EQ(config.server_ip, "192.168.1.1");
    EXPECT_EQ(config.tcp_port, 50000);
    EXPECT_EQ(config.udp_port, 50000);
}

TEST(CommandLineParser, ParsesFullArgumentsWithUdpPort) {
    ArgvHelper args(
        {"r-type_client", "Player1", "127.0.0.1", "50000", "-up", "50001"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_FALSE(config.solo_mode);
    EXPECT_EQ(config.username, "Player1");
    EXPECT_EQ(config.server_ip, "127.0.0.1");
    EXPECT_EQ(config.tcp_port, 50000);
    EXPECT_EQ(config.udp_port, 50001);
}

TEST(CommandLineParser, ParsesWithLongUdpFlag) {
    ArgvHelper args(
        {"r-type_client", "User", "10.0.0.1", "12345", "--udp-port", "54321"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_FALSE(config.solo_mode);
    EXPECT_EQ(config.username, "User");
    EXPECT_EQ(config.server_ip, "10.0.0.1");
    EXPECT_EQ(config.tcp_port, 12345);
    EXPECT_EQ(config.udp_port, 54321);
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST(CommandLineParser, ThrowsOnMissingArguments) {
    ArgvHelper args({"r-type_client"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Missing required argument");
}

TEST(CommandLineParser, ThrowsOnInvalidTcpPort) {
    ArgvHelper args({"r-type_client", "User", "127.0.0.1", "70000"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid TCP-PORT");
}

TEST(CommandLineParser, ThrowsOnNonNumericTcpPort) {
    ArgvHelper args({"r-type_client", "User", "127.0.0.1", "abc"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid TCP-PORT");
}

TEST(CommandLineParser, ThrowsOnInvalidUdpPort) {
    ArgvHelper args(
        {"r-type_client", "User", "127.0.0.1", "50000", "-up", "0"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid UDP-PORT");
}

TEST(CommandLineParser, ThrowsOnEmptyUsername) {
    ArgvHelper args({"r-type_client", ""});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "USERNAME cannot be empty");
}

TEST(CommandLineParser, ThrowsOnUsernameTooLong) {
    std::string long_username(33, 'A');  // 33 characters (max is 32)
    ArgvHelper args({"r-type_client", long_username});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE),
        "USERNAME too long \\(max 32 characters\\)");
}

TEST(CommandLineParser, AcceptsMaxLengthUsername) {
    std::string max_username(32, 'A');  // Exactly 32 characters
    ArgvHelper args({"r-type_client", max_username, "127.0.0.1", "50000"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.username, max_username);
    EXPECT_EQ(config.username.length(), 32u);
}

TEST(CommandLineParser, ThrowsOnMissingUdpPortValue) {
    ArgvHelper args({"r-type_client", "User", "127.0.0.1", "50000", "-up"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE),
        "Missing value for -up/--udp-port flag");
}

TEST(CommandLineParser, ThrowsOnUnknownFlag) {
    ArgvHelper args(
        {"r-type_client", "User", "127.0.0.1", "50000", "--verbose"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_FAILURE), "Unknown argument");
}

TEST(CommandLineParser, ExitsWithSuccessOnHelpFlag) {
    ArgvHelper args({"r-type_client", "--help"});

    EXPECT_EXIT(RC::CommandLineParser::Parse(args.Argc(), args.Argv()),
        ::testing::ExitedWithCode(EXIT_SUCCESS), "Usage:");
}

// =============================================================================
// Port Boundary Tests
// =============================================================================

TEST(CommandLineParser, ValidatesPortBoundaries) {
    // Test minimum valid port
    ArgvHelper args_min({"r-type_client", "User", "127.0.0.1", "1"});
    RC::ClientConfig config_min =
        RC::CommandLineParser::Parse(args_min.Argc(), args_min.Argv());
    EXPECT_EQ(config_min.tcp_port, 1);

    // Test maximum valid port
    ArgvHelper args_max({"r-type_client", "User", "127.0.0.1", "65535"});
    RC::ClientConfig config_max =
        RC::CommandLineParser::Parse(args_max.Argc(), args_max.Argv());
    EXPECT_EQ(config_max.tcp_port, 65535);
}

// =============================================================================
// IP Address/Hostname Tests
// =============================================================================

TEST(CommandLineParser, HandlesIPv4Addresses) {
    ArgvHelper args({"r-type_client", "User", "192.168.1.100", "50000"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.server_ip, "192.168.1.100");
}

TEST(CommandLineParser, HandlesHostnames) {
    ArgvHelper args({"r-type_client", "User", "localhost", "50000"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.server_ip, "localhost");
}

TEST(CommandLineParser, HandlesSpecialCharactersInUsername) {
    ArgvHelper args({"r-type_client", "User_123-XYZ", "127.0.0.1", "50000"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_EQ(config.username, "User_123-XYZ");
}

// =============================================================================
// Solo Mode Edge Cases
// =============================================================================

TEST(CommandLineParser, SoloModeWithUdpPort) {
    // Solo mode but with UDP port override
    ArgvHelper args({"r-type_client", "Player1", "-up", "50001"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_TRUE(config.solo_mode);
    EXPECT_EQ(config.username, "Player1");
    EXPECT_EQ(config.server_ip, "127.0.0.1");
    EXPECT_EQ(config.tcp_port, 50000);
    EXPECT_EQ(config.udp_port, 50001);
}

TEST(CommandLineParser, TwoArgsWithUdpPort) {
    // IP but no port, with UDP port flag
    ArgvHelper args(
        {"r-type_client", "Player1", "192.168.1.1", "--udp-port", "50002"});

    RC::ClientConfig config =
        RC::CommandLineParser::Parse(args.Argc(), args.Argv());

    EXPECT_FALSE(config.solo_mode);
    EXPECT_EQ(config.username, "Player1");
    EXPECT_EQ(config.server_ip, "192.168.1.1");
    EXPECT_EQ(config.tcp_port, 50000);
    EXPECT_EQ(config.udp_port, 50002);
}
