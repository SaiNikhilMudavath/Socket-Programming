import subprocess
import json
import matplotlib.pyplot as plt

bash_script = "./run.sh"
with open('times.txt','w') as file:
    pass

for p in range(1,11):
    with open('config.json','r') as file:
        data=json.load(file)
    data["p"]=p
    with open('config.json','w') as file:
        json.dump(data,file,indent=4)
    for i in range(10):
        process = subprocess.Popen(["bash", bash_script], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        process.wait()
        stdout,stderr=process.communicate()
        # print(stdout)

# print("Bash script output:")
# print(stdout.decode())
# print("Bash script errors:")
# print(stderr.decode())


# import subprocess
# import time

# server_executable = "./server"
# client_executable = "./client"

# with open('times.txt','w') as file:
#     pass
# for i in range(10):
#     # Start the server process
#     server_process = subprocess.Popen([server_executable], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#     print(f"Started server process {i + 1}")

#     # Give the server a moment to start
#     time.sleep(2)

#     # Start the client process
#     client_process = subprocess.Popen([client_executable], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#     print(f"Started client process {i + 1}")

#     # Optionally capture and print output from server and client
#     server_stdout, server_stderr = server_process.communicate(timeout=30)  # Adjust timeout as needed
#     client_stdout, client_stderr = client_process.communicate(timeout=30)  # Adjust timeout as needed

#     client_process.wait()

#     server_process.terminate()
