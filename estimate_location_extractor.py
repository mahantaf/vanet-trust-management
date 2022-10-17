from posixpath import split
import sys
import matplotlib.pyplot as plt
import pickle


def extract_estimate_locations(directory, fileName):
    estimate_locs, timestamps = [], []
    with open(fileName, 'r') as f:
        estimateLocations = open(directory + "estimateLocations.csv", "w")
        lines = f.readlines()
        # print(lines)
        for line in lines:
            line = line.strip()
            splitLine = line.split(',')
            if "EstimateLocation: " in line:
                splitLine = line.split(':')[1]
                estimateLocations.write(splitLine + "\n")
                splitLine = splitLine.split(',', 1)
                location = splitLine[1][2:-3].split(',')
                location = [float(x) for x in location]
                # Remove the z coordinate since we are not using it
                del location[2]
                estimate_locs.append(location)
                timestamps.append(float(splitLine[0]))
    return estimate_locs, timestamps

                
def plot_estimate_locations(estimate_locs, timestamps):
    plot_label = 'Estimate locations with 1 evil cars_2'
    x, y = zip(*estimate_locs)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.set_xlabel("Time(s)")
    ax.set_ylabel("X")
    ax.set_zlabel("Y")
    ax.plot(timestamps, x, y, label=plot_label)
    ax.legend()

    # Saving figure for future reference
    plt.savefig('{}.pdf'.format(plot_label.replace(" ", '_')))
    pickle.dump(fig, open('{}.fig.pickle'.format(plot_label.replace(" ", '_')), 'wb'))

    plt.show()

def open_existing_fig(fig_label):
    figx = pickle.load(open(fig_label + '.fig.pickle', 'rb'))
    figx.show()


if __name__ == "__main__":
    if sys.argv[1] == 'plot': 
        fileName = sys.argv[2]
        directory = '/'.join(fileName.split('/')[:-1]) + "/"

        (estimate_locs, times) = extract_estimate_locations(directory, fileName)

        plot_estimate_locations(estimate_locs, times)
    elif sys.argv[1] == 'open':
        figName = sys.argv[2]
        print("Opening fig with name: " + figName)
        open_existing_fig(figName)