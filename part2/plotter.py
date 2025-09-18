import matplotlib.pyplot as plt
vals=[]
with open('means.txt','r') as file:
    for line in file:
        vals.append(float(line.strip()))

x=[1,4,8,12,16,20,24,28,32]
print(len(x),len(vals))
plt.plot(x,vals)
plt.xticks(x)
plt.xlabel('number of clients')
plt.ylabel('Average completion time for a client in milliseconds')
plt.grid()
plt.legend()
plt.title('Average Completion time vs no of clients')
plt.savefig('plot.png')
plt.show()