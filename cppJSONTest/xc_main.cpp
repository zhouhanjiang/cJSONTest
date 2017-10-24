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
#include "cJSON.h"
//#include "xc_ffmpeg.h"
//#include "stdafx.h"
#include <ctime>

#define XCFF_LOGI(...)   printf("info:" __VA_ARGS__); printf("\n"); fflush(stdout);
#define XCFF_LOGD(...)   printf("debug:" __VA_ARGS__); printf("\n"); fflush(stdout);
#define XCFF_LOGW(...)   printf("warn:" __VA_ARGS__); printf("\n"); fflush(stdout);
#define XCFF_LOGE(...)   printf("error:" __VA_ARGS__); printf("\n"); fflush(stdout); assert(0);

//static char  CJSON_FILE_PATH[]="R:/GitHub/cppJSONTest/cjson.json";
#define  CJSON_FILE_PATH "/home/zhouhanjiang/XCloud/MediaGatewayLongRun/xc_rtc_native_demo.json"
//#define  CJSON_FILE_PATH "R:/GitHub/cppJSONTest/cjson.json"
//#define CJSON_FILE_PATH "R:/GitLab/xl_thunder_pc_test/xcloud_mediagateway_test/PullStream/Script/xc_rtc_native_demo_push.json"
#define int32_t int


