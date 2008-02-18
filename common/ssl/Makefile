CXXFLAGS=-g

ssl_tests: ssl_test.o pf_ssl_ssl.o certificate.o private_key.o
	$(CXX) *.o -lssl -o ssl_tests

clean:
	@rm -f *.o 2>&1 >/dev/null
