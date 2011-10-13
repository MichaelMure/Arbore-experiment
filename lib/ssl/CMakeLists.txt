add_library(ssl SHARED
   #certificate.cpp
   connection.cpp
   connection_ssl.cpp
   connection_nossl.cpp
   #crl.cpp
   #pf_ssl_ssl.cpp
   #pf_ssl_nossl.cpp
   #private_key.cpp
   )
SET(PFLIBS ${PFLIBS} ssl)
