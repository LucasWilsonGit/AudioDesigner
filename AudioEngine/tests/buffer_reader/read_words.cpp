#include "AudioEngine/dsp.hpp"
#include <sstream>
#include "make_test_buffer.hpp"

template <class CharT, size_t Alignment>
class buffer_reader_tester : public AudioEngine::buffer_reader<CharT, Alignment> {
public:
    using AudioEngine::buffer_reader<CharT, Alignment>::buffer_reader;

    CharT& curr_char() const noexcept { return AudioEngine::buffer_reader<CharT, Alignment>::curr_char(); }
};

class test_failure_err : public std::runtime_error {
    std::string s;
public:
    test_failure_err(std::string const& ins) : std::runtime_error(ins), s(ins) {}

    [[nodiscard]] char const* what() const noexcept { return s.c_str(); }
};

template <class ReaderType>
void test_reader_type(ReaderType& reader, size_t count, std::string const& result) {
    std::vector<std::string> words(count);

    for (size_t i = 0; i < count; i++) {
        if (!(reader >> words[i])) {
            throw test_failure_err(format("Failed to parse {} words from the reader", count));
        }
    }

    std::ostringstream oss;
    bool c = false;
    for (std::string const& word : words) {
        if (c)
            oss << " ";
        c = true;
        oss << word;
    }

    std::string out = oss.str();
    std::cout << out << "\n";

    if (out != result) {
        throw test_failure_err( format("Test failed checking {} words. Got `{}` expected `{}`", count, out, result) );
    }
}

int main() {
    char const* msg = "Hello, world! This is a test message\nThe quick brown fox, jumped over the lazy dog.\r\n\r\nLorem ipsum!";
    auto [buff, len] = make_test_buffer<char>(msg);

    buffer_reader_tester<char, 16> reader(std::move(buff), len);

    test_reader_type(reader, 7, "Hello, world! This is a test message");
    test_reader_type(reader, 9, "The quick brown fox, jumped over the lazy dog.");

    try {
        test_reader_type(reader, 2, "Lorem ipsum! THIS TEST MUST FAIL");
        return -1;
    }
    catch (test_failure_err const& e) {
    }
}