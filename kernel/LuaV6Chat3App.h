#pragma once

#include "V6Chat3App.h"

#define XUNLEI_IDEAPP_CLASS		"V6Chat3App.Class"
#define XUNLEI_IDEAPP_OBJ		"V6Chat3App"

#include <list>

#include "Listener\ServerMsgListener.h"

class LuaV6Chat3App
{
public:
	LuaV6Chat3App(void);
	~LuaV6Chat3App(void);

    static V6Chat3App* __stdcall Instance(void*);

	static int AddTray(lua_State *luaState);                              //�������ͼ��
	static int GetMyHead(lua_State *luaState);                            //��ȡ�Լ���ͷ��
	static int GetMyInfo(lua_State *luaState);                            //��ȡ�Լ�������
	static int GetMyIpAndPort(lua_State *luaState);                       //��ñ���IP�������˿ں�
	static int SetMyIntroduction(lua_State *luaState);                    //�����Լ��Ľ���
	static int SelectMyHead(lua_State *luaState);                         //����ѡ��ͼƬ�Ի���ѡ��ͷ��
	static int Start(lua_State *luaState);                                //�����ʼ��ť�¼�
	static int StartServer(lua_State *luaState);                          // �������ӷ������������û��б�������ɺ���ã�ȷ������״̬�ܼ�ʱ���£�
	static int Quit(lua_State *luaState);                                 //�˳�
	static int DeleteTray(lua_State *luaState);                           //ɾ������

	static int GetUserList(lua_State *luaState);                          //��ȡ�û��б�
	static int GetUserInfoByID(lua_State *luaState);                      //ͨ��uid����û�����
	static int GetUnreadMessage(lua_State *luaState);                     //��ȡ���µ�δ����Ϣ
	static int DeleteUnreadMessage(lua_State *luaState);                  //ɾ������һ��δ����Ϣ

	static int AddServer(lua_State *luaState);                            // ��ӷ�����
	static int GetServerList(lua_State *luaState);                        // ��÷������б�
	static int GetServerInfoByID(lua_State *luaState);                    // ͨ��sid��÷���������
	static int ChangeServerInfo(lua_State *luaState);                     // �޸�ĳ������������
	static int SearchFriendOnServer(lua_State *luaState);                 // �ڷ���������������
	static int GetSearchFriendResult(lua_State *luaState);                // ����������

	static int AddP2PFriend(lua_State *luaState);                         //���P2P����
	static int AddServerFriend(lua_State *luaState);                      //��ӷ���˺���
	static int AddFriendAck(lua_State *luaState);                         //������Ӻ���������û�ѡ��ͬ��/�ܾ���
	static int ChangeFriendGroup(lua_State *luaState);                    //���ĺ������ڷ���
	static int DeleteFriend(lua_State *luaState);                         //ɾ������

	static int ChatWithText(lua_State *luaState);                         //������������
	static int GetChatContent(lua_State *luaState);                       //����յ�����������
	static int SelectChatFile(lua_State *luaState);                       //ѡ������͵��ļ�
	static int GetSendFile(lua_State *luaState);                          //��÷����б��е��ļ�
	static int GetRecvFile(lua_State *luaState);                          //��ý����б��е��ļ�
	static int GetChatHistory(lua_State *luaState);                       //��������¼

	static int GetCursorPos(lua_State *luaState);                         //��õ�ǰ���ָ�������
	static int GetWorkareaPos(lua_State *luaState);                       //�����Ļ����
	static int GetImageHandle(lua_State *luaState);                       //���ͼƬ�ľ��
	static int OpenShellExecute(lua_State *luaState);                     //�ⲿ��һ���ļ�/�ļ���
	static int PostWindowMessage(lua_State *luaState);                    //���ʹ�����Ϣ
	static int FileExist(lua_State *luaState);                            // �ļ��Ƿ����
	static int CopyToClipboard(lua_State *luaState);                      // ����һ���ַ�����������

	static long RegisterObj(XL_LRT_ENV_HANDLE hEnv);

protected:
	static V6Chat3App* GetV6Chat3App( lua_State* luaState );
};