#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <pthread.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <string.h>

#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/tree.h>

#define MAX_SIZE 2000	//size still need to make sure?
// Global mutex variable
pthread_mutex_t mutex;

char server_message[2000];
//client_message[0] = 0x01;
char buffer[1024];
pthread_t tid[60];
pthread_t tid_read[60];
pthread_t tid_send[60];
int tid_counter = 0;
bool counterInUse = true;	//wait for thread creation no longer need i, then change it
char connect_buf[2000];

char hashnum[2048];
char flist[2048];
char hashlist[2048];
int flistnum;
char direc[100];
int end_direc = 0;
char direc_xml[50];


int countfname(){
    int last = 0;
    flistnum = 0;
    xmlDocPtr pDoc = xmlReadFile(direc_xml, NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);
    //xmlDocPtr pDoc = xmlReadFile("./client/database.xml", NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);
    
    if (pDoc == NULL){
        //printf("Document not parsed successfully.\n");
        return 0;
    }

    xmlNodePtr root_element = xmlDocGetRootElement(pDoc);
    if (root_element == NULL){
        //printf("empty document\n");
        xmlFreeDoc(pDoc);
        return 0;
    }



	strcpy(flist,"");

    //here to write the content of node, hashcode
    // xmlChar* xml_hash = (xmlChar*) hash;
    // xmlChar* xml_fname = (xmlChar*) fname;
    //here to write <xxxx>
    xmlChar *xpath = (xmlChar*) "//knownas";

    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword;
    int countnum;
    //xmlNodePtr resdev;
    xmlChar* resd;
    //First we declare our variables.
    xmlXPathContextPtr context;
    //Initialize the context variable.
    context = xmlXPathNewContext(pDoc);
    if (context == NULL) {
            //printf("Error in xmlXPathNewContext\n");
            return 0;
    }
    //Apply the XPath expression.
    //result: a set of nodes and other information needed to iterate through the set and act on the results. 
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);

    if (result == NULL) {
            //printf("Error in xmlXPathEvalExpression\n");
            return 0;
    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
        
		return 0;
    }
    

    //to get the required node
    if (result) {

        nodeset = result->nodesetval;
		countnum = nodeset->nodeNr;
		for (i=0; i < nodeset->nodeNr; i++){
			keyword = xmlNodeListGetString(pDoc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
            int j;
            for(j=0;j < strlen(keyword); j++){
                flist[last + j] = keyword[j];
            }
            flist[last + j] = '\0';
            last = last + j+1;

		}
        xmlXPathFreeObject (result);
    }
    flistnum = last;
    // write back to file:
    xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
    xmlFreeDoc(pDoc);
    xmlCleanupParser();
    return countnum;
}


int gethashlist(){
    xmlDocPtr pDoc = xmlReadFile(direc_xml, NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);
    //xmlDocPtr pDoc = xmlReadFile("./client/database.xml", NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);
    
    if (pDoc == NULL){
        //printf("Document not parsed successfully.\n");
        return 0;
    }

    xmlNodePtr root_element = xmlDocGetRootElement(pDoc);
    if (root_element == NULL){
        //printf("empty document\n");
        xmlFreeDoc(pDoc);
        return 0;
    }

    //here to write <xxxx>
    xmlChar *xpath = (xmlChar*) "//hashname";

    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword;
    
    //xmlNodePtr resdev;
    xmlChar* resd;
    //First we declare our variables.
    xmlXPathContextPtr context;
    //Initialize the context variable.
    context = xmlXPathNewContext(pDoc);
    if (context == NULL) {
        //"Error in xmlXPathNewContext;
    }
    //Apply the XPath expression.
    //result: a set of nodes and other information needed to iterate through the set and act on the results. 
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);

    if (result == NULL) {

    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){

    }
    //to get the required node
    int last = 0;
    strcpy(hashlist,"");

    if (result) {

        nodeset = result->nodesetval;

        for (i=0; i < nodeset->nodeNr; i++){
            //Remember that in XML, the text contained within an element is a child node of that element, 
            keyword = xmlNodeListGetString(pDoc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
            int j;
            for(j=0;j < strlen(keyword); j++){
                hashlist[last + j] = keyword[j];
            }
            hashlist[last + j] = ';';
            last = last + j+1;

            xmlFree(keyword);
        }
        
        xmlXPathFreeObject (result);
    }

    // write back to file:
    xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
    xmlFreeDoc(pDoc);
    xmlCleanupParser();
    return 1;
}

