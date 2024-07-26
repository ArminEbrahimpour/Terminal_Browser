#include<stdio.h>
#include<stdlib.h>
#include<curl/curl.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<libxml/HTMLparser.h>
#include<libxml/tree.h>
#include<libxml/xmlmemory.h>



#define MAX_TAG_LENGTH 10
#define MAX_CONTENT_LENGHT 10000

// need a hash map for getting the tab title and gets the pid of the process for killing it 
// can create an array which the indexes is the tab number 
char pages[10][1000000]={'\0'};

char url[50]={'\0'};


int page_index =0 ;

typedef struct {
    char *data;
    size_t length;
}CurlResponse;

void init_response(CurlResponse *response){
    response->length = 0;
    response->data = malloc(response->length + 1);
    if(!response->data){
    
        fprintf(stderr, "Failed to allocate the memory for the response\n");
        exit(EXIT_FAILURE);
    }
    response->data[0] = '\0';
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata){
    CurlResponse *response = (CurlResponse *)userdata;
    size_t new_length = response->length + size *nmemb;
    response->data = realloc(response->data, new_length + 1);
    if(!response->data){
        fprintf(stderr, "Failed to reallocate memory for response \n");
        exit(EXIT_FAILURE);
    }
    memcpy(response->data + response->length, ptr, size * nmemb);
    response->data[new_length] = '\0';
    response->length = new_length;
    return size * nmemb;

}



void banner() {
    printf("=======================================================\n");
    printf("|                                                      |\n");
    printf("|                     مرورگر                           |\n");
    printf("|                                                      |\n");
    printf("=======================================================\n");

}
void seperator(){
    printf("=======================================================\n");
}

void* curl(void* arg){
    CURL *curl;
    CURLcode res;
    CurlResponse response;


    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        init_response(&response);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
//        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Enable verbose mode for debugging
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
//        curl_easy_setopt(curl, CURLOPT_RETURNTRANSFER, 1L); // Return the result as a string
//        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Disable SSL certificate verification (insecure)

        res = curl_easy_perform(curl);
        if(res!= CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        } else {

    // Ensure the destination in pages is null-terminated and has enough space
           strncpy(pages[page_index], response.data, sizeof(pages[0]) - 1);
            pages[page_index][sizeof(pages[page_index]) - 1] = '\0'; // Ensure null-termination

          //  printf("%s\n", pages[page_index]); // Debugging: Print the copied string

            page_index++;


//            strcpy(pages[page_index], response.data);           
            //  pages[page_index] = response.data;
//            page_index ++;
//            printf("%s\n", response.data);

        }

        curl_easy_cleanup(curl);
        free(response.data);
    }

    curl_global_cleanup();
    
    pthread_exit(NULL);
}

void process_node(xmlNode * node){
    if (node == NULL ) return;


    if (node->type == XML_ELEMENT_NODE){
        char *tagName = (char *) node->name;
    }

       
    if(xmlStrcasecmp(node->name, (const xmlChar *)"h1") == 0 ||
            xmlStrcasecmp(node->name, (const xmlChar *)"h2") == 0 ||
            xmlStrcasecmp(node->name, (const xmlChar *)"h3") == 0 ||
            xmlStrcasecmp(node->name, (const xmlChar *)"h4") == 0 ||
            xmlStrcasecmp(node->name, (const xmlChar *)"h5") == 0 ||
            xmlStrcasecmp(node->name, (const xmlChar *)"h6") == 0 ||
            xmlStrcasecmp(node->name, (const xmlChar *)"header") == 0 ){
        // print the header contents in red
        if (node->children != NULL){
            xmlChar *content = xmlNodeGetContent(node);
            if (content){
                printf("\033[31m%s\033[0m\n", (char *) content);
                xmlFree(content);
            }
        }   
    }


    if( xmlStrcasecmp(node->name, (const xmlChar *)"hr") == 0 ||
            xmlStrcasecmp(node->name, (const xmlChar *)"p") == 0 ||
            xmlStrcasecmp(node->name, (const xmlChar *)"li") == 0 ||
            xmlStrcasecmp(node->name, (const xmlChar *)"ul") == 0) {
                
         
         

            // print the content or attribute
            if (node->children != NULL){
                xmlChar *content = xmlNodeGetContent(node);
                if (content){
                    printf("%s\n", (char *) content);
                    xmlFree(content);
                }
            }
    }

    // Traverse the next sibling and children 
    process_node(node->children);
    process_node(node->next);

}
  
int create_new_tab(){
    //char url[20] ;
    //get a url
    
    printf("Please Enter the url : "); 
    scanf("%s", &url);
    printf("this is the url you entered : %s \n", url);
    int pid = fork();
    if(pid==0){
        pthread_t id;
        int j =1;
        pthread_create(&id,NULL, curl, &j);
        pthread_join(id, NULL);
         
        //exit(0);
    }
    

    // create a process
    // create a thread 
    // send get request using that thread 
    // get the html data
    // send it to parser
    // add the pid to the hash table 
    wait(NULL);
    if (pid > 0){
        exit(0);
    }
    return 0;
} 
int show_tab_lists(){
    // iterate through the data structure of 
    // it has two options closing this tab or go back to menu
    // for closing the tab it calls the close tab function 
    // and for menu just show get_op function
    int page_num;

    printf("\n");
    seperator();
    printf("there are %d tabs open \n",page_index);
    printf("which page do you want to see ?\n");
    scanf("%d", &page_num);

    int size = sizeof(pages)/sizeof(pages[page_num]);
    // you can use libxml for html parsing in this section
    int length = strlen(pages[page_num -1]);
    htmlDocPtr doc = htmlReadMemory(pages[page_num-1], length, NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc == NULL){
        fprintf(stderr,"Failed to parse the html from memory");
        return 1;
    }
    // Get the root element
    xmlNode *rootElement = xmlDocGetRootElement(doc);

    // Process the document
    process_node(rootElement);

    // free the document
    xmlFreeDoc(doc);

    // Clean up parser
    xmlCleanupParser();
   
    printf("\n1.close this tab \n2.menu\n");
    printf("Please Enter the command you want : ");
    int op_num;
    scanf("%d", &op_num);
    switch(op_num){
        case 1:
            //delete this tab's index from the array
            for(int i = page_num-1;i< page_index; i++ ){
                strcpy(pages[i], pages[i+1]);
            }
            strcpy(pages[page_index], "");
            page_index --;
            break;
        case 2:
            return 0;

    } 

    return 0;
}
int close_tab(){
    // it will ask for the pid of the tab 
    // and kill it 
    printf("Ey baabaa\n");
    return 0;
}

 void get_op(){
    while(1){
        seperator();
        printf("1).New Tab\n2).Show Tab list\n3).Exit\n");
        int op_num;
        printf("Please Enter the command you wish for : " );
        scanf("%d",&op_num);
        switch(op_num){
            case 1: 
                create_new_tab();
                break;
            case 2:
                show_tab_lists();
                break;
            case 3:
                exit(0);
                break;  
        }
    }
}


int main(){
    banner();
    
    get_op();


return 0;
}
