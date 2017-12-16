WELCOME
=======

Welcome Nexus Data Injector, an open source C++ steganography application.

INFO
====

This program can Hide data within a chosen image, the image format must either be .BMP or .PNG 
since only these two formats are supported.

The size of the image can be up to 1/8 of the whole size of the image.
(e.g 800kb image can contain 100kb of data)

The data will be encrypted by a custom encryption method created only for this application.

HOW IT WORKS
============

Nexus reads the image's pixels and moves through them, setting all of the LSBs to 0
It then encrypts the given data, converts it to binary (1s and 0s) and sets the LSBs respectively

Nexus will not throw an error on data extraction if There is no data within the picture or the given password is incorrect,
Hence stopping unwanted visitors from being  able to even see if there is data within the file.

Nexus Encryption (NES (Nirex Encryption Service)) can encrypt any data with any key (The length of the key is unlimited)

COPYRIGHT
=========

Copyright (C) 2017 Nirex

Please read the LICENSE file for License details and terms of use.

CONTACT
=======

You can contact me at:

nirex.0[at]gmail[dot]com
