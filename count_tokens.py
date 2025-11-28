from transformers import AutoTokenizer
import sys

def count_tokens(file_path, tokenizer_name="bert-base-uncased"):
    try:
        # Initialize tokenizer
        tokenizer = AutoTokenizer.from_pretrained(tokenizer_name)
        
        # Read file
        with open(file_path, 'r', encoding='utf-8') as file:
            text = file.read()
        
        # Tokenize text
        tokens = tokenizer.tokenize(text)
        
        # Print results
        print(f"File: {file_path}")
        print(f"Tokenizer: {tokenizer_name}")
        print(f"Number of tokens: {len(tokens)}")
        
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
    except Exception as e:
        print(f"An error occurred: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage: python token_counter.py <file_path> [tokenizer_name]")
        print("Default tokenizer: bert-base-uncased")
        sys.exit(1)
    
    file_path = sys.argv[1]
    tokenizer_name = sys.argv[2] if len(sys.argv) > 2 else "bert-base-uncased"
    
    count_tokens(file_path, tokenizer_name)
