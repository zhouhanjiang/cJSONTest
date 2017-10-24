using namespace std;
#include <math.h>
#include <float.h>
#include <limits.h>
#include <iostream>
#include <fstream>  
#include <streambuf>
#include "xcrn.h"
//#include <string.h>
#include <unistd.h>
//#include <stdio.h>
#include <sys/time.h>
#include <memory>
#include <assert.h>
//#include "xc_ffmpeg.h"
#include "stdafx.h"
#include "cJSON.h"
#include <ctime>

#define XCFF_LOGI(...)   printf("info:" __VA_ARGS__); printf("\n"); fflush(stdout);
#define XCFF_LOGD(...)   printf("debug:" __VA_ARGS__); printf("\n"); fflush(stdout);
#define XCFF_LOGW(...)   printf("warn:" __VA_ARGS__); printf("\n"); fflush(stdout);
#define XCFF_LOGE(...)   printf("error:" __VA_ARGS__); printf("\n"); fflush(stdout); assert(0);

//static char  CJSON_FILE_PATH[]="R:/GitHub/cppJSONTest/cjson.json";
//#define  CJSON_FILE_PATH "/home/zhouhanjiang/XCloud/xc_rtc_native_demo/linux/xc_rtc_native_demo.json"
#define  CJSON_FILE_PATH "R:/GitHub/cppJSONTest/xc_rtc_native_demo.json"
//#define CJSON_FILE_PATH "R:/GitLab/xl_thunder_pc_test/xcloud_mediagateway_test/PullStream/Script/xc_rtc_native_demo_push.json"
#define int32_t int


