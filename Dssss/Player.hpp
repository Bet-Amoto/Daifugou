#pragma once
# include <Siv3D.hpp>
#include "GameEvent.hpp"
using namespace PlayingCard;
class Player {
public:
	Player(String name) { playername = name; };
	/**
	 * @brief ゲーム開始時に呼ばれる関数
	 * @param mycard 手持ちのカード
	 * @param givecards 敵に渡すカードの数
	 * @return 敵に渡すカード
	 */
	virtual Array<Card> initplayer(const Array<Card>& mycard, int givecards) = 0;
	/**
	 * @brief 自分のターンに呼ばれる関数
	 * @param gameevent 現在のゲームの情報
	 * @param mycard 自分の手持ちのカード
	 * @return 場に出すカード
	 */
	virtual Array<Card> action(const GameEvent& GameEvent, const Array<Card>& mycard) = 0;
	String getname() const { return playername; }

private:
	String playername;
};
