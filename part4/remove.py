import os

def remove_files(k):
    for i in range(k+1):
        filename = f"output_{i}.txt"
        if os.path.exists(filename):
            os.remove(filename)
            # print(f"Removed {filename}")
        else:
            # No action taken if file is not found, no error message
            pass

# Example usage:
# remove_files(5)  # This will attempt to remove files output_0.txt to output_5.txt
remove_files(50)