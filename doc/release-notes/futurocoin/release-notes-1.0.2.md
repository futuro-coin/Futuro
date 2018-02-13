Futuro Core version 1.0.2.0
=========================

Futuro Core version 1.0.2.0 is now available from

https://github.com/futuro-coin/Futuro-binaries

This is a new minor version release, bringing bug fix and performance improvements. It is strongly recommended to upgrade to this version.

How to Upgrade
==============
If you are running an older version, shut it down. Wait until it has completely shut down, then run the installer.
After successful installation run your node/wallet. In case your node throws error message please reindex your blockchain database. It can be done by adding reindex = 1  to futurocoin.conf file. When reindexing is finished shut down the node, remove the reindex = 1  entry and start the node. 


Notable changes
===============

Block rejection
---------------
There were to many blocks rejected by the network due to failure in masternode broadcast message and validation. 

1.0.2.0 Change log
================

- [`35c353c8c`](https://github.com/futuro-coin/Futuro/commit/35c353c8c) Core: reject unauthorized masternode broadcas


Credits
=======

Thanks to everyone who directly contributed to this release.
