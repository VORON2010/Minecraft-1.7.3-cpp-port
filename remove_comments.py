import os
import re

def remove_comments(text):
    # Умная регулярка, которая игнорирует комментарии внутри строк
    pattern = r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"'
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return ""
        else:
            return s
    return re.sub(pattern, replacer, text, flags=re.DOTALL | re.MULTILINE)

def process_directory(directory):
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.cpp', '.h', '.hpp', '.c')):
                filepath = os.path.join(root, file)
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                
                new_content = remove_comments(content)
                
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(new_content)
                print(f"Removed comments from {filepath}")

if __name__ == "__main__":
    process_directory("src")
