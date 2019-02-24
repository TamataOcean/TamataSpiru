# Dockerfile for running node-influx tests
# From syscoin's Danosphere
FROM influxteam/influx-testnet-box:v1
MAINTAINER Influx Team <dev@influxcoin.xyz>

# install node.js
USER root
RUN apt-get install --yes curl
RUN curl -sL https://deb.nodesource.com/setup_4.x | sudo -E bash -
RUN apt-get install --yes nodejs

# create a non-root user
RUN adduser --disabled-login --gecos "" tester

# run following commands from user's home directory
WORKDIR /home/tester

# copy the testnet-box files into the image
ADD . /home/tester/influx-testnet-box

# make tester user own the bitcoin-testnet-box
RUN chown -R tester:tester /home/tester/influx-testnet-box

# use the tester user when running the image
USER tester

# run commands from inside the testnet-box directory
WORKDIR /home/tester/influx-testnet-box
RUN npm install node-influx

# run test suite
CMD ["npm", "test"]