//https://stackoverflow.com/questions/48224698/appending-data-in-existing-xml-using-libxml2
//http://www.xmlsoft.org/html/libxml-tree.html#xmlAddNextSibling
//http://xmlsoft.org/tutorial/ar01s04.html
int removexml(char *fname){
    xmlDocPtr pDoc = xmlReadFile(direc_xml, NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);
    //xmlDocPtr pDoc = xmlReadFile("./client/database.xml", NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);
    
    if (pDoc == NULL){
        //printf("Document not parsed successfully.\n");
        return 0;
    }

    xmlNodePtr root_element = xmlDocGetRootElement(pDoc);
    if (root_element == NULL){
        //printf("empty document\n");
        xmlFreeDoc(pDoc);
        return 0;
    }


    //here to write the content of node, hashcode
    xmlChar* xml_fname = (xmlChar*) fname;
    //here to write <xxxx>
    xmlChar *xpath = (xmlChar*) "//knownas";

    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword;
    
    //xmlNodePtr resdev;
    xmlChar* resd;
    //First we declare our variables.
    xmlXPathContextPtr context;
    //Initialize the context variable.
    context = xmlXPathNewContext(pDoc);
    if (context == NULL) {
        //Error in xmlXPathNewContext
    }
    //Apply the XPath expression.
    //result: a set of nodes and other information needed to iterate through the set and act on the results. 
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);

    if (result == NULL) {
            // printf("remove,Error in xmlXPathEvalExpression\n");
            return 0;
    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
        //printf("No result\n");
        
        xmlXPathFreeObject(result);
        // write back to file:
        xmlSaveFileEnc("./client/database.xml", pDoc, "UTF-8");
        xmlFreeDoc(pDoc);
        xmlCleanupParser();
        return 0;
    }
    int flag = 0;
    //to get the required node
    if (result) {
        nodeset = result->nodesetval;
        for (i=0; i < nodeset->nodeNr; i++) {
            //Remember that in XML, the text contained within an element is a child node of that element, 
            keyword = xmlNodeListGetString(pDoc, nodeset->nodeTab[i]->xmlChildrenNode, 1);


            //(!xmlStrcmp(cur->name, (const xmlChar *)"keyword"))  http://xmlsoft.org/tutorial/ar01s04.html
            //strcmp
            if(xmlStrcmp(keyword, xml_fname) == 0){
                //http://www.xmlsoft.org/html/libxml-tree.html#xmlUnlinkNode
                xmlUnlinkNode(nodeset->nodeTab[i]);
                //http://www.xmlsoft.org/html/libxml-tree.html#xmlFreeNode
                xmlFreeNode(nodeset->nodeTab[i]);
                flag =1;
            }
            xmlFree(keyword);
        }
        
        xmlXPathFreeObject (result);
    }

    // write back to file:
    xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
    xmlFreeDoc(pDoc);
    xmlCleanupParser();
    return flag;
}

