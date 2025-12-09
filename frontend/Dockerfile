FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y software-properties-common \
    && add-apt-repository ppa:ubuntugis/ubuntugis-unstable -y \
    && apt-get update

RUN apt-get install -y \
    build-essential cmake git wget ca-certificates \
    qtbase5-dev qttools5-dev qttools5-dev-tools \
    libqt5svg5-dev libqt5xmlpatterns5-dev \
    qgis libqgis-dev \
    libgdal-dev libpq-dev libpqxx-dev \
    libproj-dev proj-bin proj-data \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app/

RUN mkdir -p /app/frontend/build && cd /app/frontend/build && \
    cmake .. && \
    make -j$(nproc)

CMD ["/app/frontend/build/frontend"]