extern "C"
{

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

std::time_t starttime = std::time(0);
std::time_t nowtime = std::time(0);

int32_t iFrameFps = 15;//帧数
int32_t iFrameH = 480;//高度
int32_t iFrameW = 320;//宽度
int32_t iFrameCaptureWay = 0; //0:渐变色
char* RoomId = 0; //房间号
char* SignalServerIp = "127.0.0.1"; //信令服务器IP
int32_t SignalServerPort = 0;//信令服务器端口
int32_t max_PullStreamCount = 1;//peer拉流数
int create_peer_mode = 3; //1:push&pull;2:push;3:pull

static bool s_bRun = true;
static int32_t s_iAnchorPeerId = 0;
static int32_t s_iLastPeerId = 0;
static int32_t s_iPullPeerCount = 0;

uint64_t xc_currentMs()
{
  timeval nowUs;
  gettimeofday(&nowUs, NULL);
  return (int64_t)nowUs.tv_sec * 1000 + (int64_t)nowUs.tv_usec / 1000;
}

static void OnMessage(int32_t iPeerId, EXCRNMsg eMsg, uint64_t lparam, uint64_t rparam) {
  std::time_t nowtime = std::time(0);
  if (nowtime - starttime < 1)
  {
	if (eMsg!=E_XCRN_MSG_ICE_SET_REMOTE_SDP){return;}
  }
  else{std::time_t starttime = std::time(0);}
  switch (eMsg) {
    case E_XCRN_MSG_FAIL: {
      EXCRNErrno eErrno = (EXCRNErrno)lparam;
      XCFF_LOGW("E_XCRN_MSG_FAIL PeerId=[%d] eErrno=[%d]-[%s]\n",
             iPeerId, eErrno, XCRN_ErrnoName(eErrno));
      s_bRun = false;
    }
      break;
    case E_XCRN_MSG_ICE_SET_LOCAL_SDP: {
      const char* szSdp = (const char*)lparam;
      XCFF_LOGI ("E_XCRN_MSG_ICE_SET_LOCAL_SDP PeerId=[%d] strlen(szSdp)=[%d] szSdp=[%s]\n", iPeerId, (int32_t)strlen(szSdp), szSdp);
    }
      break;
    case E_XCRN_MSG_ICE_SET_REMOTE_SDP: {
      const char* szSdp = (const char*)lparam;
      XCFF_LOGI ("E_XCRN_MSG_ICE_SET_REMOTE_SDP iPeerId=[%d] s_iLastPeerId=[%d] strlen(szSdp)=[%d] szSdp=[%s]\n", iPeerId, s_iLastPeerId,(int32_t)strlen(szSdp),szSdp);	  	 
      
	  if(max_PullStreamCount<1)
	  {
		  max_PullStreamCount=0;
	  }
	  else
	  {
		max_PullStreamCount=max_PullStreamCount;
	  }
	  if (iPeerId == s_iLastPeerId && s_iPullPeerCount < max_PullStreamCount) {
        /// 创建拉流
        
        XCRNCreatePeerParam paramPeer = {0};
        paramPeer.bAnchor = false;                   /// 是否是主播
        paramPeer.bCostomCapture = false;            /// 自定义采集方式
        strcpy (paramPeer.szSignalServerIp, SignalServerIp);  /// 信令服务器ip地址
        paramPeer.usSignalServerPort = SignalServerPort;  /// 信令服务器端口号
        strcpy (paramPeer.szRoomName, RoomId);        /// 连接的房间名称
        
        int32_t iViewerPeerId = iPeerId;
		XCFF_LOGI ("E_XCRN_MSG_ICE_SET_REMOTE_SDP before Create ViewerPeerPeerId=[%d] s_iPullPeerCount=[%d]\n", iViewerPeerId, s_iPullPeerCount);
        EXCRNErrno eErrno = XCRN_CreatePeer(&paramPeer, &iViewerPeerId);
        if (E_XCRN_ERRNO_SUCCESS != eErrno) {
          assert(false);
          return;
        }
        s_iLastPeerId = iViewerPeerId;
        s_iPullPeerCount++;
		XCFF_LOGI ("E_XCRN_MSG_ICE_SET_REMOTE_SDP after Create ViewerPeerPeerId=[%d] s_iPullPeerCount=[%d]\n", iViewerPeerId, s_iPullPeerCount);
      }
	
    }
      break;
    case E_XCRN_MSG_SIGNAL_SEND: {
      const char* pSend = (const char*)lparam;
      int32_t iSize = (int32_t)rparam;
      XCFF_LOGI ("E_XCRN_MSG_SIGNAL_SEND_OFFER PeerId=[%d] pSend=[%d]-[%.*s]\n", iPeerId, iSize, iSize, pSend);
    }
      break;
    case E_XCRN_MSG_SIGNAL_RECV: {
      const char* pRecv = (const char*)lparam;
      int32_t iSize = (int32_t)rparam;
      XCFF_LOGI ("E_XCRN_MSG_SIGNAL_RECV_ANSWER PeerId=[%d] pRecv=[%d]-[%.*s]\n", iPeerId, iSize, iSize, pRecv);
    }
      break;

    case E_XCRN_MSG_VIDEO_ON_SIZE: {
      int32_t iW = (int32_t)lparam;
      int32_t iH = (int32_t)rparam;
      XCFF_LOGI ("E_XCRN_MSG_VIDEO_ON_SIZE PeerId=[%d] iW=[%d] iH=[%d]\n", iPeerId, iW, iH);
    }
      break;
    case E_XCRN_MSG_VIDEO_ON_FRAME: {
      XCRNFrameParam* pFrame = (XCRNFrameParam*)lparam;      
	    XCFF_LOGI ("E_XCRN_MSG_VIDEO_ON_FRAME PeerId=[%d] [%llu] iW=[%d] iH=[%d] llTimestampUs=[%lld] SignalServerIp=[%s] SignalServerPort=[%d] RoomId=[%s] max_PullStreamCount=[%d] create_peer_mode=[%d]\n",
                   iPeerId, pFrame->ullIndex, pFrame->iW, pFrame->iH, pFrame->llTimestampUs,SignalServerIp,SignalServerPort,RoomId,max_PullStreamCount,create_peer_mode);		
    }
      break;
    case E_XCRN_MSG_SIGNAL_EOF: {
      int32_t iErrno = (int32_t)lparam;
      const char* szReason = (const char*)rparam;
      XCFF_LOGI ("E_XCRN_MSG_SIGNAL_EOF PeerId=[%d] iErrno=[%d]-[%s]", iPeerId, iErrno, szReason);
    }
      break;
    case E_XCRN_MSG_ICE_CREATE_OFFER:
    case E_XCRN_MSG_SIGNAL_CONNECT:
    case E_XCRN_MSG_ICE_CONNECTION_NEW:
    case E_XCRN_MSG_ICE_CONNECTION_CHECKING:
    case E_XCRN_MSG_ICE_CONNECTION_CONNECTED:
    case E_XCRN_MSG_ICE_CONNECTION_COMPLETED:
      XCFF_LOGI ("OnMessage PeerId=[%d] Msg=[%d]-[%s]", iPeerId, eMsg, XCRN_MessageName(eMsg));
      break;
    default:
      XCFF_LOGE ("OnState PeerId=[%d] eState=[%d]-[%s]\n", iPeerId, eMsg, XCRN_MessageName(eMsg));
      break;
  }
}


int64_t xc_test_CreatePeer(){
	if (create_peer_mode == 1 or create_peer_mode == 2) {
	//拉流判断
	if (create_peer_mode == 2){max_PullStreamCount=0;}
	
	// 创建推流
	XCRNCreatePeerParam paramPeer = {0};
	paramPeer.bAnchor = true;                   /// 是否是主播
	paramPeer.bCostomCapture = true;            /// 自定义采集方式
	strcpy (paramPeer.szSignalServerIp, SignalServerIp);  /// 信令服务器ip地址
	paramPeer.usSignalServerPort = SignalServerPort;  /// 信令服务器端口号
	strcpy (paramPeer.szRoomName, RoomId);        /// 连接的房间名称
	XCFF_LOGI("Create AnchorPeer");
	EXCRNErrno eErrno = XCRN_CreatePeer(&paramPeer, &s_iAnchorPeerId);
	if (E_XCRN_ERRNO_SUCCESS != eErrno) {
		assert(false);
		return eErrno;
	}
  
	s_iLastPeerId = s_iAnchorPeerId;

	//const char* szUrl = "rtmp://rtmp.stream2.show.xunlei.com/live/4399_645036674";
	//const char* szUrl = "/Users/zhouhanjiang/Project/XCloud/xc_rtc_demo/test.mp4";
	//xcff_decode(szUrl, s_iAnchorPeerId);
  
  
	/// 实现采集	
	int32_t iFrameArea = iFrameW * iFrameH;
	std::unique_ptr<uint32_t[]> upFrame(new uint32_t[iFrameArea]);
	uint32_t* pFrame = upFrame.get();
	uint8_t ucR = UCHAR_MAX;
	do {
		for (int32_t iIdx = 0; iIdx < iFrameArea; iIdx++) {
			pFrame[iIdx] = (ucR<<16)|(ucR<<8)|ucR;/// + 0xFF000000;
		}
		ucR-=1;

		XCRN_OnCapture(s_iAnchorPeerId, iFrameW, iFrameH, pFrame);
		usleep (1000 / iFrameFps * 1000);
	} while (s_bRun);
	XCRN_DestroyPeer(s_iAnchorPeerId);
  }
  else if (create_peer_mode == 3 and max_PullStreamCount>0) {
	// 创建拉流
    XCFF_LOGI("Create ViewerPeer"); 
    XCRNCreatePeerParam paramPeer = {0};
    paramPeer.bAnchor = false;                   /// 是否是主播
    paramPeer.bCostomCapture = false;            /// 自定义采集方式
    strcpy (paramPeer.szSignalServerIp, SignalServerIp);  /// 信令服务器ip地址
    paramPeer.usSignalServerPort = SignalServerPort;  /// 信令服务器端口号
    strcpy (paramPeer.szRoomName, RoomId);        /// 连接的房间名称
            
	int32_t iViewerPeerId = 0;
	XCFF_LOGI("XCRN_CreatePeer(pullstream).peerid=[%d]\n", iViewerPeerId);
    EXCRNErrno eErrno = XCRN_CreatePeer(&paramPeer, &iViewerPeerId);    
    if (E_XCRN_ERRNO_SUCCESS != eErrno) {
        assert(false);
        return eErrno;
    }
    
	s_iLastPeerId = iViewerPeerId;
	s_iPullPeerCount++;
	
    sleep(3000000000);
    XCRN_DestroyPeer(iViewerPeerId);
  }
}

//读文件
char* read_string_from_file(char* filename)
{
    FILE * pFile;  
    long lSize;  
    char * buffer;  	
    size_t result; 
	/* 若要一个byte不漏地读入整个文件，只能采用二进制方式打开 */   
    pFile = fopen (filename, "rb" );  
    if (pFile==NULL)  
    {  
        fputs ("File error",stderr);  
        exit (1);  
    }  
  
    /* 获取文件大小 */  
    fseek (pFile , 0 , SEEK_END);  
    lSize = ftell (pFile);  
    rewind (pFile);  
  
    /* 分配内存存储整个文件 */   
    buffer = (char*) malloc (sizeof(char)*lSize);  
    if (buffer == NULL)  
    {  
        fputs ("Memory error",stderr);   
        exit (2);  
    }  
  
    /* 将文件拷贝到buffer中 */  
    result = fread (buffer,1,lSize,pFile);  
    if (result != lSize)  
    {  
        fputs ("Reading error",stderr);  
        exit (3);  
    }  
    /* 现在整个文件已经在buffer中，可由标准输出打印内容 */  
    //XCFF_LOGD("%s", buffer);   
  
    /* 结束演示，关闭文件并释放内存 */  
    fclose (pFile);  
    //free (buffer);  
    return buffer;      
}

char* get_json_value_from_string(char* string,char*  key)
{
	char* value = "-1";
	cJSON *json , *json_value ; 
    // 解析数据包  
    json = cJSON_Parse(string);  
    if (!json)  
    {  
        XCFF_LOGI("Error before: [%s]\n",cJSON_GetErrorPtr()); 
		return value;
    }  
    else  
    {  
        // 解析开关值  
        json_value = cJSON_GetObjectItem( json , key); 
		if( json_value->type == cJSON_Number )  
        {  
            // 从valueint中获得结果  
            //itoa(json_value->valueint,value,256);
			snprintf(value, sizeof(json_value->valueint), "%s", json_value->valueint);
        }  
        else if( json_value->type == cJSON_String)
        {  
            // valuestring中获得结果  
            value = json_value->valuestring;			
        }  
        // 释放内存空间  
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

int main(int argc, char* argv[])
{
  //xcff_init();
  //printf ("xcrn version=[%s] __DATE__=[%s]\n", XCRN_Version(), __DATE__);
  //fflush(stdout);
  init();
  XCFF_LOGI("xcrn version=[%s] ;__DATE__=[%s];create_peer_mode=[%d]\n", XCRN_Version(), __DATE__,create_peer_mode);

  XCRNInitParam param = {0};  
  const char* szPathEnd = strrchr(argv[0], '/');
  memcpy (param.szCfgPath, argv[0], szPathEnd-argv[0]);
  param.lpfnOnMessage = OnMessage;
  param.eInnerLogLevel = XCRNLogLevel(2); //SENSITIVE = 0;VERBOSE = 1;INFO = 2;WARNING = 3;ERROR = 4;NONE = 5
  //strcpy (param.eInnerLogLevel, "2");   
  XCRN_Init(&param);
  
  xc_test_CreatePeer();
     
  sleep(1000000);
  XCRN_Uninit();
  return 0;
}