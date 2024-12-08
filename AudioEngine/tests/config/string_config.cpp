#include "AudioEngine/config.hpp"

int main() {
    std::string input = "DbgAudioOutput      BinaryPCM.dat";
    std::unique_ptr<char> buf = std::unique_ptr<char>(new char[input.size()]);
    memcpy(buf.get(), input.data(), input.size());

    AudioEngine::dsp_cfg_parser<char, std::allocator<char>,
            AudioEngine::dsp_cfg_bool_parser_impl,
            AudioEngine::dsp_cfg_monitor_input_parser_impl,
            AudioEngine::dsp_cfg_int64_parser_impl,
            AudioEngine::dsp_cfg_string_parser_impl
        > parser(std::move(buf), input.size());

    auto config = parser.get_config();

    auto val = config.get<std::string>("DbgAudioOutput");
    if (val != "BinaryPCM.dat") {
        std::cout << format("Read config file failed: got {}\n", val);
        return -1;
    }

    std::cout << format("Read config file: {}\n", val);

    return 0;
}