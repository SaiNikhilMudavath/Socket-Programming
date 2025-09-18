import subprocess
import json

command=['bash','run.sh']
with open('means.txt','w') as file:
    pass

for ncli in range(0,33,4):
    
    with open('config.json','r') as file:
        data=json.load(file)
    
    if(ncli)==0: data["num_clients"]=1
    else: data["num_clients"]=ncli

    with open('config.json','w') as file:
        json.dump(data,file,indent=4)
    
    for i in range(10):
        result = subprocess.run(command, capture_output=True, text=True)
        # stdout,stderr=process.communicate()
        # print(stdout)
        
    
    with open('times.txt','r') as file:
        numbers=[int(line.strip()) for line in file]
        mean=sum(numbers)/len(numbers)
        with open('means.txt','a') as file:
            file.write(str(mean)+'\n')
        print(f'for {data["num_clients"]} clients average completion time per client milliseconds',mean)
    with open('times.txt','w') as file:
        pass



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
