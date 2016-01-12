FROM ubuntu:15.10
MAINTAINER Marco Gulino <marco@gulinux.net>

RUN apt-get update && apt-get install -y build-essential pkg-config libopencv-dev libwt-dev libwtdbosqlite-dev libwthttp-dev libwtdbopostgres-dev libnova-dev libboost-all-dev cmake unzip
# TO BE REMOVED:
RUN apt-get install -y libgraphicsmagick++1-dev
RUN apt-get install -y libcurl4-openssl-dev

RUN curl -LO https://github.com/libharu/libharu/archive/RELEASE_2_3_0.zip && unzip /RELEASE_2_3_0.zip
WORKDIR libharu-RELEASE_2_3_0/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DLIBHPDF_SHARED=Off -DLIBHPDF_STATIC=On && make && make install
WORKDIR /
RUN rm -rf /RELEASE_2_3_0.zip /libharu-RELEASE_2_3_0

RUN mkdir /SkyPlanner /SkyPlanner-dist /SkyPlanner-install
WORKDIR /SkyPlanner-build
CMD cmake /SkyPlanner -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=@DOCKER_BUILD_TYPE@ -DLIBHARU_PREFIX=/usr -DHARU_LIBRARY=hpdfs -DEXTRA_LIBS="z;png" && \
  make -j@DOCKER_BUILD_JOBS@ && make package && cp -av SkyPlanner* /SkyPlanner-dist/@ubuntu_package_filename@
