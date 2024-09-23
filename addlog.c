#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>

static const char encodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const int modTable[] = {0, 2, 1};

char* base64_encode(const unsigned char *data, size_t inputLength, size_t *outputLength) {
    *outputLength = 4 * ((inputLength + 2) / 3);
    char *encodedData = malloc(*outputLength);
    if (encodedData == NULL) return NULL;

    for (int i = 0, j = 0; i < inputLength;) {
        uint32_t octet_a = i < inputLength ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < inputLength ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < inputLength ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encodedData[j++] = encodingTable[(triple >> 3 * 6) & 0x3f];
        encodedData[j++] = encodingTable[(triple >> 2 * 6) & 0x3f];
        encodedData[j++] = encodingTable[(triple >> 1 * 6) & 0x3f];
        encodedData[j++] = encodingTable[(triple >> 0 * 6) & 0x3f];
    }

    for (int i = 0; i < modTable[inputLength % 3]; i++)
        encodedData[*outputLength - 1 - i] = '=';
    
    encodedData[*outputLength] = 0;
    return encodedData;
}

int main(int argc, char **argv)
{
    //input must be a single string
    if(argc!=2)
    {
        printf("Usage of program is as follows \"./addlog log_string\"\n");
        exit(0);
    }

    FILE* log;
    FILE* loghead;
    //prepare files; check for log.txt
    if(access("log.txt", F_OK)==0)
    {
        //log.txt is present; check for loghead.txt
        if(access("loghead.txt", F_OK)!=0)
        {
            //loghead is missing
            printf("failed: File \"loghead.txt\" is missing\n");
            exit(1);
        }
        log = fopen("log.txt", "a");
    }
    else
    {
        //log.txt is missing
        log = fopen("log.txt", "w");
        loghead = fopen("loghead.txt", "w");
        char* begin = "begin";
        fprintf(loghead, "%s", begin);
        fclose(loghead);
    }

    //eliminate any new line characters from input
    if(strlen(argv[1])>0)
    {
        for(int i=0; i<strlen(argv[1])-1; i++)
        {
            if(argv[1][i]=='\n')
            {
                argv[1][i]=' ';
            }
        }
    }

    //read the previous hash from loghead
    loghead = fopen("loghead.txt", "r");
    char buffer[44];
    fread(buffer, sizeof(buffer), 1, loghead);
    fclose(loghead);

    //get the current time
    time_t cur;
    time(&cur);
    char *time = ctime(&cur);
    time[strlen(time)-1] = '\0';
    
    //create string of the elements combined; print the string to log.txt
    char* input_text;
    input_text = malloc(strlen(time)+strlen(buffer)+strlen(argv[1])+7);
    sprintf(input_text, "%s - %s %s", time, buffer, argv[1]);
    fprintf(log, "%s\n", input_text);
    fclose(log);
    
    // Initialize OpenSSL algorithms
    OpenSSL_add_all_algorithms();

    unsigned char hash[EVP_MAX_MD_SIZE]; // Buffer for the hash
    unsigned int hash_len;

    // Create and initialize the context
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, input_text, strlen(input_text));
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    
    // Base64 encode the SHA256 hash
    size_t outputLength;
    char *base64_string = base64_encode(hash, hash_len, &outputLength);

    //print base64 string to loghead.txt
    loghead = fopen("loghead.txt", "w");
    fprintf(loghead, "%s", base64_string);
    fclose(loghead);

    // Clean up
    EVP_MD_CTX_free(ctx);
    free(base64_string);
    EVP_cleanup(); // Clean up the OpenSSL algorithms library
    free(input_text);
}