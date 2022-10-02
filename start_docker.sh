#!/bin/bash

docker run -v "$PWD/package:/root/" --name pyOpenSubdiv_container --rm -it ubuntu_docker /bin/bash -c "python3 -m pip install . && /bin/bash"

sudo rm "$PWD/package/.bash_history"
sudo rm -r "$PWD/package/.cache"
sudo rm -r "$PWD/package/.ipython"
sudo rm -r "$PWD/package/build"
sudo rm -r "$PWD/package/pyOpenSubdiv.egg-info"
sudo rm -r "$PWD/package/pyOpenSubdiv/__pycache__"
sudo rm -r "$PWD/package/pyOpenSubdiv/clib/__pycache__"

# docker run -v "$PWD/package:/root/" --name pyOpenSubdiv_container --rm -it ubuntu_docker /bin/bash -c "python3 -m pip install -e . && /bin/bash"