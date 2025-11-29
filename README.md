# Telegram History Minify

A C++ CLI tool to convert Telegram chat exports (JSON) into clean text for LLM consumption.

## Features

- **Smart Message Formatting**: Converts Telegram JSON exports to human-readable text
- **Media Type Support**: Handles text messages, photos, videos, stickers, and GIFs
- **Intelligent Grouping**: Groups consecutive messages from the same sender
- **Time-based Separation**: Shows dates when there are significant time gaps (>1 hour)
- **Reply Handling**: Properly formats message replies with attribution
- **Progress Tracking**: Shows progress bar for large chat exports
- **Service Message Filtering**: Automatically skips service messages

## Building

### Prerequisites

- C++17 compatible compiler
- CMake 3.15 or higher
- Conan package manager

### Build Instructions

1. Install dependencies using Conan:
   ```bash
   conan install . --build=missing --output-folder=build
   ```

2. Build the project:
   ```bash
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
   cmake --build .
   ```

The executable will be created as `tgminify` (`tgminify.exe` for Windows) in the build directory.

## Usage

The tool reads Telegram chat export JSON from stdin and outputs the formatted text to stdout.

### Basic Usage

```bash
# Process a Telegram export file
cat result.json | ./tgminify >chat.txt
```

### Input Format

The tool expects a Telegram chat export JSON file with a `messages` array containing message objects like:

```json
{
  "messages": [
    {
      "id": 1,
      "type": "message",
      "date": "2023-01-01T12:00:00",
      "date_unixtime": "1672574400",
      "from": "John Doe",
      "from_id": "user123",
      "text": "Hello, world!"
    }
  ]
}
```

### Output Format

The tool produces a clean, readable format:

```
### DATE: 2023-01-01T12:00:00
[John Doe]: Hello, world!
###
[Jane Smith]: Hi there!
###
[John Doe]: How are you?
```

## Configuration

The tool uses several hardcoded constants that can be modified in the source code:

- `min_time_gap = 3600`: Minimum time gap (in seconds) to show a date separator (default: 1 hour)
- `max_no_date_cnt = 100`: Maximum consecutive messages without a date before forcing one
- `buffer_size = 1 << 25`: JSON parsing buffer size (32MB)

## Supported Message Types

- **Text messages**: Regular text content
- **Photos**: Marked as `*photo*` followed by any caption
- **Videos**: Marked as `*video*`
- **GIFs**: Marked as `*gif*`
- **Stickers**: Marked as `*sticker <emoji>*` with the sticker emoji
- **Replies**: Formatted as `[Sender](-> OriginalSender): Message`

## Dependencies

- **RapidJSON**: High-performance JSON parsing library
