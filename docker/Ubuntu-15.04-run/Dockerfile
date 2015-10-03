FROM ubuntu:15.04
MAINTAINER Marco Gulino <marco@gulinux.net>

RUN apt-get update && apt-get install -y libcurl3 libwt38 libwthttp38 libwtdbosqlite38 libwtdbopostgres38 libgraphicsmagick++3 libboost-signals1.55.0 libboost-chrono1.55.0 libboost-thread1.55.0 \
  libboost-regex1.55.0 libboost-system1.55.0 libboost-filesystem1.55.0 libboost-random1.55.0 libboost-date-time1.55.0 libboost-program-options1.55.0 \
  libnova-0.14-0 postgresql-client xz-utils
COPY @ubuntu_package_filename@ /
RUN dpkg -i @ubuntu_package_filename@ && rm -f @ubuntu_package_filename@
EXPOSE 8080
VOLUME /var/cache/SkyPlanner/dss
CMD /usr/bin/SkyPlanner --http-address 0.0.0.0 --http-port 8080 -c /etc/wt/wt_config.xml --docroot /usr/share/Wt\;/resources 