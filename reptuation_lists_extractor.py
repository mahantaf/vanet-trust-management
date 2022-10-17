from posixpath import split
import sys
fileName = sys.argv[1]

directory = '/'.join(fileName.split('/')[:-1]) + "/"

with open(fileName, 'r') as f:
#	f_arr = list(range(8))
#	packetForwarding = open(directory + "packetForwardingStatistics.csv", "w")
	reputationStats = open(directory + "reputationStats.csv", "w")
#	for i in  range(0, 8):
#		f_arr[i] = open(directory + str(i)+".csv", "w")
	lines = f.readlines()
	numRecvd = 0 
	numForwarded = 0
	first = 0
	prev_line_reputations = ""
	for line in lines:
		line = line.strip()
		splitLine = line.split(',')
		if "Reputations" in splitLine[0]:
			time = splitLine[1]
			if first == 0:
				prev_line_reputations = time + ", "
				for i in range(len(splitLine)-2):
					prev_line_reputations += (splitLine[i+2]) + ", "
				first = 1
			else:
				newLine = time + ", "
				# for i in range(len(splitLine)-2):
				# 	newLine += splitLine[i+2].split('[')[1].split(']')[0] + ", "
				# reputationStats.write(newLine + "\n")
				reputationStats.write(prev_line_reputations + "\n")
				first = 0