extern "C"
{

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

int32_t iFrameFps = 15;//֡��
int32_t iFrameH = 480;//�߶�
int32_t iFrameW = 320;//���
int32_t iFrameCaptureWay = 0; //0:����ɫ
char* RoomId = 0; //�����
char* SignalServerIp = "127.0.0.1"; //���������IP
int32_t SignalServerPort = 0;//����������˿�
int32_t max_PullStreamCount = 1;//peer������
int create_peer_mode = 3; //1:push&pull;2:push;3:pull

std::time_t starttime = std::time(0);
std::time_t nowtime = std::time(0);

//XCFF_LOGI("time t: [%d]\n",t); 

//���ļ�
char* read_string_from_file(char* filename)
{
    FILE * pFile;  
    long lSize;  
    char * buffer;  	
    size_t result; 
	/* ��Ҫһ��byte��©�ض��������ļ���ֻ�ܲ��ö����Ʒ�ʽ�� */   
    pFile = fopen (filename, "rb" );  
    if (pFile==NULL)  
    {  
        fputs ("File error",stderr);  
        exit (1);  
    }  
  
    /* ��ȡ�ļ���С */  
    fseek (pFile , 0 , SEEK_END);  
    lSize = ftell (pFile);  
    rewind (pFile);  
  
    /* �����ڴ�洢�����ļ� */   
    buffer = (char*) malloc (sizeof(char)*lSize);  
    if (buffer == NULL)  
    {  
        fputs ("Memory error",stderr);   
        exit (2);  
    }  
  
    /* ���ļ�������buffer�� */  
    result = fread (buffer,1,lSize,pFile);  
    if (result != lSize)  
    {  
        fputs ("Reading error",stderr);  
        exit (3);  
    }  
    /* ���������ļ��Ѿ���buffer�У����ɱ�׼�����ӡ���� */  
    //XCFF_LOGD("%s", buffer);   
  
    /* ������ʾ���ر��ļ����ͷ��ڴ� */  
    fclose (pFile);  
    //free (buffer);  
    return buffer;      
}

char* get_json_value_from_string(char* string,char*  key)
{
	char* value = "-1";
	cJSON *json , *json_value ; 
    // �������ݰ�  
    json = cJSON_Parse(string);  
    if (!json)  
    {  
        XCFF_LOGI("Error before: [%s]\n",cJSON_GetErrorPtr()); 
		return value;
    }  
    else  
    {  
        // ��������ֵ  
        json_value = cJSON_GetObjectItem( json , key); 
		if( json_value->type == cJSON_Number )  
        {  
            // ��valueint�л�ý��  
            itoa(json_value->valueint,value,256);
			//snprintf(value, sizeof(json_value->valueint), "%s", json_value->valueint);
        }  
        else if( json_value->type == cJSON_String)
        {  
            // valuestring�л�ý��  
            value = json_value->valuestring;			
        }  
        // �ͷ��ڴ�ռ�  
        //cJSON_Delete(json);
		return value;
	}
  
}


void init()
{
  /* print the version */
  XCFF_LOGI("cJSON_Version: %s\n", cJSON_Version());
  char* json_string = read_string_from_file(CJSON_FILE_PATH);
  
  XCFF_LOGI("json_string: %s\n", json_string);
  char* json_iFrameFps = get_json_value_from_string(json_string,"iFrameFps");
  XCFF_LOGI("json_iFrameFps(string): %s\n", json_iFrameFps);
  iFrameFps = atoi(json_iFrameFps);
  XCFF_LOGI("iFrameFps(int): %d\n", iFrameFps);

  char* json_iFrameH = get_json_value_from_string(json_string,"iFrameH");
  XCFF_LOGI("json_iFrameH(string): %s\n", json_iFrameH);
  iFrameH = atoi(json_iFrameH);
  XCFF_LOGI("iFrameH(int): %d\n", iFrameH);
  
  char* json_iFrameW = get_json_value_from_string(json_string,"iFrameW");
  XCFF_LOGI("json_iFrameW(string): %s\n", json_iFrameW);
  iFrameW = atoi(json_iFrameW);
  XCFF_LOGI("iFrameW(int): %d\n", iFrameW);

  char* json_iFrameCaptureWay = get_json_value_from_string(json_string,"iFrameCaptureWay");
  XCFF_LOGI("json_iFrameCaptureWay(string): %s\n", json_iFrameCaptureWay);
  iFrameCaptureWay = atoi(json_iFrameCaptureWay);
  XCFF_LOGI("iFrameCaptureWay(int): %d\n", iFrameCaptureWay);
  
  char* json_SignalServerIp = get_json_value_from_string(json_string,"SignalServerIp");
  XCFF_LOGI("json_SignalServerIp(string): %s\n", json_SignalServerIp);
  SignalServerIp = json_SignalServerIp;//itoa(json_SignalServerIp);
  XCFF_LOGI("SignalServerIp(string): %s\n", SignalServerIp);

  char* json_RoomId = get_json_value_from_string(json_string,"RoomId");
  XCFF_LOGI("json_RoomId(string): %s\n", json_RoomId);
  RoomId = json_RoomId;
  XCFF_LOGI("RoomId(string): %s\n", RoomId);

  char* json_SignalServerPort = get_json_value_from_string(json_string,"SignalServerPort");
  XCFF_LOGI("json_SignalServerPort(string): %s\n", json_SignalServerPort);
  SignalServerPort = atoi(json_SignalServerPort);
  XCFF_LOGI("SignalServerPort(int): %d\n", SignalServerPort);
  
  char* json_max_PullStreamCount = get_json_value_from_string(json_string,"max_PullStreamCount");
  XCFF_LOGI("json_max_PullStreamCount(string): %s\n", json_max_PullStreamCount);
  max_PullStreamCount = atoi(json_max_PullStreamCount);
  XCFF_LOGI("max_PullStreamCount(int): %d\n", max_PullStreamCount);
  
  char* json_create_peer_mode = get_json_value_from_string(json_string,"create_peer_mode");
  XCFF_LOGI("json_create_peer_mode(string): %s\n", json_create_peer_mode);
  create_peer_mode = atoi(json_create_peer_mode);
  XCFF_LOGI("create_peer_mode(int): %d\n", create_peer_mode);
}

int main(void)
{
    
    init();
	
	/* Now some samplecode for building objects concisely: */
    //create_objects();
    return 0;
}


//ʾ��demo
/* Used by some code below as an example datatype. */
struct record
{
    const char *precision;
    double lat;
    double lon;
    const char *address;
    const char *city;
    const char *state;
    const char *zip;
    const char *country;
};


/* Create a bunch of objects as demonstration. */
static int print_preallocated(cJSON *root)
{
    /* declarations */
    char *out = NULL;
    char *buf = NULL;
    char *buf_fail = NULL;
    size_t len = 0;
    size_t len_fail = 0;

    /* formatted print */
    out = cJSON_Print(root);

    /* create buffer to succeed */
    /* the extra 5 bytes are because of inaccuracies when reserving memory */
    len = strlen(out) + 5;
    buf = (char*)malloc(len);
    if (buf == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* create buffer to fail */
    len_fail = strlen(out);
    buf_fail = (char*)malloc(len_fail);
    if (buf_fail == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* Print to buffer */
    if (!cJSON_PrintPreallocated(root, buf, (int)len, 1)) {
        printf("cJSON_PrintPreallocated failed!\n");
        if (strcmp(out, buf) != 0) {
            printf("cJSON_PrintPreallocated not the same as cJSON_Print!\n");
            printf("cJSON_Print result:\n%s\n", out);
            printf("cJSON_PrintPreallocated result:\n%s\n", buf);
        }
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    /* success */
    printf("%s\n", buf);

    /* force it to fail */
    if (cJSON_PrintPreallocated(root, buf_fail, (int)len_fail, 1)) {
        printf("cJSON_PrintPreallocated failed to show error with insufficient memory!\n");
        printf("cJSON_Print result:\n%s\n", out);
        printf("cJSON_PrintPreallocated result:\n%s\n", buf_fail);
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    free(out);
    free(buf_fail);
    free(buf);
    return 0;
}

/* Create a bunch of objects as demonstration. */
static void create_objects(void)
{
    /* declare a few. */
    cJSON *root = NULL;
    cJSON *fmt = NULL;
    cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;
    int i = 0;

    /* Our "days of the week" array: */
    const char *strings[7] =
    {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };
    /* Our matrix: */
    int numbers[3][3] =
    {
        {0, -1, 0},
        {1, 0, 0},
        {0 ,0, 1}
    };
    /* Our "gallery" item: */
    int ids[4] = { 116, 943, 234, 38793 };
    /* Our array of "records": */
    struct record fields[2] =
    {
        {
            "zip",
            37.7668,
            -1.223959e+2,
            "",
            "SAN FRANCISCO",
            "CA",
            "94107",
            "US"
        },
        {
            "zip",
            37.371991,
            -1.22026e+2,
            "",
            "SUNNYVALE",
            "CA",
            "94085",
            "US"
        }
    };
    volatile double zero = 0.0;

    /* Here we construct some JSON standards, from the JSON site. */

    /* Our "Video" datatype: */
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
    cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "type", "rect");
    cJSON_AddNumberToObject(fmt, "width", 1920);
    cJSON_AddNumberToObject(fmt, "height", 1080);
    cJSON_AddFalseToObject (fmt, "interlace");
    cJSON_AddNumberToObject(fmt, "frame rate", 24);

    /* Print to text */
    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our "days of the week" array: */
    root = cJSON_CreateStringArray(strings, 7);

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our matrix: */
    root = cJSON_CreateArray();
    for (i = 0; i < 3; i++)
    {
        cJSON_AddItemToArray(root, cJSON_CreateIntArray(numbers[i], 3));
    }

    /* cJSON_ReplaceItemInArray(root, 1, cJSON_CreateString("Replacement")); */

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our "gallery" item: */
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "Image", img = cJSON_CreateObject());
    cJSON_AddNumberToObject(img, "Width", 800);
    cJSON_AddNumberToObject(img, "Height", 600);
    cJSON_AddStringToObject(img, "Title", "View from 15th Floor");
    cJSON_AddItemToObject(img, "Thumbnail", thm = cJSON_CreateObject());
    cJSON_AddStringToObject(thm, "Url", "http:/*www.example.com/image/481989943");
    cJSON_AddNumberToObject(thm, "Height", 125);
    cJSON_AddStringToObject(thm, "Width", "100");
    cJSON_AddItemToObject(img, "IDs", cJSON_CreateIntArray(ids, 4));

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our array of "records": */
    root = cJSON_CreateArray();
    for (i = 0; i < 2; i++)
    {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddStringToObject(fld, "precision", fields[i].precision);
        cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
        cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
        cJSON_AddStringToObject(fld, "Address", fields[i].address);
        cJSON_AddStringToObject(fld, "City", fields[i].city);
        cJSON_AddStringToObject(fld, "State", fields[i].state);
        cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
        cJSON_AddStringToObject(fld, "Country", fields[i].country);
    }

    /* cJSON_ReplaceItemInObject(cJSON_GetArrayItem(root, 1), "City", cJSON_CreateIntArray(ids, 4)); */

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "number", 1.0 / zero);

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);
}
