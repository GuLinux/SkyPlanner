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

WORKDIR /SkyPlanner-build

VOLUME /SkyPlanner
CMD cmake /SkyPlanner -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug -DLIBHARU_PREFIX=/usr -DHARU_LIBRARY=hpdfs -DEXTRA_LIBS="z;png" && \
  make ${MAKEOPTS} all install



#RUN apt-get update && apt-get install -y libcurl3 libwt38 libwthttp38 libwtdbosqlite38 libwtdbopostgres38 libgraphicsmagick++11 libboost-signals1.58.0 libboost-chrono1.58.0 libboost-thread1.58.0 \
  libboost-regex1.58.0 libboost-system1.58.0 libboost-filesystem1.58.0 libboost-random1.58.0 libboost-date-time1.58.0 libboost-program-options1.58.0 \
  libnova-0.14-0 postgresql-client xz-utils
#COPY @ubuntu_package_filename@ /
#COPY @entrypoint_filename@ /entrypoint
#RUN dpkg -i @ubuntu_package_filename@ && rm -f @ubuntu_package_filename@
#EXPOSE 8080
#VOLUME /var/cache/SkyPlanner/dss
#VOLUME /var/cache/SkyPlanner/static_files
#ENV STATIC_FILES_PATH=/usr/share/SkyPlanner STATIC_FILES_CACHE_PATH=/var/cache/SkyPlanner/static_files COPY_STATIC_FILES=false

#CMD /entrypoint --http-address 0.0.0.0 --http-port 8080 -c /etc/wt/wt_config.xml --docroot /usr/share/Wt\;/resources 

