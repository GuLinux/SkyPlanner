FROM ubuntu:15.10
MAINTAINER Marco Gulino <marco@gulinux.net>

RUN apt-get update && apt-get install -y libcurl3 libwt38 libwthttp38 libwtdbosqlite38 libwtdbopostgres38 libgraphicsmagick++11 libboost-signals1.58.0 libboost-chrono1.58.0 libboost-thread1.58.0 \
  libboost-regex1.58.0 libboost-system1.58.0 libboost-filesystem1.58.0 libboost-random1.58.0 libboost-date-time1.58.0 libboost-program-options1.58.0 \
  libnova-0.14-0 postgresql-client xz-utils
COPY @ubuntu_package_filename@ /
COPY @entrypoint_filename@ /entrypoint
RUN dpkg -i @ubuntu_package_filename@ && rm -f @ubuntu_package_filename@
EXPOSE 8080
VOLUME /var/cache/SkyPlanner/dss
VOLUME /var/cache/SkyPlanner/static_files
ENV STATIC_FILES_PATH=/usr/share/SkyPlanner STATIC_FILES_CACHE_PATH=/var/cache/SkyPlanner/static_files COPY_STATIC_FILES=false

CMD /entrypoint --http-address 0.0.0.0 --http-port 8080 -c /etc/wt/wt_config.xml --docroot /usr/share/Wt\;/resources 
