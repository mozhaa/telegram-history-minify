#include <cstdio>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

const std::size_t buffer_size = 1 << 25;
const int min_time_gap = 3600;
const int max_no_date_cnt = 100;

class progress_bar {
    unsigned int _count;
    unsigned int _limit;

    void _update() {
        if (_count == 1 || _count == _limit || (_limit > 100 && _count % (_limit / 100) == 0) ||
            (_limit <= 100 && _count % std::max(1u, _limit / 10) == 0)) {
            _print();
        }
    }

    void _print() {
        int percentage = (_count * 100) / _limit;
        fprintf(stderr, "\rProgress: %d%%", percentage);
    }

public:
    progress_bar(unsigned int limit) : _count(0), _limit(limit) {}
    ~progress_bar() { fprintf(stderr, "\n"); }

    void increment() {
        _count++;
        _update();
    }
};

int main(int argc, char *argv[]) {
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

    std::unordered_map<int, unsigned int> msg_id_to_idx;
    std::string previous_from_id = "";
    int previous_time = 0;
    int no_date_cnt = 0;
    progress_bar pb(num_messages);
    for (unsigned int i = 0; i < num_messages; ++i, pb.increment()) {
        const auto &msg = messages_array.GetArray()[i];

        if (!msg.IsObject()) {
            continue;
        }

        if (msg.HasMember("id") && msg["id"].IsInt()) {
            msg_id_to_idx[msg["id"].GetInt()] = i;
        }

        if (msg.HasMember("type") && msg["type"].IsString() &&
            std::string_view(msg["type"].GetString()) == "service") {
            continue;
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

        if (msg.HasMember("photo")) {
            text = "*photo* " + text;
        }

        if (text.size() == 0 && msg.HasMember("media_type")) {
            std::string media_type = msg["media_type"].GetString();
            if (media_type == "sticker") {
                if (msg.HasMember("sticker_emoji") && msg["sticker_emoji"].IsString()) {
                    text = std::string("*sticker ") + msg["sticker_emoji"].GetString() + "*";
                } else {
                    throw std::runtime_error(
                        "message with media_type=sticker has not 'sticker_emoji' member");
                }
            } else if (media_type == "video_file") {
                text = "*video*";
            } else if (media_type == "animation") {
                text = "*gif*";
            }
        }

        if (text.size() == 0) {
            continue;
        }

        std::string sender;
        if (msg.HasMember("from") && msg["from"].IsString()) {
            sender = msg["from"].GetString();
        } else {
            sender = "unknown";
        }

        std::string from_id;
        if (msg.HasMember("from_id") && msg["from_id"].IsString()) {
            from_id = msg["from_id"].GetString();
        }

        std::string reply_to = "";
        bool is_reply = false;
        if (msg.HasMember("reply_to_message_id") && msg["reply_to_message_id"].IsInt()) {
            int reply_id = msg["reply_to_message_id"].GetInt();
            const auto it = msg_id_to_idx.find(reply_id);
            if (it != msg_id_to_idx.end()) {
                const auto &old_msg = messages_array.GetArray()[it->second];
                if (old_msg.HasMember("from") && old_msg["from"].IsString()) {
                    reply_to = old_msg["from"].GetString();
                    is_reply = true;
                }
            }
        }

        bool show_date = false;
        if (msg.HasMember("date_unixtime") && msg["date_unixtime"].IsString()) {
            int time = std::stoi(msg["date_unixtime"].GetString());
            if (time - previous_time > min_time_gap) {
                show_date = true;
            }
            previous_time = time;
        } else {
            throw std::runtime_error("message has no 'date_unixtime' member");
        }

        if (!is_reply && from_id.size() > 0 && previous_from_id.size() > 0 &&
            previous_from_id == from_id) {
            printf("%s\n", text.c_str());
        } else {
            if (show_date || no_date_cnt > max_no_date_cnt) {
                no_date_cnt = 0;
                if (msg.HasMember("date") && msg["date"].IsString()) {
                    printf("### DATE: %s\n", msg["date"].GetString());
                } else {
                    throw std::runtime_error("message has no 'date' member");
                }
            } else {
                printf("###\n");
                ++no_date_cnt;
            }

            if (is_reply) {
                printf("[%s](-> %s): %s\n", sender.c_str(), reply_to.c_str(), text.c_str());
            } else {
                printf("[%s]: %s\n", sender.c_str(), text.c_str());
            }
        }

        previous_from_id = std::move(from_id);
    }

    return 0;
}
