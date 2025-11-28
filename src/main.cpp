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
    
    fprintf(stderr, "Waiting for input from stdin...\n");
    rapidjson::FileReadStream is(stdin, buffer.get(), buffer_size);
    fprintf(stderr, "Parsing JSON...\n");

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

    fprintf(stderr, "Processing %u messages...\n", num_messages);

    rapidjson::SizeType processed_count = 0;
    for (rapidjson::SizeType i = 0; i < num_messages; ++i) {
        const auto &msg = messages_array.GetArray()[i];

        ++processed_count;
        if (processed_count == 1 || processed_count == num_messages ||
            (num_messages > 100 && processed_count % (num_messages / 100) == 0) ||
            (num_messages <= 100 &&
             processed_count % std::max<rapidjson::SizeType>(1, num_messages / 10) == 0)) {
            int percentage = (processed_count * 100) / num_messages;
            fprintf(stderr, "\rProgress: %d%%", percentage);
        }

        if (!msg.IsObject()) {
            continue;
        }

        if (msg.HasMember("type") && msg["type"].IsString() &&
            std::string_view(msg["type"].GetString()) == "service") {
            continue;
        }

        std::string sender;
        if (msg.HasMember("from") && msg["from"].IsString()) {
            sender = msg["from"].GetString();
        } else {
            sender = "unknown";
        }

        std::string text;
        if (msg.HasMember("text")) {
            if (msg["text"].IsString()) {
                text = msg["text"].GetString();
            } else if (msg["text"].IsArray()) {
                std::stringstream ss;
                for (const auto &text_elem : msg["text"].GetArray()) {
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

    fprintf(stderr, "\n");
    return 0;
}
