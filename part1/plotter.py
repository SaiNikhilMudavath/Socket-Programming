import matplotlib.pyplot as plt
import numpy as np
vals=[]
means=[]
errors=[]
i=1
with open('times.txt','r') as file:
    for line in file:
        if i%10==0:
            vals.append(int(line))
            print(vals)
            mean=np.mean(vals)
            std_dev=np.std(vals,ddof=1)
            std_error=std_dev/np.sqrt(10)
            margin_of_error=1.96*std_error

            means.append(mean)
            errors.append(margin_of_error)
            vals=[]
            i+=1
        else: 
            vals.append(int(line))
            i+=1

x=[1,2,3,4,5,6,7,8,9,10]
plt.errorbar(x,means,yerr=errors, fmt='o', capsize=5, capthick=2, elinewidth=1.5, color='b',label='Confidence intervals')

plt.plot(x, means, color='g', marker='o', label='Mean Trend')
print(len(x),len(vals))
# plt.plot(x,vals)
plt.xticks(x)
plt.xlabel('p')
plt.ylabel('Average completion time in milliseconds')
plt.title('Average Completion time vs p with confindence intervals')
plt.grid(True)
plt.legend()
plt.savefig('plot.png')
plt.close()