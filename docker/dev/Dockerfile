FROM ubuntu:16.04
RUN apt-get update && apt-get install -y git python3-pip build-essential npm libboost-python-dev libnova-dev && apt-get clean && npm install -g bower && npm install -g gulp && ln -s /usr/bin/nodejs /usr/bin/node && pip3 install --upgrade pip
WORKDIR /skyplanner
RUN useradd -ms /bin/bash skyplanner 
USER skyplanner
ENV BOOST_PYTHON3_LIB=boost_python-py35
CMD /entrypoint

