

# 5G VANET Trust Management Simulation

In this project we used omnet++ to simulate Vehicular Adhoc Network (VANET) and add trust management algorithm to that.
We used [Simu5G](http://simu5g.org/) project to model the 5g network and we added our trust management algorithms along with that project code.
This documentation is written to help you replicate our work or contribute to it.

## Table of Contents

- [Installation](#installation)
- [Execution](#execution)
- [Build Project](#build-project)
- [Contributing](#contributing)
- [License](#license)

## Installation

In order to run the simulation you have to have oment++ and sumo installed
on your machine. There different methods that you can use to install this project.
Our recommended version is to use the Docker file provided in the repository but you are free to
use other methods.

### Install on Ubuntu

1. Install system dependencies:
```bash
apt-get update && \\
apt-get -qq install git wget cmake python3 g++ libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev libgl2ps-dev python3-dev swig default-jdk maven libeigen3-dev unzip
```
2. Update cmake version:

```bash
mkdir ~/temp && \
cd ~/temp && \
wget https://github.com/Kitware/CMake/releases/download/v3.22.2/cmake-3.22.2.tar.gz && \
tar -xzvf cmake-3.22.2.tar.gz && \
cd cmake-3.22.2/ && \
./bootstrap && \
make -j$(nproc) && \
make install && \
cd ~ && \
rm -rf temp/
```
3. Clone the repository and checkout `mahan-build` branch:

```bash
git clone https://github.com/mahantaf/vanet-trust-management.git && \
cd vanet-trust-management/ && \
git checkout mahan-build
```

4. Download and extract SUMO:

```bash
git clone --recursive https://github.com/eclipse/sumo &&\
export SUMO_HOME="$PWD/sumo"
```

5. Configure and build SUMO from source:

```bash
cd sumo && \
mkdir build/cmake-build && \
cd build/cmake-build && \
cmake ../.. && \
make -j$(nproc) && \
make install
```

6. Install omnetpp dependencies:

```bash
apt-get -y install build-essential gcc bison flex perl \
python qt5-default libqt5opengl5-dev tcl-dev tk-dev \
libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-3.0-0 libpcap-dev
```

7. Install omnetpp and export path to environment variable:

```bash
cd omnetpp-5.6.1 && \
source setenv -f &&\
sed -i 's/WITH_OSGEARTH=yes/WITH_OSGEARTH=no/g' configure.user && \
sed -i 's/WITH_OSG=yes/WITH_OSG=no/g' configure.user && \
./configure && \
make -j$(nproc) &&\
export PATH="/path/to/github-repo/omnetpp-5.6.1/bin:$PATH"
```

### Install using Dockerfile

1. Download and build the Dockerfile in this repository using the following command:

```bash
DOCKER_BUILDKIT=1 docker build . --ssh default=$SSH_AUTH_SOCK -t <CONTAINER_NAME>
```

## Execution

### Run on Ubuntu

Simply run the `omnetpp` command or directly run it from the source:

```bash
/path/to/github-repo/omnetpp-5.6.1/bin/omnetpp
```

### Run using docker (Ubuntu)

1. Generate the xauth token to allow GUI applications from docker container to run on the local host:

```bash
 xauth list
```

2. Run the image using the following command:

```bash
docker run -it --net=host -e DISPLAY -v /tmp/.X11-unix <CONTAINTER_NAME> bash
```

3. Add the listed token from step 4 to the docker container:

```bash
$(containter) xauth add <token>
```

4. Start omnetpp:

```bash
$(containter) ~/mahantp/legendary-simu5g-broccoli/omnetpp-5.6.1/bin/omnetpp
```

### Run using docker (Mac)

1. Install XQuartz to allow GUI applications from docker container to run on the local host:

```bash
brew cask install xquartz
```

2. Open XQuartz:

```bash
open -a XQuartz
```

3. Export display:

```bash
export DISPLAY=:0
```

4. Ensure the "Allow connections from network clients" option in Preferences >> Security is turned on:
![XQuartz security setting page](https://sourabhbajaj.com/images/blog/2017-02/xquartz_preferences.png "XQuartz security setting page")

5. Add all hosts as an allowed source:

```bash
/opt/X11/bin/xhost +
```

6. Run the image using the following command:

```bash
docker run -it --net=host -e DISPLAY=host.docker.internal:0 -v /Users/<username>/.Xauthority:/home/mahantp/.Xauthority <CONTAINTER_ID> bash
```

7. Start omnetpp:

```bash
$(containter) ~/mahantp/legendary-simu5g-broccoli/omnetpp-5.6.1/bin/omnetpp
```

## Build Project

After successfully run the omnetpp you can add the simulation as a project in omentpp and start run the simulations.
Here are the steps to replicate the simulation.

1. Start the container and run the omnetpp
2. Select the repository directory as the workspace directory
3. From the menu choose "File" >> "Open Projects from File System"
4. Specify repository directory as directory for searching existing projects
5. After the search is complete select the following projects from the list
   6. inet
   7. veins
   8. veins_inet
   9. Simu5G-1.1.0
10. For each of the projects do the following
    11. From the menu choose "Project" >> "Properties"
    12. On the left menu select "OMNeT++" >> "Makemake"
    13. In the Makemake window select "src:makemake" from the list and click "options" on the right
    14. In the new window choose tab "Custom"
    15. In the text box put the following code and save the option:
    ```
    MSGC:=$(MSGC) --msg6
    ```
9. From the menu choose "Project" >> "Build All"
10. After building successfully you can the simulation which is location at:
`./Simu5G-1.1.0/simulations/NR/cars/omnetpp.ini`
