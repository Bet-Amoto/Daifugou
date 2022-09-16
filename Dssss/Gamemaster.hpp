﻿#pragma once
# include <Siv3D.hpp>
#include "GameEvent.hpp"
#include "Player.hpp"
using namespace PlayingCard;
class  Gamemastar {
public:
	Gamemastar(Array<Player*> ps,bool log=0);
	//山札を作る
	void makeyamafuda(bool setblind = false);
	//ゲームのメイキング
	void initgame();
	//ゲームの進行
	void progress();
	//ゲームが終了したか
	bool isfinish();
	void draw() const;
	Array<Player*> getRanking();
private:
	int32 nextturn(int32 n);
	bool ishave(int who, Array<Card> cards);
	bool ishave(int who, Card card);
	bool usecard(int who, Array<Card> cards);
	bool usecard(int who, Card card);
	Array<int32> setranking(Array<int32> fin);
	int32 countplayer(int32 c);
	void printlog(String str,int logLv=1);
	Pack pack;
	GameEvent events;
	int32 gamecount = 4;
	int32 setcount = 0;
	int32 nowturn = 0;
	int32 countturn = 0;
	int32 passcount = 0;
	Font font;
	Array<Card> yamafuda;
	Array<Card> blindcard;
	Array<Array<Card>> playercard;
	Array<Player*> players;
	Array<int32> ranking;
	Array<int32> finish;
	int32 loglv;
	const Array<String> rankname{ U"大富豪",U"富豪", U"貧民", U"大貧民" };
};
