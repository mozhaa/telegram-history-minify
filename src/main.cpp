#include <cstdio>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

int main(int argc, char *argv[]) {
    const std::size_t buffer_size = 1 << 25;
    std::unique_ptr<char> buffer(new char[buffer_size]);
    rapidjson::FileReadStream is(stdin, buffer.get(), buffer_size);

    rapidjson::Document d;
    rapidjson::ParseResult parse_ok = d.ParseStream(is);

    if (!parse_ok) {
        fprintf(stderr, "Error: JSON parsing failed.\n");
        fprintf(stderr, "Error code: %d\n", parse_ok.Code());
        fprintf(stderr, "Error offset: %zu\n", parse_ok.Offset());

        const char *parse_error = rapidjson::GetParseError_En(parse_ok.Code());
        fprintf(stderr, "Error message: %s\n", parse_error);
        return 1;
    }

    if (!d.HasMember("messages") || !d["messages"].IsArray()) {
        fprintf(stderr, "Error: JSON does not contain a 'messages' array.\n");
        return 1;
    }

    const rapidjson::Value &messages_array = d["messages"];
    const rapidjson::SizeType num_messages = messages_array.Size();

    for (const auto &message_value : messages_array.GetArray()) {
        if (!message_value.IsObject()) {
            continue;
        }

        if (message_value.HasMember("type") && message_value["type"].IsString() &&
            std::string_view(message_value["type"].GetString()) == "service") {
            continue;
        }

        std::string sender;
        if (message_value.HasMember("from") && message_value["from"].IsString()) {
            sender = message_value["from"].GetString();
        } else {
            sender = "unknown";
        }

        std::string text;
        if (message_value.HasMember("text")) {
            if (message_value["text"].IsString()) {
                text = message_value["text"].GetString();
            } else if (message_value["text"].IsArray()) {
                std::stringstream ss;
                for (const auto &text_elem : message_value["text"].GetArray()) {
                    if (text_elem.IsObject() && text_elem.HasMember("text") &&
                        text_elem["text"].IsString()) {
                        ss << text_elem["text"].GetString();
                    } else if (text_elem.IsString()) {
                        ss << text_elem.GetString();
                    }
                }
                text = ss.str();
            }
        }

        printf("[%s]: %s\n", sender.c_str(), text.c_str());
    }

    return 0;
}
