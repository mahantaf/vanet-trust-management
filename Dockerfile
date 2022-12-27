# Use the following command to create the docker image
# `sudo DOCKER_BUILDKIT=1 docker build . --ssh default=$SSH_AUTH_SOCK` (https://stackoverflow.com/a/67293284)
FROM ubuntu:16.04
ARG DEBIAN_FRONTEND=noninteractive

LABEL Description="Dockerised Simulation of Urban MObility(SUMO)"
LABEL maintainer="Nishchay Agrawal <agrawal.nishchay5@gmail.com>"
SHELL ["/bin/bash", "-c"]

#ENV SUMO_VERSION 0.31.0
ENV OMNETPP_VERSION 5.6.1
ENV USER nagrawal63
ENV HOME /home/$USER
ENV CMAKE_VERSION 3.22
ENV CMAKE_BUILD 2
ENV GITHUB_REPO legendary-simu5g-broccoli

RUN adduser $USER --disabled-password

# Install system dependencies.
RUN apt-get update && apt-get -qq install git wget cmake python3 g++ libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev libgl2ps-dev python3-dev swig default-jdk maven libeigen3-dev unzip

#Update cmake version
RUN mkdir ~/temp && \
    cd ~/temp && \
    wget https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION.$CMAKE_BUILD/cmake-$CMAKE_VERSION.$CMAKE_BUILD.tar.gz && \
    tar -xzvf cmake-$CMAKE_VERSION.$CMAKE_BUILD.tar.gz && \
    cd cmake-$CMAKE_VERSION.$CMAKE_BUILD/ && \
    ./bootstrap && \
    make -j$(nproc) && \
    make install && \
    cd ~ && \
    rm -rf temp/

# Create ssh directory and scan github to add keys to known hosts
RUN mkdir /root/.ssh/ &&\
    touch /root/.ssh/known_hosts &&\
    ssh-keyscan github.com >> /root/.ssh/known_hosts

# Clone code from github    
RUN --mount=type=ssh \
    git clone --progress --verbose git@github.com:nagrawal63/legendary-simu5g-broccoli.git $HOME/${GITHUB_REPO}

# Download and extract source code
RUN  cd $HOME/$GITHUB_REPO && \
     git clone --recursive https://github.com/eclipse/sumo &&\
     export SUMO_HOME="$PWD/sumo"

# Configure and build from source.
RUN cd $HOME/$GITHUB_REPO/sumo && \
    mkdir build/cmake-build && \
    cd build/cmake-build && \
    cmake ../.. && \
    make -j$(nproc) && \
    make install

# Download omnetpp
RUN rm -rf $HOME/${GITHUB_REPO}/omnetpp-${OMNETPP_VERSION} && \
    cd $HOME/$GITHUB_REPO/ && \
    wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-${OMNETPP_VERSION}/omnetpp-${OMNETPP_VERSION}-src-linux.tgz && \
    tar xzf omnetpp-${OMNETPP_VERSION}-src-linux.tgz && \
    rm omnetpp-${OMNETPP_VERSION}-src-linux.tgz

#Install omnetpp dependencies
RUN apt-get -y install build-essential gcc bison flex perl \
 python qt5-default libqt5opengl5-dev tcl-dev tk-dev \
 libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-3.0-0 libpcap-dev

#Install omnetpp and export path to environment variable
RUN cd $HOME/$GITHUB_REPO/omnetpp-${OMNETPP_VERSION} &&\
    source setenv -f &&\
    sed -i 's/WITH_OSGEARTH=yes/WITH_OSGEARTH=no/g' configure.user && \
    sed -i 's/WITH_OSG=yes/WITH_OSG=no/g' configure.user && \
    ./configure && \
    make -j$(nproc) &&\
    export PATH="${HOME}/${GITHUB_REPO}/omnetpp-/${OMNETPP_VERSION}/bin:$PATH"

#Instantiate Xauth
RUN cd $HOME/ &&\
    touch .Xauthority

#Download and unzip veins
# RUN cd $HOME &&\
#     wget http://veins.car2x.org/download/veins-5.2.zip &&\
#     unzip veins-5.2.zip &&\
#     rm veins-5.2.zip

# #Download and unzip simu5g
# RUN cd $HOME &&\
#     wget https://github.com/Unipisa/Simu5G/archive/refs/tags/v1.1.0.zip &&\
#     unzip v1.1.0.zip &&\
#     mv Simu5G-1.1.0/ simu5g/ &&\
#     rm v1.1.0.zip