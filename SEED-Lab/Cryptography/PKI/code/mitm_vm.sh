#!/bin/bash

# copy the certificate, key file, and the configuration file from the shared folder
cp /volumes/example.crt /certs
cp /volumes/example.key /certs
cp /volumes/example_apache_ssl.conf

# restart apache, type the password for the key file
service apache2 restart
