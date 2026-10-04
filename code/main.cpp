#include <print>
#include <string>
#include <string_view>

#if TESTS_ENABLED
#include <catch2/catch_all.hpp>

struct TestRunListener : public Catch::EventListenerBase
{
    using EventListenerBase::EventListenerBase;

    static constexpr std::string_view COLOR_RESET = "\033[0m";
    static constexpr std::string_view COLOR_RED = "\033[0;31m";
    static constexpr std::string_view COLOR_GREEN = "\033[0;32m";
    static constexpr std::string_view COLOR_YELLOW = "\033[0;33m";
    static constexpr std::string_view COLOR_GRAY = "\033[0;90m";
    static constexpr std::string_view COLOR_CYAN   = "\033[0;36m";

    /*void sectionStarting(const Catch::SectionInfo& sectionInfo) override
    {
        std::println("{}[ RUNNING ]{} {}", COLOR_GREEN, COLOR_RESET, sectionInfo.name);
    }*/

    /*void assertionEnded(const Catch::AssertionStats& assertionStats) override
    {
        const Catch::AssertionResult& result = assertionStats.assertionResult;
        if (!result.isOk())
        {
            const Catch::SourceLineInfo& lineInfo = result.getSourceInfo();
            std::println("    {}[ FAILURE ]{} {}:{}", COLOR_RED, COLOR_RESET, lineInfo.file, lineInfo.line);

            if (result.hasExpression())
            {
                std::println("      {}Expression:{} {}", COLOR_GRAY, COLOR_RESET, result.getExpression());
            }
            if (result.hasExpandedExpression())
            {
                std::println("      {}Expanded:  {} {}", COLOR_GRAY, COLOR_RESET, result.getExpandedExpression());
            }
            if (result.hasMessage())
            {
                std::println("      {}Message:   {} {}", COLOR_YELLOW, COLOR_RESET, result.getMessage());
            }
        }
    }*/

    void sectionEnded(const Catch::SectionStats& sectionStats) override
    {
        if (sectionStats.sectionInfo.name == m_currentTestCase)
        {
            // Don't print section name twice
            return;
        }

        if (sectionStats.assertions.skipped)
        {
            std::println("{}[ SKIP ]{} {}", COLOR_YELLOW, COLOR_RESET, sectionStats.sectionInfo.name);
        }
        else if (sectionStats.assertions.failedButOk)
        {
            std::println("{}[ WARN ]{} {}", COLOR_YELLOW, COLOR_RESET, sectionStats.sectionInfo.name);
        }
        else if (!sectionStats.assertions.allOk())
        {
            std::println("{}[ FAILED ]{} {}", COLOR_RED, COLOR_RESET, sectionStats.sectionInfo.name);
        }
        else if (sectionStats.assertions.allOk())
        {
            std::println("{}[ OK ]{} {} {}({:.03f} sec){}",
                COLOR_GREEN,
                COLOR_RESET,
                sectionStats.sectionInfo.name,
                COLOR_GRAY,
                sectionStats.durationInSeconds,
                COLOR_RESET);
        }
    }

    void testCaseStarting(const Catch::TestCaseInfo& testCaseInfo) override
    {
        m_currentTestCase = testCaseInfo.name;
        std::println("{}[ RUNNING ]{} {}", COLOR_GREEN, COLOR_RESET, testCaseInfo.name);
    }

    void testCaseEnded(const Catch::TestCaseStats& testCaseStats) override
    {
        if (!testCaseStats.totals.testCases.allPassed())
        {
            Failed_Test failedTest =
            {
                .m_name = testCaseStats.testInfo->name,
                .m_file = testCaseStats.testInfo->lineInfo.file,
                .m_line = testCaseStats.testInfo->lineInfo.line
            };
            m_failedTests.push_back(std::move(failedTest));
        }
    }

    void testRunEnded(const Catch::TestRunStats& stats) override
    {
        if (!m_failedTests.empty())
        {
            std::println(
                "{} ================================ FAILED TESTS ================================= {}",
                COLOR_RED,
                COLOR_RESET);

            std::println(
                "{}[ SUMMARY ]{} Failed {} of {} test cases:",
                COLOR_RED,
                COLOR_RESET,
                m_failedTests.size(),
                stats.totals.testCases.total());

            for (const Failed_Test& test : m_failedTests)
            {
                std::println(
                    "  \"{}\" {}({}:{}){}",
                    test.m_name,
                    COLOR_GRAY,
                    test.m_file,
                    test.m_line,
                    COLOR_RESET);
            }
        }
    }

private:
    struct Failed_Test
    {
        std::string m_name;
        std::string_view m_file;
        size_t m_line;
    };

    std::string m_currentTestCase;
    std::vector<Failed_Test> m_failedTests;
};

CATCH_REGISTER_LISTENER(TestRunListener)

int main(int argc, const char* argv[])
{
    return Catch::Session().run(argc, argv);
}
#else
int main()
{
    std::println("Hello, world!");

    return 0;
}
#endif // TESTS_ENABLED
