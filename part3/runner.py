import subprocess
import json
import matplotlib.pyplot as plt
import sys
import time


def run_bash(bash_script):
    if bash_script=='./run_aloha.sh': filename='means_aloha.txt'
    elif bash_script=='./run_beb.sh': filename='means_beb.txt'
    elif bash_script=='./run_sense.sh': filename='means_sense.txt'

    with open(filename,'w') as file:
        pass

    for n_clients in range(0,33,4):
    
        with open('config.json','r') as file:
            data=json.load(file)
        
        if(n_clients)==0: data["num_clients"]=1
        else: data["num_clients"]=n_clients

        with open('config.json','w') as file:
            json.dump(data,file,indent=4)
        
        for i in range(5):
            process = subprocess.Popen(["bash", bash_script], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            process.wait()
            # stdout,stderr=process.communicate()
            # print(stdout)
            # print(stderr)
        
        with open('times.txt','r') as file:
            numbers=[int(line.strip()) for line in file]
            mean=sum(numbers)/len(numbers)
            with open(filename,'a') as file2:
                file2.write(str(mean)+'\n')
            print(f'for {data["num_clients"]} clients average completion time per client in milliseconds is',mean)

        with open('times.txt','w') as file:
            pass

protocol=sys.argv[1]

scripts=['./run_aloha.sh','./run_beb.sh','./run_sense.sh']

if protocol=="all":
    vals=[]
    for i in scripts:
        run_bash(i)
    with open('means_aloha.txt','r') as file:
            numbers=[float(line.strip()) for line in file]
            vals.append(numbers[len(numbers)-1])
    with open('means_beb.txt','r') as file:
            numbers=[float(line.strip()) for line in file]
            vals.append(numbers[len(numbers)-1])
    with open('means_sense.txt','r') as file:
            numbers=[float(line.strip()) for line in file]
            vals.append(numbers[len(numbers)-1])
    print(vals)

elif protocol=="aloha":
    run_bash('./run_aloha.sh')
elif protocol=="beb":
    run_bash('./run_beb.sh')
elif protocol=='sense':
    run_bash('./run_sense.sh')



