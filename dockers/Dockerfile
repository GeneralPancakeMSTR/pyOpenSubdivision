# docker build -t ubuntu_docker .
# docker run -it --name ubuntu_container --rm ubuntu_docker /bin/bash
# docker run -v '/mnt/raid1/git/GeneralPancakeMSTR/pyOpenSubdivision:/root/pyOpenSubdivision' -v "/$(pwd)/sync:/root/sync" --name ubuntu_container --rm -it ubuntu_docker /bin/bash /root/sync/command.sh

# docker run -v "$(pwd)/../pyOpenSubdiv:/root/pyOpenSubdiv" --name pyOpenSubdiv_container --rm -it ubuntu_docker /bin/bash -c "cd pyOpenSubdiv && /bin/bash"

# docker run -v "$(pwd)/../pyOpenSubdiv:/root/pyOpenSubdiv" --name pyOpenSubdiv_container --rm -it ubuntu_docker /bin/bash -c "cd pyOpenSubdiv && /bin/bash"



FROM ubuntu:latest 

WORKDIR /root

RUN apt-get update && apt-get upgrade -y && apt-get clean

RUN apt-get install -y curl

RUN apt-get install -y python3 python3-dev python3-distutils
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.10 2
RUN curl -s https://bootstrap.pypa.io/get-pip.py -o get-pip.py && \
    python3 get-pip.py --force-reinstall && \
    rm get-pip.py

COPY requirements.txt .
RUN pip3 install -r requirements.txt


