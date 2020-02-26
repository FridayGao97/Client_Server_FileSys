Description:
This project implement a client/server application. Clients can send request as "list, upload,download,and delete "files to server, and server will operate the corresponnding performance. And the server is that it is a deduplicating server. Files with the exact same content are stored at the server as a single copy of the file.
-----------------------------------------------------------------------------------------------------------------------
Acknowledgement:
https://stackoverflow.comquestions48224698appending-data-in-existing-xml-using-libxml2
http://www.xmlsoft.org/html/libxml-tree.html#xmlAddNextSibling
http://xmlsoft.org/tutorial/ar01s04.html

-----------------------------------------------------------------------------------------------------------------------
Instruction to run the code:
1. Go to the correct directory of the program.
2. Use make to compile the code to produce the executable for both client and server as default.
3. first run sever by "./ddupserver.out ./xxx #port" (eg. ./ddupserver.out ./servers 1123);
4. then run client by "./ddupclient.out ip_address port" (eg. ./ddupclient.out 127.0.0.1 1123);
5. Make clean will clear the executable.
6. be sure the files are in the same directory of clients.
7. Server will automatically create a folder for respository directory with xml.
-----------------------------------------------------------------------------------------------------------------------
Testing:
We have tested the client and server. client can upload and download a large size of file which exceed 2000 bytes (we test one file size more than 9000 bytes). As long as the server is running we can do all command, if server quit, it will erase all information. 

