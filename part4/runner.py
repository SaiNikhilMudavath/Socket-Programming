import subprocess
import json
import matplotlib.pyplot as plt
import sys
import time


def run_bash(bash_script):
    if bash_script=='./run_rr.sh': filename='means_rr.txt'
    elif bash_script=='./run_fifo.sh': filename='means_fifo.txt'

    with open(filename,'w') as file:
        pass
    
    with open('times.txt','w') as file:
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
            stdout,stderr=process.communicate()
        
        with open('times.txt','r') as file:
            numbers=[int(line.strip()) for line in file]
            mean=sum(numbers)/len(numbers)
            with open(filename,'a') as file2:
                file2.write(str(mean)+'\n')
            print(f'for {data["num_clients"]} clients average completion time per client in milliseconds is',mean)

        with open('times.txt','w') as file:
            pass


protocol=sys.argv[1]

scripts=['./run_rr.sh','./run_fifo.sh']

if protocol=="all":
    for i in scripts:
        run_bash(i)
    vals=[]
    with open('means_rr.txt','r') as file:
            numbers=[int(line.strip()) for line in file]
            vals.append(numbers[len(numbers)-1])
    with open('means_fifo.txt','r') as file:
            numbers=[int(line.strip()) for line in file]
            vals.append(numbers[len(numbers)-1])

elif protocol=="rr":
    run_bash('./run_rr.sh')
elif protocol=="fifo":
    run_bash('./run_fifo.sh')
