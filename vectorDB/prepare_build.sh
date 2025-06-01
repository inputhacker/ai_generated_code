# 필요한 개발 도구 설치
sudo apt-get update
sudo apt-get install -y build-essential cmake git

# Hnswlib 설치
git clone https://github.com/nmslib/hnswlib.git
cd hnswlib
mkdir build && cd build
cmake ..
make
sudo make install
cd ../..

# nlohmann/json 설치
git clone https://github.com/nlohmann/json.git
cd json
mkdir build && cd build
cmake ..
make
sudo make install
cd ../..

# SentencePiece 설치
git clone https://github.com/google/sentencepiece.git
cd sentencepiece
mkdir build && cd build
cmake ..
make
sudo make install
cd ../..
