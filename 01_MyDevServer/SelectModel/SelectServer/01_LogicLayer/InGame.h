#pragma once


// 여기서부터 합시다 
// TODO
// 인게임 클래스 구현 --> Room 구현 --> 로비 구현 --> 로비매니저 구현 


namespace LogicLayer
{
	enum class GameState
	{
		NONE,
		START,
		ING,
		END
	};

	class InGame
	{
	public:
		InGame();
		~InGame();
		void Init();
	};
}