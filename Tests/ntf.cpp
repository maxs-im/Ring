//
// Tests for notification .hpp/.cpp
//

#include "../utils/notification.hpp"
#include "gtest/gtest.h"

//namespace {
    TEST(Notification, Empty)
    {
        Notification ntf;

        EXPECT_TRUE(ntf.get_author().empty());
        EXPECT_TRUE(ntf.get_command().empty());
        EXPECT_TRUE(ntf.get_message().empty());
    }
    TEST(Notification, Getters)
    {
        auto a = "author", c = "command", m = "message";
        Notification ntf(a, c, m);

        EXPECT_EQ(a, ntf.get_author());
        EXPECT_EQ(c, ntf.get_command());
        EXPECT_EQ(m, ntf.get_message());
    }
    TEST(Notification, Construct2)
    {
        auto a = "author";
        Notification ntf(a, "@command message");

        EXPECT_EQ(a, ntf.get_author());
        EXPECT_EQ("command", ntf.get_command());
        EXPECT_EQ("message", ntf.get_message());
    }
    TEST(Notification, Constants)
    {
        Notification ntf(NTFCommand::SYSTEM_LOGIN, NTFCommand::KICK, "");

        EXPECT_TRUE(ntf.get_author().empty());
        EXPECT_EQ("kick", ntf.get_command());
    }


    struct NotificationTest : testing::Test {
        const char *a = "author",
            *c = "command",
            *m = "message";
        Notification ntf;
        NotificationTest() : ntf{a, c, m} {}
        NotificationTest(const std::string& _a,
                const std::string& _c, const std::string& _m)
                : ntf(_a, _c, _m)
        {}
        NotificationTest(const std::string& _a,
                     const std::string& _t)
                     : ntf(_a, _t)
        {}

        std::string generate_text(const std::string& _c, const std::string& _m) {
            return "@" + _c + " " + _m;
        }
    };

    TEST_F(NotificationTest, TestGenFn)
    {
        Notification ntf2(ntf.get_author(),
                generate_text(ntf.get_command(), ntf.get_message()));

        EXPECT_EQ(ntf.get_author(), ntf2.get_author());
        EXPECT_EQ(ntf.get_command(), ntf2.get_command());
        EXPECT_EQ(ntf.get_message(), ntf2.get_message());
    }
    TEST_F(NotificationTest, EncodeDefault)
    {
        EXPECT_EQ(ntf.encode(),
                "author @command message\n");
    }
    TEST_F(NotificationTest, DecodeDefault)
    {
        EXPECT_EQ(NTFCommand::decode_notification(ntf),
                "author: @command message");
    }
    TEST_F(NotificationTest, Notification2_3Code)
    {
        Notification ntf2("author", "@command message");

        EXPECT_EQ(ntf.encode(), ntf2.encode());
        EXPECT_EQ(NTFCommand::decode_notification(ntf),
                  NTFCommand::decode_notification(ntf2));

        EXPECT_EQ(ntf.get_author(), ntf2.get_author());
        EXPECT_EQ(ntf.get_command(), ntf2.get_command());
        EXPECT_EQ(ntf.get_message(), ntf2.get_message());
    }
    TEST_F(NotificationTest, UpdateThrow)
    {
        EXPECT_ANY_THROW(ntf.update("some text"));
    }


    struct notification_state
    {
        std::string author, command, message;
        std::string encoded, decoded;

    };

    struct WithParamsNtf
            : NotificationTest, testing::WithParamInterface<notification_state>
    {
        WithParamsNtf()
            : NotificationTest(GetParam().author, GetParam().command,
                    GetParam().message)
        {}
    };

    TEST_P(WithParamsNtf, ImportantDifferentEquals)
    {
        Notification ntf2(GetParam().author,
                generate_text(GetParam().command, GetParam().message));

        EXPECT_EQ(ntf.get_author(), ntf2.get_author());
        EXPECT_EQ(ntf.get_command(), ntf2.get_command());
        EXPECT_EQ(ntf.get_message(), ntf2.get_message());
    }
    TEST_P(WithParamsNtf, Coding)
    {
        auto params = GetParam();
        EXPECT_EQ(ntf.get_author(), params.author);
        EXPECT_EQ(ntf.get_command(), params.command);
        EXPECT_EQ(ntf.get_message(), params.message);
        EXPECT_EQ(ntf.encode(), params.encoded);
        EXPECT_EQ(NTFCommand::decode_notification(ntf), params.decoded);
    }
    TEST_P(WithParamsNtf, Update)
    {
        Notification ntf_copy = ntf;

        EXPECT_EQ(ntf.get_author(), ntf_copy.get_author());
        EXPECT_EQ(ntf.get_command(), ntf_copy.get_command());
        EXPECT_EQ(ntf.get_message(), ntf_copy.get_message());

        auto encoded = GetParam().encoded;

        // remove DELIMITER
        auto clean_encode(encoded);
        clean_encode.pop_back();

        ntf_copy.update(clean_encode);

        EXPECT_EQ(ntf.get_author(), ntf_copy.get_author());
        EXPECT_EQ(ntf.get_command(), ntf_copy.get_command());
        EXPECT_EQ(ntf.get_message(), ntf_copy.get_message());
    }


    INSTANTIATE_TEST_CASE_P(EmptyCM, WithParamsNtf,
            testing::Values(
                    notification_state{"a", "", "",
                                       "a @ \n", "a: "},
                    notification_state{"a", "", "m",
                                       "a @ m\n", "a: m"},
                    notification_state{"a", "c", "m",
                                       "a @c m\n", "a: @c m"},
                    notification_state{"a", NTFCommand::KICK, "",
                                       "a @kick \n", "a:"}
            ));
    INSTANTIATE_TEST_CASE_P(EmptyAuthorNotCommand, WithParamsNtf,
            testing::Values(
                    notification_state{NTFCommand::SYSTEM_LOGIN, "", "",
                                       " @ \n", "\t-----><-----"},
                    notification_state{NTFCommand::SYSTEM_LOGIN, "", "m",
                                       " @ m\n", "\t----->m<-----"},
                    notification_state{NTFCommand::SYSTEM_LOGIN, "c", "m",
                                       " @c m\n", "\t----->m<-----"},
                    notification_state{NTFCommand::SYSTEM_LOGIN, "c", "",
                                       " @c \n", "\t-----><-----"}
            ));
    INSTANTIATE_TEST_CASE_P(EmptyAuthorCommand, WithParamsNtf,
            testing::Values(
                    notification_state{NTFCommand::SYSTEM_LOGIN, NTFCommand::JOIN, "m",
                                       " @join m\n", "\tGreet m in chat"},
                    notification_state{NTFCommand::SYSTEM_LOGIN, NTFCommand::LEAVE, "m",
                                       " @leave m\n", "\tm left chat"},
                    notification_state{NTFCommand::SYSTEM_LOGIN, NTFCommand::KICK, "m",
                                       " @kick m\n", "\tDISCONNECTED: m"},
                    notification_state{NTFCommand::SYSTEM_LOGIN, NTFCommand::PASSWORD, "m",
                                       " @password m\n", "\t----->m<-----"}
            ));
//}

