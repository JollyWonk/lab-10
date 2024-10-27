from collections import Counter
import re

with open('text.txt', 'r', encoding='utf-8') as file:
    text = file.read()

words = re.findall(r'\b\w+\b', text.lower())

word_count = Counter(words)

with open('freq.txt', 'w', encoding='utf-8') as file:
    for word, count in word_count.items():
        file.write(f'{word}={count}\n')

print("Частоти слів успішно записані у файл freq.txt.")
