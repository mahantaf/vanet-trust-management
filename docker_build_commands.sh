
#Command to build docker container
sudo DOCKER_BUILDKIT=1 docker build . --ssh default=$SSH_AUTH_SOCK -t simu5gcontainer

#Command to run docker container
sudo docker run --name simu5gTrial -i -t simu5gcontainer bash

sudo docker run -it --net=host -e DISPLAY -v /tmp/.X11-unix simu5gcontainer bash


#Command to get GUI
get xauth token from host using: `xauth list`

go into docker container: `xauth add <token>`