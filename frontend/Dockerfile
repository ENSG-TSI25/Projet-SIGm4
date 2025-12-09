FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake git wget ca-certificates \
    qtbase5-dev qttools5-dev qttools5-dev-tools \
    libqt5svg5-dev libqt5xmlpatterns5-dev \
    qgis libqgis-dev \
    libgdal-dev libpq-dev libpqxx-dev libproj-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app/

RUN mkdir -p /app/build && cd /app/build && \
    cmake .. && \
    make -j$(nproc)

CMD ["/app/build/frontend"]
