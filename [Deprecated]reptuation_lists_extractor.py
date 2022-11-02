from posixpath import split
import sys

if __name__ == "__main__":
	fileName = sys.argv[1]

	directory = '/'.join(fileName.split('/')[:-1]) + "/"

def extract_reputation_list(directory, fileName):
	reputation_list = list()
	with open(fileName, 'r') as f:
		reputationStats = open(directory + "reputationStats.csv", "w")
		lines = f.readlines()
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
					print(prev_line_reputations)
					reputationStats.write(prev_line_reputations + "\n")
					reputation_list.append(prev_line_reputations)
					first = 0
		return reputation_list
