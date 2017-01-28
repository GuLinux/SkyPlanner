FROM ubuntu:16.04
RUN apt-get update && apt-get install -y git python3-pip build-essential npm libboost-python-dev libnova-dev vim sqlite3 && apt-get clean && npm install -g bower && npm install -g gulp && ln -s /usr/bin/nodejs /usr/bin/node && pip3 install --upgrade pip
WORKDIR /skyplanner
RUN useradd -ms /bin/bash skyplanner 
USER skyplanner
ENV BOOST_PYTHON3_LIB=boost_python-py35 FLASK_APP=skyplanner/skyplanner.py LC_ALL=C.UTF-8 LANG=C.UTF-8

CMD /entrypoint

