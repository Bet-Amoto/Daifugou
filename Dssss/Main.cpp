# include <Siv3D.hpp>
#include "GameEvent.hpp"
#include "Player.hpp"
#include "Gamemaster.hpp"
using namespace PlayingCard;

//反則しない程度にランダムに打つ
class randombot :public Player {
public:
	//Player()のところでこのbotの名前を設定
	randombot() :Player(U"RandomBot") {};

	Array<Card> initplayer(const Array<Card>& mycard, int givecards) override{
		//貧民、大貧民の時はゲームマスターがかってに強い順にカードを選択するのでgivecardsの値は0になります
		//渡すカードが2枚なら弱いほうから2枚選択
		if (givecards == 2)return Array<Card>{mycard[0], mycard[1]};
		//渡すカードが1枚なら弱いほうから1枚選択
		if (givecards == 1)return Array<Card>{mycard[0]};
		//渡すカードが0なら空の配列を返す
		return Array<Card>();
	}
	Array<Card> action(const GameEvent& gameevent, const Array<Card>& mycard) override{
		Array<Card> tmp = Array<Card>();
		//場に出せるカードを検索
		for (auto p : mycard)if (gameevent.checkcard(p))tmp << p;
		//出せるカードがなければパス
		if(tmp.size()==0)return Array<Card>();
		//出せるカードの中からランダムに1枚カードを選ぶ
		Card n = tmp.choice();
		//選択したカードを返す
		return Array<Card>{n};
	}
private:
};

class mybot :public Player {
public:
	mybot() :Player(U"MyBot") {};
	Array<Card> initplayer(const Array<Card>& mycard, int givecards) override {
		if (givecards == 2)return Array<Card>{mycard[0], mycard[1]};
		if (givecards == 1)return Array<Card>{mycard[0]};
		return Array<Card>();
	}
	Array<Card> action(const GameEvent& gameevent, const Array<Card>& mycard) override {
		Array<Card> tmp = Array<Card>();
		for (auto p : mycard)if (gameevent.checkcard(p))tmp << p;
		if (tmp.size() == 0)return Array<Card>();
		gameevent.sortcard(tmp);
		Card n = tmp.front();
		return Array<Card>{n};
	}
private:
};

void Main()
{
	Window::Resize(1280, 720);
	Scene::SetBackground(Palette::Darkgreen);
	const Array<String> rankname{ U"大富豪",U"富豪", U"貧民", U"大貧民" };
	Array<Player*> players{ (new mybot()),(new randombot()) ,(new randombot()) ,(new randombot()) };
	Gamemastar mastar(players);
	mastar.initgame();
	const double cooltime = 1;
	double t = 0;
	while (System::Update())
	{
		t += Scene::DeltaTime();
		if (t > cooltime) {
			if (mastar.isfinish()) {
				ClearPrint();
				Print << U"ゲームが終了しました";
				Array<Player*> ranking = mastar.getRanking();
				for (auto p : step(ranking.size())) {
					Print << U"{}:{}"_fmt(rankname[p], ranking[p]->getname());
				}
				mastar.initgame();
				t = 0; continue;
			}
			mastar.progress();
			t = 0;
		}
		mastar.draw();
	}
}
