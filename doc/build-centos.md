CENTOS BUILD NOTES
====================
Orginalnie z https://github.com/litecoin-project/litecoin/blob/ba8ed3a93be7e7a97db6bc00dd7280fa2f1548bc/doc/build-unix.md

Rozwiazanie dependencji
---------------------

These dependencies are required:

 Library     | Centos package   | Purpose          | Description
 ------------|------------------|------------------|----------------------
 libssl      | openssl-devel    | Crypto           | Random Number Generation, Elliptic Curve Cryptography
 libboost    | boost-devel      | Utility          | Library for threading, data structures, etc
 libevent    | libevent-devel   | Networking       | OS independent asynchronous networking

Optional dependencies:

 Library     | Centos package   | Purpose          | Description
 ------------|------------------|------------------|----------------------
 miniupnpc   | miniupnpc-devel  | UPnP Support     | Firewall-jumping support
 libdb4.8    | libdb4-devel     | Berkeley DB      | Wallet storage (only needed when wallet enabled)
             | libdb4-cxx-devel |                  |
 qt          | qt5-qttools-devel| GUI              | GUI toolkit (only needed when GUI enabled)
             | qt5-qtbase-devel |                  |
 protobuf    | protobuf-devel   | Payments in GUI  | Data interchange format used for payment protocol (only needed when GUI enabled)
 libqrencode | qrencode-devel   | QR codes in GUI  | Optional for generating QR codes (only needed when GUI enabled)
 univalue    | ???????          | Utility          | JSON parsing and encoding (bundled version will be used unless --with-system-univalue passed to configure) [http://mirror.ip-projects.de/gentoo/gentoo-portage/dev-libs/univalue/ ; http://ftp.uni-stuttgart.de/gentoo-portage/dev-libs/univalue/]
 libzmq3     | zeromq3          | ZMQ notification | Optional, allows generating ZMQ notifications (requires ZMQ version >= 4.x)

Step-by-Step Instalacja wszystkich pakietow pod kompilacji
---------------------
```
yum -y update
yum -y install epel-release
curl -o /tmp/rpmforge-release-0.5.3-1.el7.rf.x86_64.rpm http://repository.it4i.cz/mirrors/repoforge/redhat/el7/en/x86_64/rpmforge/RPMS/rpmforge-release-0.5.3-1.el7.rf.x86_64.rpm
rpm -Uvh /tmp/rpmforge-release-0.5.3-1.el7.rf.x86_64.rpm
yum -y update
yum -y install gcc-c++ libtool make autoconf automake openssl-devel libevent-devel boost-devel libdb4-devel libdb4-cxx-devel
yum -y install miniupnpc-devel
yum -y install qt5-qttools-devel qt5-qtbase-devel protobuf-devel
yum -y install qrencode-devel
```

Kompilacja
---------------------
!!! Kompilowac na systemie skonfigurowanym takim jak system docelowy (wersje pakietow, fedora sie nienadaje) !!!

Pobrac zrodlo na serwer i kolejno:
```
./autogen.sh
mkdir _install
./configure --disable-tests --disable-gui-tests --prefix=$PWD/_install
make
make install
```
Pakiet bedzie sie znajdowal w katalogu \_install

Odpalic z:
``/home/futurecoin/futurecoin/bin/futurocoind -conf=/home/futurecoin/futurecoin/futurocoin.conf -daemon -datadir=/home/futurecoin/futurecoin/``



Paczki potrzebne do odpalenia skompilowanej aplikacji.
---------------------
```
yum -y install boost libdb4-cxx miniupnpc libevent
```

Odpalenie
---------------------
Narazie z palca ``_install/bin/futurocoind &``, oczywiscie jest ``_install/bin/futurocoind --help`` i milion opcji... dojdziemy

Bitcoin Core switches & konfig file (narazie tylko officialne info)
---------------------
https://en.bitcoin.it/wiki/Running_Bitcoin










Bitcoin Blockchain API
---------------------
Stworzyc uzytkownika bitcoin
przelogowac sie na uzytkownika bitcoin
przejsc do katalogu domowego bitcoin
```
curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.6/install.sh | bash
nvm install v4.8.4
sudo yum -y install epel-release
sudo yum -y update
sudo yum -y install zeromq3-devel
sudo yum -y install gcc gcc-c++ make openssl-devel
npm install -g bitcore
```
Wykonac w katalogu gdzie ma zostac stworzony node z danymi/blockchainami
```
bitcore create [nodename]
```
wykonac po wejsciu do katalogu [nodname]
```
bitcore install insight-api
```
startowanie bitcore noda bedac w katalogu [nodename]
```
bitcored
```

NginX
---------------------
zainstalowac i wgrac konfig z https://bitbucket.org/futurenetdevevelopers/bitcoin-blockchain-api/src/77e782ba8374?at=master

MongoDB
---------------------
``sudo vi /etc/yum.repos.d/mongodb-org-3.4.repo``
```
[mongodb-org-3.4]
name=MongoDB Repository
baseurl=https://repo.mongodb.org/yum/redhat/$releasever/mongodb-org/3.4/x86_64/
gpgcheck=1
enabled=1
gpgkey=https://www.mongodb.org/static/pgp/server-3.4.asc
```
``sudo yum install -y mongodb-org``

```
sudo systemctl enable mongod
sudo systemctl start mongod
```
```
sudo mongo
use admin
db.createUser({user:"bitcore",pwd:"secretpasswd", roles:[{role:"root",db:"admin"}]})
exit
```

Wgranei i odpalenie Bitcoin Blockchain API
---------------------
node w wersji v6.11.4
```
nvm install v6.11.4
nvm use v6.11.4
```

pobrac zrodlo https://bitbucket.org/futurenetdevevelopers/bitcoin-blockchain-api/src/77e782ba8374?at=master i odpalamy w katalogu po rozpakowaniu:
```
npm install
npm run build
npm run start:prod
```
config w src/config/prod.ts



Bitcoind w systemd
---------------------
``sudo vi /etc/systemd/system/bitcored.service``
```
[Unit]
Description=bitcored.service
After=network.target

[Service]
Type=simple
User=bitcoin
Environment="PATH=$PATH:/home/bitcoin/.nvm/versions/node/v4.8.4/bin"
WorkingDirectory=/home/bitcoin/bitcoin
ExecStart=/home/bitcoin/.nvm/versions/node/v4.8.4/bin/bitcored
ExecStop=/bin/kill -9 $MAINPID
ExecReload=/bin/kill -2 $MAINPID
KillMode=process
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

Dodanie futurocond do systemd
---------------------
``sudo vi /etc/systemd/system/futurecored.service``
```
[Unit]
Description=futurecored.service
After=network.target

[Service]
Type=simple
User=futurecoin
Environment="PATH=$PATH:/home/futurecoin/futurecoin/bin"
WorkingDirectory=/home/futurecoin/futurecoin
ExecStart=/home/futurecoin/futurecoin/bin/futurocoind -conf=/home/futurecoin/futurecoin/futurocoin.conf -datadir=/home/futurecoin/futurecoin/
ExecStop=/bin/kill -9 $MAINPID
ExecReload=/bin/kill -2 $MAINPID
KillMode=process
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

PM2
---------------------
!!! Nie uzywac pm2 unstartup, zabije i wyczysci wszystkie aktualnie dzialajace aplikacjie !!!

po skompilowaniu apki ``npm run build`` odpalamy ja bedac w jej katalogu przy pomocy:
``pm2 start npm --name bitchain -- run start:prod``

Jezeli pm2 niezostalo dodane do systemd wywolujemy:
``pm2 startup``
i potem jako root odpalamy wygenerowana komende eg.:
``sudo env PATH=$PATH:/home/bitchain/.nvm/versions/node/v6.11.4/bin /home/bitchain/.nvm/versions/node/v6.11.4/lib/node_modules/pm2/bin/pm2 startup systemd -u bitchain --hp /home/bitchain``

Scala building toolkit sbt User Wallet Manager Node
---------------------
``yum install bc``

install java (8u151) from http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html

```
curl https://bintray.com/sbt/rpm/rpm | sudo tee /etc/yum.repos.d/bintray-sbt-rpm.repo
yum install sbt
```
Budowac przy pomocy komendy ``sbt assembl`` bedac w katalogu walleta

Odpalanie User Wallet Manager Node
---------------------
install java jre 8u151
pobrac i rozpakowac http://www.oracle.com/technetwork/java/javase/downloads/jce8-download-2133166.html do ``<JAVA_HOME>/jre/lib/security/``
odpalic poprzez ``java -jar app-assembly-0.1-SNAPSHOT.jar``


RPC user/password i inne
---------------------
pozwala na polaczenie poprzez HTTP JSON.

Masternode
---------------------
https://dashpay.atlassian.net/wiki/spaces/DOC/pages/56655887/Mining+vs.+Masternode
pytanie czy my to bedziemy miec?






---------------------
---------------------
---------------------
---------------------
---------------------

!!! Nie istotne !!! <<<<<<<<<<<<<<<<<<-------------------------
-----------------------------------------------------------------------------


Problem ze zmiennymi CPPFLAGS_FOR_BUILD & CFLAGS_FOR_BUILD <> juz niepotrzebne
---------------------
W pliku `src/secp256k1/Makefile.am`
odnalezc zmienna CPPFLAGS_FOR_BUILD i dodac przed nia:
```
CPPFLAGS_FOR_BUILD =
CFLAGS_FOR_BUILD =
```

C++ compile 7.2
---------------------
```
sudo yum install libmpc-devel mpfr-devel gmp-devel
curl -o /tmp/gcc-7.2.0.tar.gz ftp://ftp.mirrorservice.org/sites/sourceware.org/pub/gcc/releases/gcc-7.2.0/gcc-7.2.0.tar.gz
tar xvfj gcc-7.2.0.tar.gz
cd gcc-7.2.0
./configure --enable-languages=c,c++
./configure --disable-multilib --enable-languages=c,c++
make -j 4
make install
```

Automake 1.15
---------------------
```
curl -o /tmp/automake-1.15.tar.gz http://ftp.gnu.org/gnu/automake/automake-1.15.tar.gz
tar xvzf automake-1.15.tar.gz
cd automake-1.15
./configure --prefix=/usr
make
make install
```

libtool
---------------------
```
wget https://ftp.gnu.org/gnu/libtool/libtool-2.4.6.tar.gz
./configure --disable-static --prefix=/usr/bin
make
make install
```

pkg-config
---------------------
```
wget https://pkg-config.freedesktop.org/releases/pkg-config-0.29.2.tar.gz
./configure --prefix=/usr/bin        \
            --with-internal-glib \
            --disable-host-tool  \
            --docdir=/usr/share/doc/pkg-config-0.29.2 &&
make
make install
```
