import matplotlib.pyplot as plt
import sys

def extract_reputation_list(fileName):
    start_time = 0
    first_message = True
    reputations_with_time = {}
    message_times = {}
    with open(fileName, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if "Reputations" in line:
                line = line.split(',')
                time = float(line[1])
                i = 2

                while i < len(line):
                    car_id = int(line[i].split('[')[1].split(']')[0])
                    reputation = float(line[i+1].split('\n')[0])
                    if car_id not in reputations_with_time.keys():
                        reputations_with_time[car_id] = list()
                    
                    reputations_with_time[car_id].append((time, reputation))
                    i+=2
            elif "ReceivedEventLocation" in line:
                car_id = int(line.split('[')[1].split(']')[0])
                time = float(line.split(',')[1])
                if first_message:
                    start_time = time
                if car_id not in message_times.keys():
                    message_times[car_id] = [(start_time, 0)]
                message_times[car_id].append((time, message_times[car_id][-1][1] + 1))
                print(str(car_id) + ", " + str(message_times[car_id][-1]))
    return reputations_with_time, message_times

def plot_reputations(reputations_list, title):
    fig = plt.figure()
    ax = plt.subplot(111)

    for car in reputations_list.keys():
        x, y = zip(*reputations_list[car])
        label = "car " + str(car)
        ax.plot(x, y, label=label)

    ax.legend(loc='upper right', bbox_to_anchor=(1.2, 1),
          fancybox=True, shadow=True)
    ax.set_xlabel('Time(s)')
    ax.set_ylabel('Reputation(0-1)')
    plt.title(label=title)
    plt.tight_layout()
    plt.grid('on', linestyle='--')

    plt.savefig('{}{}.pdf'.format(directory, title.replace(" ", '_')))

    plt.show()

def dual_reputation_and_message_plotter(reputations_list, message_times, title):
    fig = plt.figure()
    ax = plt.subplot(111)

    for car in reputations_list.keys():
        x, y = zip(*reputations_list[car])
        label = "car " + str(car)
        ax.plot(x, y, label=label)

    ax.legend(loc='upper right', bbox_to_anchor=(1.2, 1),
          fancybox=True, shadow=True)
    ax.set_xlabel('Time(s)')
    ax.set_ylabel('Reputation(0-1)')

    ax2 = ax.twinx()
    for car in reputations_list.keys():
        x, y = zip(*message_times[car][1:])
        label = "car " + str(car)
        ax2.plot(x, y, label=label, linestyle='dashed')

    plt.title(label=title)
    plt.tight_layout()
    plt.grid('on', linestyle='--')

    plt.savefig('{}{}.pdf'.format(directory, title.replace(" ", '_')))

    plt.show()

if __name__ == "__main__":
    fileName = sys.argv[1]
    directory = '/'.join(fileName.split('/')[:-1]) + "/"
    print(directory)

    reputations_list, message_times = extract_reputation_list(fileName)
    # print(reputations_list)
    # plot_reputations(reputations_list, sys.argv[2])
    dual_reputation_and_message_plotter(reputations_list, message_times, sys.argv[2])

