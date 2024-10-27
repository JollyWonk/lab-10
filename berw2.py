import os
current_directory = os.getcwd()
files_in_directory = os.listdir(current_directory)

print("Файли в поточній папці:")
for file_name in files_in_directory:
    print(file_name)

for file_name in files_in_directory:
    if file_name.endswith('.txt'):
        os.remove(file_name)
        print(f"Файл {file_name} видалено.")

print("Всі файли з розширенням .txt видалені.")