int check_removexml(char *fname){
    xmlDocPtr pDoc = xmlReadFile(direc_xml, NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);
    //xmlDocPtr pDoc = xmlReadFile("./client/database.xml", NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);
   
    
    if (pDoc == NULL){
        // printf("Document not parsed successfully.\n");
        return 0;
    }

    xmlNodePtr root_element = xmlDocGetRootElement(pDoc);
    if (root_element == NULL){
        // printf("empty document\n");
        xmlFreeDoc(pDoc);
        return 0;
    }

    printf("Root Node is %s\n", root_element->name);

    //here to write the content of node, hashcode
    xmlChar* xml_fname = (xmlChar*) fname;
    //here to write <xxxx>
    xmlChar *xpath = (xmlChar*) "//file";

    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword;
    
    //xmlNodePtr resdev;
    xmlChar* resd;
    //First we declare our variables.
    xmlXPathContextPtr context;
    //Initialize the context variable.
    context = xmlXPathNewContext(pDoc);
    if (context == NULL) {
        // printf("Error in xmlXPathNewContext\n");
    }
    //Apply the XPath expression.
    //result: a set of nodes and other information needed to iterate through the set and act on the results. 
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);

    if (result == NULL) {
        
        xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
        xmlFreeDoc(pDoc);
        xmlCleanupParser();
        return 0;
    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
       
        // write back to file:
        xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
        xmlFreeDoc(pDoc);
        xmlCleanupParser();
        return 0;
    }
    int flag = 0;
    //to get the required node
    if (result) {
        nodeset = result->nodesetval;
       
        for (i=0; i < nodeset->nodeNr; i++) {
            int childnum = xmlChildElementCount(nodeset->nodeTab[i]);

            if(childnum == 1){
                //http://www.xmlsoft.org/html/libxml-tree.html#xmlUnlinkNode
                xmlUnlinkNode(nodeset->nodeTab[i]);
                //http://www.xmlsoft.org/html/libxml-tree.html#xmlFreeNode
                xmlFreeNode(nodeset->nodeTab[i]);
                flag =1;
                break;
            }
            xmlFree(keyword);
        }
        
        xmlXPathFreeObject (result);
    }

    // write back to file:
    xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
    xmlFreeDoc(pDoc);
    xmlCleanupParser();
    return flag;
}


void createXML(char *uri){
	int rc;
    xmlTextWriterPtr writer;
    xmlChar *tmp;

    /* Create a new XmlWriter for uri, with no compression. */
    writer = xmlNewTextWriterFilename(uri, 0);
    if (writer == NULL) {
        printf("testXmlwriterFilename: Error creating the xml writer\n");
        return;
    }

    /* Start the document with the xml default for the version,
     * encoding ISO 8859-1 and the default for the standalone
     * declaration. */
    rc = xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterStartDocument\n");
        return;
    }

	/* Start an element named "EXAMPLE". Since thist is the first
     * element, this will be the root element of the document. */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "repository");
    if (rc < 0) {
        printf("testXmlwriterFilename: repository...\n");
        return;
    }

    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return;
    }

	/* Here we could close the elements ORDER and EXAMPLE using the
     * function xmlTextWriterEndElement, but since we do not want to
     * write any other elements, we simply call xmlTextWriterEndDocument,
     * which will do all the work. */
    rc = xmlTextWriterEndDocument(writer);
	if (rc < 0) {
        printf("testXmlwriterFilename: Error at xmlTextWriterEndDocument\n");
        return;
    }

    xmlFreeTextWriter(writer);
}


int addhashxml(char *hash, char *fname){
    xmlDocPtr pDoc = xmlReadFile(direc_xml, NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);
    
    if (pDoc == NULL){
        // printf("Document not parsed successfully.\n");
        return 0;
    }

    xmlNodePtr root_element = xmlDocGetRootElement(pDoc);
    if (root_element == NULL){
        // printf("empty document\n");
        xmlFreeDoc(pDoc);
        return 0;
    }


    //here to write the content of node, hashcode
    xmlChar* xml_hash = (xmlChar*) hash;
    xmlChar* xml_fname = (xmlChar*) fname;
    //here to write <xxxx>
    xmlChar *xpath = (xmlChar*) "/file";

    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword;
    
    //xmlNodePtr resdev;
    xmlChar* resd;
    //First we declare our variables.
    xmlXPathContextPtr context;
    //Initialize the context variable.
    context = xmlXPathNewContext(pDoc);
    if (context == NULL) {
            // printf("Error in xmlXPathNewContext\n");
            
    }
    //Apply the XPath expression.
    //result: a set of nodes and other information needed to iterate through the set and act on the results. 
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);

    if (result == NULL) {
            // printf("Error in xmlXPathEvalExpression\n");
    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){

        xmlNodePtr pNode = xmlNewNode(NULL, (xmlChar*)"file");
        // xmlNodeSetContent(pNode, xml_hash);
        xmlAddChild(root_element, pNode);

        xmlNodePtr cNode = xmlNewNode(NULL, (xmlChar*)"hashname");
        xmlNodeSetContent(cNode, xml_hash);
        xmlAddChild(pNode, cNode);

        xmlNodePtr kNode = xmlNewNode(NULL, (xmlChar*)"knownas");
        xmlNodeSetContent(kNode, xml_fname);
        xmlAddSibling(cNode, kNode);

        xmlXPathFreeObject(result);
         // write back to file:
        xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
        xmlFreeDoc(pDoc);
        xmlCleanupParser();
        return 1;
    }

    
    xmlNodePtr pNode = xmlNewNode(0, (xmlChar*)"file");
    // xmlNodeSetContent(pNode, xml_hash);
    xmlAddChild(root_element, pNode);
    
    xmlNodePtr cNode = xmlNewNode(0, (xmlChar*)"hashname");
    xmlNodeSetContent(cNode, xml_hash);
    xmlAddChild(pNode, cNode);
    // printf("p Node is %s\n", pNode->name);

    xmlNodePtr kNode = xmlNewNode(0, (xmlChar*)"knownas");
    xmlNodeSetContent(kNode, xml_fname);
    xmlAddSibling(cNode, kNode);

    xmlXPathFreeObject (result);
    //to get the required node
    if (result) {
        xmlNodePtr pNode = xmlNewNode(0, (xmlChar*)"file");
        // xmlNodeSetContent(pNode, xml_hash);
        xmlAddChild(root_element, pNode);
        
        xmlNodePtr cNode = xmlNewNode(0, (xmlChar*)"hashname");
        xmlNodeSetContent(cNode, xml_hash);
        xmlAddChild(pNode, cNode);

        xmlNodePtr kNode = xmlNewNode(0, (xmlChar*)"knownas");
        xmlNodeSetContent(kNode, xml_fname);
        xmlAddSibling(cNode, kNode);

        xmlXPathFreeObject (result);
    }

    // write back to file:
    xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
    xmlFreeDoc(pDoc);
    xmlCleanupParser();
    return 1;
}

