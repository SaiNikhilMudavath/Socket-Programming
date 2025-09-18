import matplotlib.pyplot as plt
rr=[];fifo=[]
with open('means_rr.txt','r') as file:
    rr=[float(line.strip()) for line in file]
with open('means_fifo.txt','r') as file:
    fifo=[float(line.strip()) for line in file]

x=[1,4,8,12,16,20,24,28,32]
plt.plot(x,rr,color='r',marker='o',label="Round Robin")
plt.plot(x,fifo,color='b',marker='s',label="FIFO")

plt.xticks(x)
# plt.title(graph_title)
plt.xlabel('number of clients')
plt.ylabel('Average completion time for a client in milliseconds')
plt.legend()
plt.grid()
plt.savefig('plot.png')
plt.close()