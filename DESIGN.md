
Design Decision:
1. For mapping file names to its contents, initially we want to use two linked list as a hashset structure, one linked list will first store its hash number and link to another linked list that stores the filename with same hash number. Then we a structure array to store all hash linked list so that we can iterate all hash number to find the specific contents and the filename under that. However, due to there are six midterm exams for computer engineering student and a number of other courses' workload during the past month, the linked list code will not properly and we seemed not have much time to debug it and fixed it. So, at the end, we use xml method to retrieve every filename and hash number seperately, and using "database.xml" shown as description of assignment to map the filename with its content(its hash num).

2. For the server responding efficiently and quickly to client requests, we send the communciation message right after we got the required basic information without further complex compution before sending. For send file and its contents, we will first send the required communication message without file contens first, then open the file and read from to buffer then send the contents buffre to receivers. 

3. For making sure the internal data structures areconsistent when multiplethreads try to perform changes, we use mutex(pthread_mutex_lock(&mutex) / pthread_mutex_unlock(&mutex) ) to build a cirtical section under the case of upload file, download file, delete file. With adding lock to threads, threads will not occupy the CPU during other threads dealing with file changes, and will not interrupt the changes in operating files. It is safe for server and client to get the wantted result. 

4. For matches the hash of already stored file(s), we first will have a temp file to store the uploading client's file undter server directory, and using md5 check sum to get its hash number. If our hash linked list works, we will first to search the hash linked list to find whether there was hash number stored before; it find it, then remove the temp file from server folder, and append the filename to that file linked list which are lined by hash number; if not found it, we add keep the temp file and append its hash number and filename to hash number list. But, now we do not have a work linked list sturcture, we just keep the temp file and update its hashnum and filename to xml, if same hash number we just add sibling to hash number<hashname> as <knownas>, if there is no same one, we add sibling to <file> and add child <hashname>, <knownas> under that <file>. 

5. For dealing with client errors, we add error cases under the whole input condition, also error cases under each specific required input condtion. 

6. 

7. For the server starting on a given directory, we will check whether directory exist or create sucessfully, then create a xml file under that to store the intefacing information. we also use a function to check the exsistence of the request file before any operation.

8. For 0 size file, we will not upload to server. And server will not receive any message from client.
