


FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    ca-certificates \
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libqt5svg5-dev \
    libqt5xmlpatterns5-dev \
    qgis \
    libqgis-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app
COPY . /app/
# Build your Qt/QGIS C++ application
RUN mkdir -p /app/build && cd /app/build && \
    cmake /app \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_FLAGS="-I/usr/include/qgis -I/usr/include/qgis/qgis" \
    && make -j$(nproc)


# Default command → run your GUI
CMD ["/app/build/frontend"]