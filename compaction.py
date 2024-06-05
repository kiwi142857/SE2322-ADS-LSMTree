import matplotlib.pyplot as plt

# Read the throughput data
with open('throughput.txt', 'r') as file:
    data = [int(line) for line in file]

# Plot the data
plt.plot(data)
plt.xlabel('Time (seconds)')
plt.ylabel('Throughput (operations per 100 ms)')
plt.show()