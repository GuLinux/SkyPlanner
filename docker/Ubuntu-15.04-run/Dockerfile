FROM ubuntu:15.04
MAINTAINER Marco Gulino <marco@gulinux.net>

RUN apt-get update && apt-get install -y build-essential cmake qtbase5-dev pkg-config libopencv-dev
RUN apt-get install -y libgraphicsmagick++1-dev
RUN apt-get install -y libqcustomplot-dev
RUN apt-get install -y libgphoto2-dev libindi-dev libnova-dev

RUN mkdir /GalaxyHunter /GalaxyHunter-dist /GalaxyHunter-install
WORKDIR /GalaxyHunter-build
CMD cmake /GalaxyHunter -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release && make -j4 && make package && cp -av GalaxyHunter* /GalaxyHunter-dist
