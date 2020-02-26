.PHONY: clean

# defalut
SocketThread: 

	gcc -o ddupclient.out -std=c99 -pthread `xml2-config --cflags` ddupclient.c `xml2-config --libs`
	gcc -o ddupserver.out -std=c99 -pthread `xml2-config --cflags` ddupserver.c `xml2-config --libs` -lssl -lcrypto
	

clean:
	$(RM) *.out