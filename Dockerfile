FROM debian:latest

RUN apt-get update && apt-get install -y build-essential wget python libglfw3-dev libglew-dev uuid-dev
ADD . /app
WORKDIR /app
ARG MAGICLEAP_ARG
ENV MAGICLEAP_ENV ${MAGICLEAP_ARG}
RUN \
  if [ ! -z "$MAGICLEAP_ENV" ]; then export MAGICLEAP="$MAGICLEAP_ENV"; fi && \
  wget "https://nodejs.org/dist/v10.0.0/node-v10.0.0-linux-x64.tar.gz" -O node.tar.gz && \
  tar -zxf node.tar.gz && \
  rm node.tar.gz && \
  mv node-v10.0.0-linux-x64 node && \
  export PATH="$PATH:$(pwd)/node/bin" && \
  npm install --unsafe-perm . && \
  tar -czf exokit-linux-full.tar.gz --exclude=".*" --exclude="*.tar.gz" * && \
  tar -czf exokit-linux.tar.gz --exclude=".*" --exclude="*.tar.gz" *

