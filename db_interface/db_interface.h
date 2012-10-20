#ifndef _DB_INTERFACE_H_
#define _DB_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MYINFO_DBFILE_NAME "..\\..\\UserData\\My\\my_info.db"
#define MYINFO_DB_ROWS     8

#define PEERSINFO_DBFILE_NAME "..\\..\\UserData\\peers_info.db"
#define PEERSINFO_DB_ROWS     12

#define SERVERSINFO_DBFILE_NAME "..\\..\\UserData\\servers_info.db"
#define SERVERSINFO_DB_ROWS     7

#define CHATHISTORY_DBFILE_NAME "..\\..\\UserData\\chat_history.db"
#define CHATHISTORY_DB_ROWS     4

typedef struct New_Peer_Profile
{
	char* nick_name;
	char* group_name;
	char* sex;
	char* age;
	char* introduction;
	char* head_image_path;
	char* server_id;
	char* server_uid;
	char* last_ip;
	char* tcp_port;
	char* udp_port;

	struct New_Peer_Profile* next;
}*insert_peer_profile_list, new_peer_profile;

	//����Լ����û���Ϣ�����ش�����Ϣ��id��ʧ�ܷ���0��
	int insert_myProfile(const char *name,const char *sex,const char *age,const char *introduction,const char *head_image_path, const char *v4_port, const char *v6_port);
	//�����Լ����û���Ϣ���粻��������ӣ�Ҳ�ɴ���idΪ0����ӣ������ش�����Ϣ��id��ʧ�ܷ���0��
	int update_myProfile(const char *id,const char *name,const char *sex,const char *age,const char *introduction,const char *head_image_path, const char *v4_port, const char *v6_port);
	//����id��ȡ�Լ����û���Ϣ�������release_db_data�ͷ�name��introduction,head_image_path
    bool get_myProfileById(const char *id, char **name, unsigned short *sex, unsigned int *age, char **introduction, char **head_image_path, int *v4_port, int *v6_port);
	//����nick_name��ȡ�Լ����û���Ϣ�������release_db_data�ͷ�introduction,head_image_path
    bool get_myProfileByName(const char *name, unsigned int *id, unsigned short *sex, unsigned int *age, char **introduction, char **head_image_path, int *v4_port, int *v6_port);

	//��ȡ���к�����Ϣ�������release_indb_table_data�ͷ�peers_table
	bool get_peersProfile(char ***peers_table, int *rows, int *cols);
	//������к�����Ϣ
	bool clear_peersProfile();
	//����uid��ȡ���ѵ��û���Ϣ�������release_db_data�ͷ�nick_name��group_name��introduction,head_image_path,server_uid,last_ip
	bool get_peerProfileById(const char *uid, char **nick_name, char **group_name, int *sex, int *age, char **introduction, char **head_image_path, int *server_id, char **server_uid, char **last_ip, int *tcp_port, int *udp_port);
	//���һ��������Ϣ�����ش�����Ϣ��id��ʧ�ܷ���0��
	int insert_peerProfile(const char *nick_name, const char *group_name, const char *sex, const char *age, const char *introduction, const char *head_image_path, const char *server_id, const char *server_uid, const char *last_ip, const char *tcp_port, const char *udp_port);
	// ��Ӷ���������Ϣ
	bool insert_peersProfile(const insert_peer_profile_list peers_list);
	// ������ӵ����һ��������Ϣ��uid
	int last_insert_peerProfileUid();
	//����һ��������Ϣ���粻��������ӣ�Ҳ�ɴ���uidΪ0����ӣ������ش�����Ϣ��id��ʧ�ܷ���0��
	int update_peerProfile(const char *uid, const char *nick_name, const char *group_name, const char *sex, const char *age, const char *introduction, const char *head_image_path, const char *server_id, const char *server_uid, const char *last_ip, const char *tcp_port, const char *udp_port);
	// ɾ��һ��������Ϣ
	bool delete_peerProfile(const char *uid);

	//��ȡ���з������Ϣ�������release_indb_table_data�ͷ�servers_table
	bool get_serversInformation(char ***servers_table, int *rows, int *cols);
	//������з������Ϣ
	bool clear_serversInformation();
	//���һ���������Ϣ�����ش�����Ϣ��id��ʧ�ܷ���0��
	int insert_serverInformation(const char *name, const char *ip, const char *port, const char *description, const char *uid, const char *upassword);
	// ����һ���������Ϣ���粻��������ӣ�Ҳ�ɴ���sidΪ0����ӣ������ش�����Ϣ��id��ʧ�ܷ���0��
	int update_serverInformation(const char *sid, const char *name, const char *ip, const char *port, const char *description, const char *uid, const char *upassword);

	// ���һ�������¼�������release_db_data�ͷ�insert_time�����ش�����¼��id��ʧ�ܷ���0��
	int insert_chatItem(const char *uid, const char *name, const char *item, char **insert_time);
	// ����ָ������������¼�������release_indb_table_data�ͷ�chat_table����*idΪ-1ʱ������������������
	bool get_chatItems(const char *uid, const char *id, const char *num, char ***chat_table, int *rows, int *cols);

	void release_db_data(void **p);
	void release_indb_table_data(char **p);

#ifdef __cplusplus
}
#endif

#endif