int updatexml(char *hash, char *fname){
    xmlDocPtr pDoc = xmlReadFile(direc_xml, NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET);    
    if (pDoc == NULL){
        // printf("Document not parsed successfully.\n");
        return 0;
    }

    xmlNodePtr root_element = xmlDocGetRootElement(pDoc);
    if (root_element == NULL){
        // printf("empty document\n");
        xmlFreeDoc(pDoc);
        return 0;
    }

    //here to write the content of node, hashcode
    xmlChar* xml_hash = (xmlChar*) hash;
    xmlChar* xml_fname = (xmlChar*) fname;
    //here to write <xxxx>
    xmlChar *xpath = (xmlChar*) "//hashname";

    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword;
    
    //xmlNodePtr resdev;
    xmlChar* resd;
    //First we declare our variables.
    xmlXPathContextPtr context;
    //Initialize the context variable.
    context = xmlXPathNewContext(pDoc);
    if (context == NULL) {
            // printf("Error in xmlXPathNewContext\n");
    }
    //Apply the XPath expression.
    //result: a set of nodes and other information needed to iterate through the set and act on the results. 
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);

    if (result == NULL) {
        // printf("Error in xmlXPathEvalExpression\n");
    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
        // printf("updatexml: No result, now appending first one\n");

        xmlXPathFreeObject(result);
         // write back to file:
        xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
        xmlFreeDoc(pDoc);
        xmlCleanupParser();
        return 0;
        //addhashxml(hash,fname);
        
        
    }
    int flag = 0;
    //to get the required node
    if (result) {
        nodeset = result->nodesetval;
   
        for (i=0; i < nodeset->nodeNr; i++){
            //Remember that in XML, the text contained within an element is a child node of that element, 
            keyword = xmlNodeListGetString(pDoc, nodeset->nodeTab[i]->xmlChildrenNode, 1);

            //(!xmlStrcmp(cur->name, (const xmlChar *)"keyword"))  http://xmlsoft.org/tutorial/ar01s04.html
            if(xmlStrcmp(keyword, xml_hash) == 0){
                xmlNodePtr pNode = xmlNewNode(0, (xmlChar*)"knownas");
                xmlNodeSetContent(pNode, xml_fname);
                xmlAddSibling(nodeset->nodeTab[i], pNode);
                flag = 1;
            }
            xmlFree(keyword);
        }
        xmlXPathFreeObject(result);
    }

    // write back to file:
    xmlSaveFileEnc(direc_xml, pDoc, "UTF-8");
    xmlFreeDoc(pDoc);
    xmlCleanupParser();
    return flag;
}


