#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef unsigned int uint;

#define REGIST_OK "regist_ok"
#define REGIST_FAILED "regist failed: name existed"

#define CANCELL_OK "cancell ok"
#define CANCELL_ONLOGIN "cancell onlogin"
#define CANCELL_FAILED "cancell failed"

#define LOGIN_OK "login_ok"
#define RELOGIN "relogin"
#define LOGIN_FAILED "login failed: name error or pwd error or relogin"

#define SEARCH_USR_NO "no such people"
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

#define UNKNOW_ERROR "unknow error"
#define FRIEND_EXIST "friend exist"
#define ADD_FRIEND_OFFLINE "usr offline"
#define ADD_FRIEND_NOTEXSIT "usr not exsit"
#define DELETE_FRIEND_OK "delete friend ok"

#define DIR_NOT_EXIST "dir not exist"
#define FILE_NAME_EXIST "file name exist"
#define CREATE_DIR_OK "create dir ok"

#define DEL_DIR_OK "delete dir ok"
#define DEL_DIR_FAILED "delete dir failed"

#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAILED "rename file failed"

#define ENTER_DIR_OK "enter dir ok"

#define UPLOAD_FILE_OK "upload file ok"
#define UPLOAD_FILE_FAILED "upload file failed"

#define DOWNLOAD_FILE_OK "download file ok"
#define DOWNLOAD_FILE_FAILED "download file failed"


enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_TYPE_REGIST_REQUEST,       //注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND,

    ENUM_MSG_TYPE_CANCELL_REQUEST,      //注销请求
    ENUM_MSG_TYPE_CANCELL_RESPOND,

    ENUM_MSG_TYPE_LOGIN_REQUEST,        //登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,   //在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,

    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,   //搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,   //添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,

    ENUM_MSG_TYPE_ADD_FRIEND_AGREE,     //同意添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_DISAGREE,  //拒绝添加好友请求

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST, //刷新好友请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,//删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST, //私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,

    ENUM_MSG_TYPE_PUBLIC_CHAT_REQUEST,  //公共聊天请求
    ENUM_MSG_TYPE_PUBLIC_CHAT_RESPOND,

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,   //创建文件夹请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,

    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,   //刷新文件请求
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,

    ENUM_MSG_TYPE_DEL_DIR_REQUEST,      //删除文件夹请求
    ENUM_MSG_TYPE_DEL_DIR_RESPOND,

    ENUM_MSG_TYPE_RENAME_REQUEST,       //重命名文件请求
    ENUM_MSG_TYPE_RENAME_RESPOND,

    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,    //进入文件夹请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,  //上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,
    ENUM_MSG_TYPE_UPLOAD_FILE_CANCEL,

    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,//下载文件请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,
    ENUM_MSG_TYPE_DOWNLOAD_FILE_CANCEL,

    ENUM_MSG_TYPE_SEND_FILE,            //接收文件
    ENUM_MSG_TYPE_SEND_FILE_PROGRESS,   //接收进度


    ENUM_MSG_TYPE_MAX=0x00ffffff,
};

struct FileInfo
{
    char caFileName[64];// 文件名字
    int iFileType;      // 文件类型
};

struct PDU
{
    uint uiPDULen;  //总的协议数据单元大小
    uint uiMsgType; //消息类型
    char caData[64];
    uint uiMsgLen;  //实际消息长度
    char caMsg[];   //实际消息
};

PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
