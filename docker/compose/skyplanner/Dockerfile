FROM ubuntu:16.04
MAINTAINER Marco Gulino <marco@gulinux.net>

RUN apt-get update && apt-get install -y build-essential pkg-config libopencv-dev libwt-dev libwtdbosqlite-dev \
    libwthttp-dev libwtdbopostgres-dev libnova-dev libboost-all-dev cmake unzip gdb
# TO BE REMOVED:
RUN apt-get install -y libgraphicsmagick++1-dev libcurl4-openssl-dev

RUN curl -LO https://github.com/libharu/libharu/archive/RELEASE_2_3_0.zip && \
    unzip /RELEASE_2_3_0.zip && \
    mkdir -p libharu-RELEASE_2_3_0/build && \
    cd libharu-RELEASE_2_3_0/build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DLIBHPDF_SHARED=Off -DLIBHPDF_STATIC=On && \
    make && make install && cd / && \
    rm -rf /RELEASE_2_3_0.zip /libharu-RELEASE_2_3_0
VOLUME /SkyPlanner

COPY entrypoint /entrypoint
WORKDIR /SkyPlanner-build
#CMD  ["/bin/bash", "/entrypoint", "--http-address", "0.0.0.0", "--http-port", "8080", "-c", "/data/wt_config.xml", "--docroot", "/data/SkyPlanner-Static\;/resources"] 
CMD  /entrypoint -- --http-address 0.0.0.0 --http-port 8080 -c /data/wt_config.xml --docroot /data/SkyPlanner-Static\;/resources,/moon_phases,/loading-64.png,/skyplanner_logo.png