int cfileexists(const char * filename){
    /* try to open file to read */
    FILE *file;

    if (file = fopen(filename, "r")){

        fclose(file);
        return 1;
    }
    return 0;
}

//https://www.geeksforgeeks.org/c-program-find-size-file/
int findSize(char file_name[]) 
{ 
    // opening the file in read mode 
    FILE* fp = fopen(file_name, "r"); 
  
    // checking if the file exist or not 
    if (fp == NULL) { 
        // printf("File Not Found!\n"); 
        return -1; 
    } 
  
    fseek(fp, 0L, SEEK_END); 
  
    // calculating the size of the file 
    int res = ftell(fp); 
  
    // closing the file 
    fclose(fp); 
  
    return res; 
} 

char md5[33];
int md5sum(const char * filename){
    unsigned char c[MD5_DIGEST_LENGTH];
    int i;
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (inFile == NULL) 
	{
        printf ("%s can't be opened.\n", filename);
        return 0;
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++){
        //sprintf(&md5[i*2], "%02x", (unsigned int)c[i]);
        sprintf(&md5[i*2], "%02x", c[i]);
    }    

    fclose (inFile);
    // return &(c[0]);
    return 1;
}

// char wbuff[2000];
void server_op(int newSocket, char* cmd_line){
	char FByte = cmd_line[0];
	char connect_buf[2000];

	// //list--------------------------------------------------------------------------
	if (FByte == 0x00){	
        
		strcpy(connect_buf,"");

		int filenum = countfname();
        if(filenum == 0){
            strcpy(connect_buf,"");
            connect_buf[0] = 0xFF;
            strcat(connect_buf, "SERROR no file to list");
            send(newSocket, connect_buf, strlen(connect_buf), 0);
        }
		connect_buf[0] = 0x01;

		char temp[2];
		temp[0] = (filenum>>8) & 0xFF;
		temp[1] = filenum & 0xFF;


		connect_buf[1] = temp[0];
		connect_buf[2] = temp[1];

		int i = 0;
		for(i; i < flistnum; i++){
			connect_buf[3+i] = flist[i];
		}
		
		send(newSocket, connect_buf, i+3, 0);

	}
	// //upload--------------------------------------------------------------------------
	else if(FByte == 0x02){ 
        strcpy(connect_buf,"");		


        pthread_mutex_lock(&mutex);

        cmd_line[strcspn(cmd_line,"\n")] = 0;
		connect_buf[0] = 0x03;
        //write files in server from client 
		char ufname[50];
        char uxmlfname[50];
		strcpy(ufname, direc);
		int i = 1;
		while(cmd_line[i] != '\0'){
			ufname[i+end_direc-1] = cmd_line[i];
            uxmlfname[i-1] = cmd_line[i];
			i++;
		}
		ufname[i+end_direc-1] = '\0';
        uxmlfname[i-1] = '\0';



		char temp[4];
		for(int j = 0; j< 4; j++)
		        temp[3-j] = cmd_line[i+1+j];
		int ufsize = *(int *)temp;

        if(cfileexists(ufname) == 1){
            strcpy(connect_buf,"");
            connect_buf[0] = 0xFF;
            strcat(connect_buf, "SERROR file already exists");
            send(newSocket, connect_buf, strlen(connect_buf), 0);
            strcpy(connect_buf,"");
            return;
 		}

        else{
            int recv_size = MAX_SIZE;


            FILE * write_fd;

            write_fd = fopen (ufname, "w");

            int j;
            if(2000 >ufsize){
                
                if (recv(newSocket,cmd_line,MAX_SIZE,0)>0){
                    
                    //fputs(cmd_line,stdout);
                    
                    fwrite(cmd_line,1, ufsize, write_fd);    
                    // write(rbuff, 1, strlen(rbuff),write_fd);
                }
                fclose(write_fd);
            }

            else{
                int i = ufsize/2000;
                for (j = 0;j<i;j++){
                    if (recv(newSocket,cmd_line,MAX_SIZE,0)>0){

                        fwrite(cmd_line,1, MAX_SIZE, write_fd);    

                    }                
                }
                if (recv(newSocket,cmd_line,ufsize-2000*j,0)>0){
                    fwrite(cmd_line,1, ufsize-2000*j, write_fd);
                }
                fclose(write_fd);
            }
            strcpy(cmd_line,"");

            //check with md5
            md5sum(ufname);

            

            if(updatexml(md5,uxmlfname)== 0){
                //printf("built new hashcode\n");
                addhashxml(md5,uxmlfname);
            }
            send(newSocket, connect_buf, strlen(connect_buf), 0);

            
        }
        

		
        //update flist, hashlist
        countfname();
        gethashlist();
        pthread_mutex_unlock(&mutex);

	}
	// //delete--------------------------------------------------------------------------
	else if(FByte == 0x04){	
        strcpy(connect_buf,"");	
        printf("server receive delete\n");
        cmd_line[strcspn(cmd_line,"\n")] = 0;
		connect_buf[0] = 0x05;
		//remove the reuired file in server side
		char rfname[50];
        
		int i = 1;
        while(cmd_line[i] != '\0'){
            rfname[i-1] = cmd_line[i];
            i++;
        }
		rfname[i-1] = '\0';
        if (cfileexists(rfname) == 0){
            
            //printf("0xff file is not in server side\n");
            strcpy(connect_buf,"");
            connect_buf[0] = 0xFF;
            strcat(connect_buf, "SERROR file not found");
            send(newSocket, connect_buf, strlen(connect_buf), 0);
            return;
        }

        int r = removexml(rfname);
        check_removexml(rfname);

        if(r == 1){

            pthread_mutex_lock(&mutex);


            int status;

            char src[100];
            strcpy(src, direc);
            strcat(src, rfname);

            status = remove(src);
            
            if (status != 0){
                printf("Unable to delete the file\n");
                perror("Following error occurred");
            }
            

            //update flist, hashlist
            countfname();
            gethashlist();

            //send to client 
            send(newSocket, connect_buf, strlen(connect_buf), 0);


            pthread_mutex_unlock(&mutex);
        }

        //end of critical section
	
	}
	// //download--------------------------------------------------------------------------
	else if(FByte == 0x06){	

        pthread_mutex_lock(&mutex);	

        strcpy(connect_buf,"");	
		connect_buf[0] = 0x07;
        cmd_line[strcspn(cmd_line,"\n")] = 0;
        //check the file exist in server side, if yes, send to client.
        char dfname[50];
        char dxmlfname[50];
        strcpy(dfname, direc);
        int i = 1;
        while(cmd_line[i] != '\0'){
        	dfname[i+end_direc-1] = cmd_line[i];
            dxmlfname[i-1] = cmd_line[i];
        	i++;
        }
        dfname[i+end_direc-1] = '\0';
        dxmlfname[i-1] = '\0';
        printf("dfname: %s\n",dfname);
        printf("dxmlfname: %s\n",dxmlfname);

        if (cfileexists(dfname) == 0){
            strcpy(connect_buf,"");
            connect_buf[0] = 0xFF;
            strcat(connect_buf, "SERROR file not found");
            send(newSocket, connect_buf, strlen(connect_buf), 0);
            strcpy(connect_buf,"");
            pthread_mutex_unlock(&mutex);
            return;
 		}
        else{

            int usize = findSize(dfname);

            connect_buf[1] = (usize >> 24) & 0xFF;
            connect_buf[2] = (usize >> 16) & 0xFF;
            connect_buf[3] = (usize >> 8) & 0xFF;
            connect_buf[4] = usize & 0xFF;
            send(newSocket, connect_buf, 6, 0);

            FILE* read_fd;
            char buffer[2000];

            int j;
            read_fd = fopen (dfname, "r");
            if (usize < 2000){
;
                fread(buffer, usize, 1, read_fd);

                send(newSocket, buffer, usize, 0);
                fclose(read_fd);
            }

            else{
                int i = usize/2000;
                for ( j = 0;j<i;j++){
                    fread(buffer, 2000, 1, read_fd);

                    send(newSocket, buffer, 2000, 0);
                    memset(buffer,0,MAX_SIZE);
                }
                fread(buffer, (usize-2000*j), 1, read_fd);
                send(newSocket, buffer, (usize-2000*j), 0);
                memset(buffer,0,(usize-2000*j));
                fclose(read_fd);
            }

        }
    	pthread_mutex_unlock(&mutex);


	}
	// //quitting--------------------------------------------------------------------------
	else if(FByte == 0x08){	
        strcpy(connect_buf,"");	
		connect_buf[0] = 0x09;
		send(newSocket, connect_buf, strlen(connect_buf), 0);
		
	}
    else if(FByte == 0x21){
        strcpy(connect_buf,"");
        connect_buf[0] = 0x21;
        // strcat(connect_buf, "worong command.");
        send(newSocket, connect_buf, 1, 0);
    }
	// --------------------------------------------------------------------------
	else{	//error case
        strcpy(connect_buf,"");
        connect_buf[0] = 0xFF;
        // strcat(connect_buf, "worong command.");
        send(newSocket, connect_buf, 1, 0);
	}
    strcpy(connect_buf,"");
}

