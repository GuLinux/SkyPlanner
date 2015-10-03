FROM ubuntu:15.04
MAINTAINER Marco Gulino <marco@gulinux.net>

RUN apt-get update && apt-get install -y build-essential pkg-config libopencv-dev libwt-dev libwtdbosqlite-dev libwthttp-dev libwtdbopostgres-dev libgraphicsmagick++1-dev libnova-dev libboost-all-dev libhpdf-dev
RUN apt-get install -y cmake
RUN apt-get install -y libcurl4-openssl-dev

RUN mkdir /SkyPlanner /SkyPlanner-dist /SkyPlanner-install
WORKDIR /SkyPlanner-build
CMD cmake /SkyPlanner -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release && make -j4 && make package && cp -av SkyPlanner* /SkyPlanner-dist
