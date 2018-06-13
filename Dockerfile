FROM debian:latest

ARG MAGICLEAP_ARG
ENV MAGICLEAP_ENV ${MAGICLEAP_ARG}

RUN apt-get update && apt-get install -y build-essential wget python libglfw3-dev libglew-dev uuid-dev

ADD . /app
WORKDIR /app

RUN \
  rm -Rf dump && \
  wget "https://nodejs.org/dist/v10.4.1/node-v10.4.1-linux-x64.tar.gz" -O node.tar.gz && \
  tar -zxf node.tar.gz && \
  rm node.tar.gz && \
  mv node-v10.4.1-linux-x64 node
RUN \
  export PATH="$PATH:$(pwd)/node/bin" && \
  if [ ! -z "$MAGICLEAP_ENV" ]; then export MAGICLEAP="$MAGICLEAP_ENV"; fi && \
  npm install --unsafe-perm .
RUN \
  mkdir -p /tmp/exokit-bin/bin /tmp/exokit-bin/lib/exokit && \
  cp -R . /tmp/exokit-bin/lib/exokit && \
  cp exokit-bin.sh /tmp/exokit-bin/bin/exokit && \
  cd /tmp/exokit-bin && \
  tar -czf /app/exokit-linux-bin.tar.gz --exclude=".*" --exclude="*.tar.gz" * && \
  cd /app && \
  tar -czf exokit-linux-full.tar.gz --exclude=".*" --exclude="*.tar.gz" * && \
  rm -R node && \
  tar -czf exokit-linux.tar.gz --exclude=".*" --exclude="*.tar.gz" *

