#pragma once
#include "PacketID.h"


namespace PacketLayer
{	
#pragma pack(push, 1)

	// 구조체를 여러 타입으로 정의해보자. 연습용
	
	struct RecvPacketInfo
	{
		int clientIdx = 0;
		short PacketId = 0;
		short PacketBodySize = 0;
		char* pRefData = 0;
	};
	// packet head 
	struct PktHeader
	{
		short TotalSize;
		short Id;
	};



	// 에러 던질 때 
	const int ERR_MSG_MAX = 30;
	struct SC_Error_Msg
	{
		char msg[ERR_MSG_MAX] = { 0, };
	};



	// 로그인 관련 구조체
	const int MAX_USER_ID_SIZE = 16;
	struct CS_Login_Pkt
	{
		char szID[MAX_USER_ID_SIZE+1] = { 0, };
	};
	struct SC_Login_Pkt
	{
		char msg[13] = { 0, };
	};




	//
	// 로비 관련 구조체들 
	//
	const int MAX_LOBBY_LIST_COUNT = 10;

	struct LobbyInfo
	{
		short LobbyID;
		short LobbyClientCount;
		short LobbyMaxClientCount;
	};

	struct CS_LobbyList_Pkt
	{
	};

	// 서버 -> 클라 : 로비 갯수를 뿌려주고 
	// 각각 로비에 대한 정보를 던져준다.
	struct SC_LobbyList_Pkt
	{
		short LobbyCount = 0;
		LobbyInfo LobbyList[MAX_LOBBY_LIST_COUNT];
	};

	struct CS_LobbyEnter_Pkt
	{
		short lobbyIdBySelected = -1;
	};

	const int LOBBY_ENTER_MSG = 20;
	struct SC_LobbyEnter_Pkt
	{
		short MaxUserCount;
		short MaxRoomCount;
		char msg[LOBBY_ENTER_MSG+1] = { 0, };
	};

	const int MAX_LOBBY_CHAT_SIZE = 256;
	struct SC_Lobby_Chat_Pkt
	{
		char sendID[MAX_USER_ID_SIZE + 1] = { 0, };
		char msg[MAX_LOBBY_CHAT_SIZE + 1] = { 0, };
	};
	struct CS_Lobby_Chat_Pkt
	{
		char msg[MAX_LOBBY_CHAT_SIZE + 1] = { 0, };
	};


	
	//
	// 룸에 대한 구조체
	//
	const int ROOM_ENTER_MSG = 20;
	struct SC_Room_Enter
	{
		char ID[MAX_USER_ID_SIZE] = { 0, };
	};
	struct CS_Room_Enter
	{
		short roomIdBySelected;
	};




#pragma pack(pop)


	
}