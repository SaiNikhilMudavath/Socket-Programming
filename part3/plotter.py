import matplotlib.pyplot as plt
aloha=[];beb=[];sense=[]
with open('means_aloha.txt','r') as file:
    aloha=[float(line.strip())/1000 for line in file]
with open('means_beb.txt','r') as file:
    beb=[float(line.strip())/1000 for line in file]
with open('means_sense.txt','r') as file:
    sense=[float(line.strip())/1000 for line in file]

x=[1,4,8,12,16,20,24,28,32]
plt.plot(x,aloha,color='r',marker='o',label="Slotted Aloha")
plt.plot(x,beb,color='b',marker='s',label="BEB")
plt.plot(x,sense,color='g',marker='^',label='SBEB')
plt.xticks(x)
# plt.title(graph_title)
plt.xlabel('number of clients')
plt.ylabel('Average completion time for a client in seconds')
plt.legend()
plt.grid()
plt.savefig('plot.png')
plt.close()