void * readingThread(void *arg){
	int newSocket = *((int *)arg);
    char client_message[2000];
	while(1)
	{
        
		memset(client_message,0,2000);

		if (recv(newSocket,&client_message,sizeof(client_message),0) == 0)
			printf("Error\n");
		else{
			// printf("%s\n", client_message);
			server_op(newSocket, client_message);
		}
		
	}
}


void * socketThread(void *arg)
{
	// Send message to the client socket 
	int newSocket = *((int *)arg);

	if( pthread_create(&tid_read[tid_counter], NULL, readingThread, &newSocket) != 0 )
		printf("Failed to create thread\n");

	// if( pthread_create(&tid_send[tid_counter], NULL, sendingThread, &newSocket) != 0 )
	// 	printf("Failed to create thread\n");
	

	counterInUse = false;

	pthread_join(tid_read[tid_counter],NULL);
	pthread_join(tid_send[tid_counter],NULL);
	
	
	send(newSocket,buffer,13,0);
	printf("Exit socketThread \n");
	close(newSocket);
	pthread_exit(NULL);
	
}


int main(int argc, char *argv[])
{
	int serverSocket, newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;


	// generate directory address
    //client_message[strcspn(client_message,"\n")] = 0;
    char *arg1 = argv[1];
    for(end_direc; end_direc < strlen(arg1);end_direc++){

        direc[end_direc] = arg1[end_direc];
    }
    if (mkdir(direc,0777)){
        printf("Unable to create folder\n");
   
    }

    strcpy(direc_xml,direc);
    strcat(direc_xml,"/database.xml");
    direc_xml[strcspn(direc_xml,"\n")] = 0;
    direc[end_direc] = '/';
    end_direc = end_direc+1;
    direc[strcspn(direc,"\n")] = 0;



    // generate port number
	int port_num = atoi(argv[2]);
	//Create the socket. 
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	// Configure settings of the server address struct
	// Address family = Internet 
	serverAddr.sin_family = AF_INET;
	
	//Set port number, using htons function to use proper byte order 
	serverAddr.sin_port = htons(port_num);
	
	//Set IP address to localhost 
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	//Set all bits of the padding field to 0 
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
	
	//Bind the address struct to the socket 
	bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));



    // direc_xml = strcat(direc, "database.xml");
	createXML(direc_xml);

    // Initialize our global mutex variable
    pthread_mutex_init(&mutex,NULL);

	//Listen on the socket, with 40 max connection requests queued 
	if(listen(serverSocket,50)==0)
		printf("Listening\n");
	else
		printf("Error\n");

	while(1)
	{
		//Accept call creates a new socket for the incoming connection
		addr_size = sizeof serverStorage;
		newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
        
		//for each client request creates a thread and assign the client request to it to process
        //so the main thread can entertain next request
		if( newSocket >= 0)
		{
			printf("user number %d \n", tid_counter);
			if( pthread_create(&tid[tid_counter], NULL, socketThread, &newSocket) != 0 )
				printf("Failed to create thread\n");

			while(counterInUse);	//while i is still in use
			tid_counter++;
			counterInUse = false;
			
		}
		else if( newSocket < 0)
			printf("Failed to connect");
			
        if( tid_counter >= 50)
        {
			tid_counter = 0;
			while(tid_counter < 50)
			{
				pthread_join(tid[tid_counter++],NULL);
			}
			tid_counter = 0;
        }
    }
  return 0;
}